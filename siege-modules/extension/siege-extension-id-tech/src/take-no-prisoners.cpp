#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <cassert>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>
#include "id-tech-shared.hpp"

using siege::platform::hardware_context;
using siege::platform::game_action;
using siege::platform::keyboard_binding;
using siege::platform::mouse_binding;
using siege::platform::controller_binding;
using siege::platform::input_mapping_ex;
using siege::platform::controller_input_type;
using siege::configuration::key_type;

using siege::platform::game_command_line_caps;
using siege::platform::game_action;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;

namespace fs = std::filesystem;
namespace stl = std::ranges;
using namespace std::literals;

std::optional<std::string_view> bind_key_for_vkey(const input_mapping_ex& mapping) noexcept;
std::optional<std::string_view> mouse_key_for_vkey(const input_mapping_ex& mapping) noexcept;
std::optional<input_mapping_ex> vkey_for_mouse_key(const std::string_view& mapping) noexcept;
std::optional<input_mapping_ex> vkey_for_bind_key(const std::string_view& mapping) noexcept;
std::optional<std::string_view> joy_key_for_vkey(const input_mapping_ex& mapping) noexcept;
std::optional<std::string_view> pov_key_for_vkey(const input_mapping_ex& mapping) noexcept;

constexpr std::string_view str(const std::array<char, 32>& data) noexcept
{
  if (data.back() != '\0')
  {
    return std::string_view{ data.data(), data.size() };
  }

  return std::string_view{ data.data() };
}

constexpr std::array<char, 32> arr(std::string_view data) noexcept
{
  std::array<char, 32> result{};
  assert(data.size() <= result.size());
  auto size = std::clamp<std::size_t>(data.size(), 0, result.size());

  stl::copy(data.begin(), data.begin() + size, result.begin());
  return result;
}

extern "C" {
extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"dinput" } };

extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { L"Joystick", L"Mouse" } },
  .string_settings = { { L"name" } },
  .player_name_setting = L"name",
  .controller_enabled_setting = L"Joystick",
  .mouse_enabled_setting = L"Mouse",
};

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+backward", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+strafeleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+straferight", u"Strafe Right", u"Movement" },
  game_action{ game_action::digital, "+jump", u"Jump", u"Movement" },
  game_action{ game_action::digital, "+down", u"Prone", u"Movement" },
  game_action{ game_action::digital, "+up", u"Stand-up", u"Movement" },
  game_action{ game_action::digital, "+speed", u"Run", u"Movement" },
  game_action{ game_action::analog, "+turnleft", u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, "+turnright", u"Turn Right", u"Aiming" },
  game_action{ game_action::digital, "+raisecamera", u"Raise Camera", u"Aiming" },
  game_action{ game_action::digital, "+midcamera", u"Center Camera", u"Aiming" },
  game_action{ game_action::digital, "+lowercamera", u"Lower Camera", u"Aiming" },
  game_action{ game_action::digital, "+aimup", u"Aim Up", u"Aiming" },
  game_action{ game_action::digital, "aimcenter", u"Aim Center", u"Aiming" },
  game_action{ game_action::digital, "+aimdown", u"Aim Down", u"Aiming" },
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "+use", u"Use", u"Combat" },
  game_action{ game_action::digital, "previtem", u"Previous Item", u"Combat" },
  game_action{ game_action::digital, "nextitem", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "useitem", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "dropitem", u"Drop Item", u"Combat" },
  game_action{ game_action::digital, "nextweapon", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "prevweapon", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "dropweapon", u"Drop Weapon", u"Combat" },
  game_action{ game_action::digital, "showscore", u"Show Scores", u"Interface" },
  game_action{ game_action::digital, "looktoggle", u"Look Toggle", u"Misc" },
  game_action{ game_action::digital, "pause", u"Pause", u"Misc" },
} };

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x20120494) },
  { "concmds"sv, std::size_t(0x2012049c) },
  { "cl_showactors"sv, std::size_t(0x2011e600) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 9> function_name_ranges{ {
  { "-mAim"sv, "looktoggle"sv },
  { "-look"sv, "+yawpos"sv },
  { "-6DOF"sv, "+use"sv },
  { "suicide"sv, "impulse"sv },
  { "-lowercamera"sv, "+raisecamera"sv },
  { "-speed"sv, "+strafeleft"sv },
  { "A_LoadPosMarkFile"sv, "A_MoveActor"sv },
  { "goldblum"sv, "juggernaut"sv },
  { "vp_fov"sv, "vp_enginecmd"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 4> variable_name_ranges{ { { "a_acceleration"sv, "a_friction"sv },
  { "cl_showactors"sv, "cl_TimeStampVel"sv },
  { "joyStrafe"sv, "in_ForwardSpeed"sv },
  { "vp_enginetest"sv, "vp_mipdist"sv } } };

std::errc get_function_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(function_name_ranges, length, data, saved);
}

std::errc get_variable_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(variable_name_ranges, length, data, saved);
}

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
}

std::errc is_input_mapping_valid(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping)
{
  using namespace siege::platform;
  if (mapping == nullptr)
  {
    return std::errc::bad_address;
  }

  if (caps == nullptr)
  {
    return std::errc::bad_address;
  }

  if (mapping->mapping_size < sizeof(input_mapping_ex))
  {
    return std::errc::bad_message;
  }

  if (mapping->context == hardware_context::mouse)
  {
    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 0
        && !(mapping->action_name == arr("+left"sv) || mapping->action_name == arr("+right"sv)))
    {
      return std::errc::not_supported;
    }
    return std::errc{};
  }

  if (is_for_controller(caps->context))
  {
    if (caps->hardware_index != 0 || mapping->hardware_index >= 2)
    {
      return std::errc::not_supported;
    }

    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 0
        && !(mapping->action_name == arr("+left"sv) || mapping->action_name == arr("+right"sv) || mapping->action_name == arr("+strafeleft"sv) || mapping->action_name == arr("+straferight"sv)))
    {
      return std::errc::not_supported;
    }

    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 1
        && !(mapping->action_name == arr("+forward"sv) || mapping->action_name == arr("+backward"sv)))
    {
      return std::errc::not_supported;
    }
  }

  return std::errc{};
}

