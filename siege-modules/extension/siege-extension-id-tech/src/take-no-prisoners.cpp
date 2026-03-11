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
  .flags = { { L"listen", L"serve", L"join" } },
  .int_settings = { { L"Joystick", L"Mouse" } },
  .string_settings = { { L"name", L"map" } },
  .ip_connect_setting = L"join",
  .player_name_setting = L"name",
  .listen_setting = L"listen",
  .dedicated_setting = L"serve",
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
  game_action{ game_action::analog, "+turnleft", u"Turn Left", u"Movement" },
  game_action{ game_action::analog, "+turnright", u"Turn Right", u"Movement" },
  game_action{ game_action::digital, "+raisecamera", u"Raise Camera", u"Camera" },
  game_action{ game_action::digital, "+midcamera", u"Center Camera", u"Camera" },
  game_action{ game_action::digital, "+lowercamera", u"Lower Camera", u"Camera" },
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
  game_action{ game_action::digital, "looktoggle", u"Look Toggle", u"Camera" },
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

std::errc is_input_mapping_valid(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping) noexcept
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

predefined_string*
  get_predefined_id_tech_2_map_command_line_settings_with_extension(const wchar_t* base_dir, bool include_zip, std::string_view map_ext) noexcept;

predefined_string*
  get_predefined_string_command_line_settings(const wchar_t* name) noexcept
{
  if (name && std::wstring_view(name) == L"map")
  {
    return get_predefined_id_tech_2_map_command_line_settings_with_extension(L".", false, ".vbm");
  }

  return nullptr;
}
}