std::errc apply_prelaunch_settings_ex(const wchar_t* exe_path_str, siege::platform::game_command_line_args_ex* args) noexcept
{
  if (exe_path_str == nullptr)
  {
    return std::errc::bad_address;
  }

  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  auto packaged_args = static_cast<siege::platform::packaged_args>(*args);

  std::ofstream custom_bindings("siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  for (auto& input : packaged_args.action_bindings)
  {
    if (auto key = mouse_key_for_vkey(input)
          .or_else([&](...) { return pov_key_for_vkey(input); })
          .or_else([&](...) { return joy_key_for_vkey(input); }))
    {
      config.emplace(key_type({ *key }), key_type(str(input.action_name)));
      continue;
    }

    if (auto key = bind_key_for_vkey(input))
    {
      config.emplace(key_type({ "bind"sv, *key }), key_type(str(input.action_name)));
      continue;
    }
  }

  const auto& bindings = packaged_args.action_bindings;

  const static auto is_mouse = [](hardware_context context) { return context == hardware_context::mouse; };

  struct analog_binding
  {
    std::string_view setting;
    std::string_view action_positive;
    std::string_view action_negative;
    decltype(siege::platform::is_left_direction)* is_correct_direction;
    decltype(siege::platform::is_for_controller)* is_correct_input;
  };

  constexpr static std::array<analog_binding, 5> flippables{ {
    analog_binding{ "in_FlipMouseX"sv, "+turnleft"sv, "+turnright"sv, siege::platform::is_left_direction, is_mouse },
    analog_binding{ "in_FlipMouseY"sv, "+forward"sv, "+backward"sv, siege::platform::is_up_direction, is_mouse },
    analog_binding{ "in_FlipJoyX"sv, "+turnleft"sv, "+turnright"sv, siege::platform::is_left_direction, siege::platform::is_for_controller },
    analog_binding{ "in_FlipJoyX"sv, "+strafeleft"sv, "+straferight"sv, siege::platform::is_left_direction, siege::platform::is_for_controller },
    analog_binding{ "in_FlipJoyY"sv, "+forward"sv, "+backward"sv, siege::platform::is_up_direction, siege::platform::is_for_controller },
  } };

  for (auto& flippable : flippables)
  {
    auto hardware_index = flippable.setting.ends_with("X") ? 0 : 1;

    auto positive = stl::find_if(bindings, [&](const input_mapping_ex& bindable) {
      return flippable.is_correct_input(bindable.context) && str(bindable.action_name) == flippable.action_positive && bindable.hardware_input_type == controller_input_type::axis && bindable.hardware_index == hardware_index;
    });

    if (positive == bindings.end())
    {
      continue;
    }

    auto negative = stl::find_if(bindings, [&](const input_mapping_ex& bindable) {
      return flippable.is_correct_input(bindable.context) && str(bindable.action_name) == flippable.action_negative && bindable.hardware_input_type == controller_input_type::axis && bindable.hardware_index == hardware_index;
    });

    if (negative == bindings.end())
    {
      continue;
    }

    if (flippable.is_correct_direction(positive->vkey))
    {
      config.emplace(key_type({ flippable.setting }), key_type("0"sv));
    }
    else
    {
      config.emplace(key_type({ flippable.setting }), key_type("1"sv));
    }
  }

  const auto joy_has_strafe_left = stl::any_of(bindings, [](const input_mapping_ex& bindable) {
    return siege::platform::is_for_controller(bindable.context) && bindable.hardware_input_type == controller_input_type::axis && bindable.hardware_index == 0
           && str(bindable.action_name) == "+strafeleft"sv;
  });

  const auto joy_has_strafe_right = stl::any_of(bindings, [](const input_mapping_ex& bindable) {
    return siege::platform::is_for_controller(bindable.context) && bindable.hardware_input_type == controller_input_type::axis && bindable.hardware_index == 0
           && str(bindable.action_name) == "+straferight"sv;
  });

  if (joy_has_strafe_left && joy_has_strafe_right)
  {
    config.emplace(key_type({ "joyStrafe"sv }), key_type("1"sv));
  }
  else
  {
    config.emplace(key_type({ "joyStrafe"sv }), key_type("0"sv));
  }

  config.save(custom_bindings);

  insert_string_setting_once(packaged_args.string_settings, L"exec", L"siege_studio_inputs.cfg");

  return std::errc{};
}

const wchar_t** format_command_line(const siege::platform::game_command_line_args* args, std::uint32_t* new_size)
{
  if (!args)
  {
    return nullptr;
  }

  if (!new_size)
  {
    return nullptr;
  }

  static std::vector<std::wstring> string_args;
  string_args.clear();

  for (auto& setting : args->string_settings)
  {
    if (!setting.name)
    {
      continue;
    }

    if (std::wstring_view(setting.name) == command_line_caps.ip_connect_setting)
    {
      continue;
    }

    if (std::wstring_view(setting.name) == command_line_caps.preferred_exe_setting)
    {
      continue;
    }

    if (!setting.value)
    {
      continue;
    }

    if (!setting.value[0])
    {
      continue;
    }

    if (std::wstring_view(setting.name) == L"map")
    {
      string_args.emplace_back(L"+map");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) == L"exec")
    {
      string_args.emplace_back(L"+exec");
      string_args.emplace_back(setting.value);
    }
    else
    {
      continue;
    }
  }

  static std::vector<const wchar_t*> raw_args;
  raw_args.resize(string_args.size());
  *new_size = (std::uint32_t)string_args.size();

  std::transform(string_args.begin(), string_args.end(), raw_args.begin(), [](const std::wstring& value) {
    return value.c_str();
  });

  return raw_args.data();
}

std::errc init_mouse_inputs(mouse_binding* binding) noexcept
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  auto upsert_inputs = [&](auto& config) {
    std::string temp;
    for (auto& item : config.keys())
    {
      if (!(item.at(0).starts_with("mouse") || item.at(0).starts_with("MOUSE") || item.at(0).starts_with("Mouse")))
      {
        continue;
      }

      auto vkey = vkey_for_mouse_key(item.at(0));

      if (!vkey)
      {
        continue;
      }

      auto entry = config.find(item);
      if (entry.at(0).empty())
      {
        continue;
      }
      temp = entry.at(0);

      if (temp.size() - 1 > binding->inputs[0].action_name.size())
      {
        temp.resize(binding->inputs[0].action_name.size() - 1);
      }

      temp = siege::platform::to_lower(temp);

      auto existing = std::find_if(binding->inputs.begin(), binding->inputs.end(), [&](auto& input) { return input.virtual_key == vkey->vkey; });

      if (existing != binding->inputs.end())
      {

        std::memcpy(existing->action_name.data(), temp.data(), temp.size());
        existing->input_type = siege::platform::controller_input_type::button;
        existing->virtual_key = vkey->vkey;
        existing->context = siege::platform::mouse_context::mouse;
        continue;
      }

      auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

      if (first_available == binding->inputs.end())
      {
        break;
      }

      std::memcpy(first_available->action_name.data(), temp.data(), temp.size());
      first_available->input_type = siege::platform::controller_input_type::button;
      first_available->virtual_key = vkey->vkey;
      first_available->context = siege::platform::mouse_context::mouse;
    }
  };

  if (auto config = load_config_from_file(L"tnp.cfg"))
  {
    upsert_inputs(*config);
  }

  if (auto config = load_config_from_file(L"userdef.cfg"))
  {
    upsert_inputs(*config);
  }

  std::array<std::pair<WORD, std::string_view>, 4> axes{
    {
      std::make_pair<WORD, std::string_view>(VK_UP, "+forward"),
      std::make_pair<WORD, std::string_view>(VK_DOWN, "+backward"),
      std::make_pair<WORD, std::string_view>(VK_LEFT, "+left"),
      std::make_pair<WORD, std::string_view>(VK_RIGHT, "+right"),
    }
  };

  upsert_mouse_axis_defaults(game_actions, axes, *binding);

  return std::errc{};
}

std::errc init_keyboard_inputs(keyboard_binding* binding) noexcept
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  auto upsert_inputs = [&](auto& config) {
    std::string temp;
    for (auto& item : config.keys())
    {
      if (!(item.at(0).starts_with("bind")))
      {
        continue;
      }

      auto vkey = vkey_for_bind_key(item.at(1));

      if (!vkey)
      {
        continue;
      }

      auto entry = config.find(item);
      if (entry.at(0).empty())
      {
        continue;
      }
      temp = entry.at(0);

      if (temp.size() - 1 > binding->inputs[0].action_name.size())
      {
        temp.resize(binding->inputs[0].action_name.size() - 1);
      }

      temp = siege::platform::to_lower(temp);

      auto existing = std::find_if(binding->inputs.begin(), binding->inputs.end(), [&](auto& input) { return input.virtual_key == vkey->vkey; });

      if (existing != binding->inputs.end())
      {
        std::memcpy(existing->action_name.data(), temp.data(), temp.size());
        existing->input_type = siege::platform::controller_input_type::button;
        existing->virtual_key = vkey->vkey;
        existing->context = siege::platform::keyboard_context::keyboard;
        continue;
      }

      auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

      if (first_available == binding->inputs.end())
      {
        break;
      }

      std::memcpy(first_available->action_name.data(), temp.data(), temp.size());
      first_available->input_type = siege::platform::controller_input_type::button;
      first_available->virtual_key = vkey->vkey;
      first_available->context = siege::platform::keyboard_context::keyboard;
    }
  };

  if (auto config = load_config_from_file(L"tnp.cfg"))
  {
    upsert_inputs(*config);
  }

  if (auto config = load_config_from_file(L"userdef.cfg"))
  {
    upsert_inputs(*config);
  }

  return std::errc{};
}


std::errc default_controller_inputs(controller_binding* binding, std::uint32_t layout_index) noexcept
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (layout_index > 0)
  {
    return std::errc::invalid_argument;
  }


  std::array<std::pair<WORD, std::string_view>, 24> actions{
    {
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_TRIGGER, "+attack"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_TRIGGER, "useitem"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_SHOULDER, "nextitem"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_SHOULDER, "nextweapon"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_A, "+jump"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_B, "+down"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_X, "+use"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "+up"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, "+speed"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_UP, "+forward"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, "+backward"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, "+strafeleft"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, "+straferight"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, "+turnleft"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, "+turnright"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_UP, "+aimup"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, "+aimdown"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, "aimcenter"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_UP, "dropitem"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_DOWN, "dropweapon"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "+lowercamera"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "+raisecamera"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_VIEW, "showscore"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_MENU, "ToggleHelp"),
    }
  };

  append_controller_defaults(game_actions, actions, *binding);

  return std::errc{};
}
}

std::optional<std::string_view> mouse_key_for_vkey(const input_mapping_ex& mapping) noexcept
{
  if (mapping.vkey == VK_LBUTTON)
  {
    return "mouse1"sv;
  }

  if (mapping.vkey == VK_RBUTTON)
  {
    return "mouse2"sv;
  }

  if (mapping.vkey == VK_MBUTTON)
  {
    return "mouse3"sv;
  }

  return std::nullopt;
}

std::optional<input_mapping_ex> vkey_for_mouse_key(const std::string_view& mapping) noexcept
{
  input_mapping_ex result{};
  result.vkey = VK_LBUTTON;

  if (mouse_key_for_vkey(result) == mapping)
  {
    return result;
  }

  result.vkey = VK_RBUTTON;

  if (mouse_key_for_vkey(result) == mapping)
  {
    return result;
  }
  result.vkey = VK_MBUTTON;

  if (mouse_key_for_vkey(result) == mapping)
  {
    return result;
  }
  return std::nullopt;
}

std::optional<std::string_view> joy_key_for_vkey(const input_mapping_ex& mapping) noexcept
{
  constexpr static std::array<std::string_view, 15> buttons{ {
    "joy1"sv,
    "joy2"sv,
    "joy3"sv,
    "joy4"sv,
    "joy5"sv,
    "joy6"sv,
    "joy7"sv,
    "joy8"sv,
    "joy9"sv,
    "joy10"sv,
    "joy11"sv,
    "joy12"sv,
    "joy13"sv,
    "joy14"sv,
    "joy15"sv,
  } };

  if (siege::platform::is_for_controller(mapping.context) && mapping.hardware_input_type == controller_input_type::button && mapping.hardware_index < buttons.size())
  {
    return buttons[mapping.hardware_index];
  }

  return std::nullopt;
}

struct mapping
{
  WORD vkey;
  std::string_view name;
};

constexpr auto& get_ascii_keys()
{
  constexpr static auto uppercase_keys = [] {
    std::array<mapping, 36> results{};
    auto upper_keys = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"sv;

    for (auto i = 0; i < upper_keys.size(); ++i)
    {
      results[i].vkey = (WORD)upper_keys[i];
      results[i].name = upper_keys.substr(i, 1);
    }
    return results;
  }();

  static_assert(uppercase_keys[0].vkey == 0x41);
  static_assert(uppercase_keys[0].name == "A"sv);

  static_assert(uppercase_keys[uppercase_keys.size() - 1].vkey == 0x39);
  static_assert(uppercase_keys[uppercase_keys.size() - 1].name == "9"sv);
  return uppercase_keys;
}

constexpr auto& get_special_keys()
{
  constexpr static auto special_keys = std::array<mapping, 46>{ {
    { .vkey = VK_UP, .name = "UPARROW"sv },
    { VK_DOWN, "DOWNARROW"sv },
    { VK_LEFT, "LEFTARROW"sv },
    { VK_RIGHT, "UPARROW"sv },
    { VK_LCONTROL, "CTRL"sv },
    { VK_RETURN, "ENTER"sv },
    { VK_LSHIFT, "SHIFT"sv },
    { VK_LMENU, "ALT"sv },
    { VK_HOME, "HOME"sv },
    { VK_PRIOR, "PGUP"sv },
    { VK_ESCAPE, "ESCAPE"sv },
    { VK_NEXT, "PGDN"sv },
    { VK_END, "END"sv },
    { VK_DELETE, "DEL"sv },
    { VK_INSERT, "INS"sv },
    { VK_PRINT, "PRINTSCREEN"sv },
    { VK_CAPITAL, "CAPS"sv },
    { VK_PAUSE, "PAUSE"sv },
    { VK_TAB, "TAB"sv },
    { VK_SPACE, "SPACE"sv },
    { VK_OEM_1, "SEMICOLON"sv },
    { VK_OEM_7, "QUOTE"sv },
    { VK_SCROLL, "SCROLL"sv },
    { VK_F1, "F1"sv },
    { VK_F2, "F2"sv },
    { VK_F3, "F3"sv },
    { VK_F4, "F4"sv },
    { VK_F5, "F5"sv },
    { VK_F6, "F6"sv },
    { VK_F7, "F7"sv },
    { VK_F8, "F8"sv },
    { VK_F9, "F9"sv },
    { VK_F10, "F10"sv },
    { VK_F11, "F11"sv },
    { VK_F12, "F12"sv },
    { VK_OEM_COMMA, ","sv },
    { VK_OEM_PERIOD, "."sv },
    { VK_OEM_MINUS, "-"sv },
    { VK_OEM_PLUS, "="sv },
    { VK_OEM_1, ";"sv },
    { VK_OEM_2, "/"sv },
    { VK_OEM_3, "`"sv },
    { VK_OEM_4, "["sv },
    { VK_OEM_5, "\\"sv },
    { VK_OEM_6, "]"sv },
    { VK_OEM_7, "'"sv },
  } };
  return special_keys;
}

std::optional<std::string_view> bind_key_for_vkey(const input_mapping_ex& mapping) noexcept
{
  auto is_valid_key = [&](auto& key) {
    return key.vkey == mapping.vkey;
  };
  auto iter = stl::find_if(get_special_keys(), is_valid_key);

  if (iter != get_special_keys().end())
  {
    return iter->name;
  }

  auto letter_iter = stl::find_if(get_ascii_keys(), is_valid_key);

  if (letter_iter != get_ascii_keys().end())
  {
    return letter_iter->name;
  }

  return std::nullopt;
}

std::optional<input_mapping_ex> vkey_for_bind_key(const std::string_view& mapping) noexcept
{
  auto is_valid_key = [&](auto& key) {
    return key.name == mapping;
  };
  auto is_valid_key_lower = [&](auto& key) {
    return key.name == siege::platform::to_upper(mapping);
  };

  auto iter = stl::find_if(get_special_keys(), is_valid_key);

  if (iter == get_special_keys().end())
  {
    iter = stl::find_if(get_special_keys(), is_valid_key_lower);
  }

  input_mapping_ex result{
    .context = hardware_context::keyboard
  };

  if (iter != get_special_keys().end())
  {
    result.vkey = iter->vkey;
    return result;
  }

  auto letter_iter = stl::find_if(get_ascii_keys(), is_valid_key);

  if (letter_iter == get_ascii_keys().end())
  {
    letter_iter = stl::find_if(get_ascii_keys(), is_valid_key_lower);
  }

  if (letter_iter != get_ascii_keys().end())
  {
    result.vkey = letter_iter->vkey;
    return result;
  }

  return std::nullopt;
}

std::optional<std::string_view> pov_key_for_vkey(const input_mapping_ex& mapping) noexcept
{
  if (mapping.hardware_input_type == controller_input_type::hat && mapping.hardware_index == 0)
  {
    if (siege::platform::is_up_direction(mapping.vkey))
    {
      return "hat1";
    }

    if (siege::platform::is_down_direction(mapping.vkey))
    {
      return "hat2";
    }

    if (siege::platform::is_left_direction(mapping.vkey))
    {
      return "hat3";
    }

    if (siege::platform::is_right_direction(mapping.vkey))
    {
      return "hat4";
    }
  }

  return std::nullopt;
}