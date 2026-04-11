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

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"dplayx" } };

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

extern auto game_actions = std::array<game_action, 37>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+backward", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+strafeleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+straferight", u"Strafe Right", u"Movement" },
  game_action{ game_action::digital, "+jump", u"Jump", u"Movement" },
  game_action{ game_action::digital, "+down", u"Crouch", u"Movement" },
  game_action{ game_action::digital, "+speed", u"Run", u"Movement" },
  game_action{ game_action::analog, "+turnleft", u"Turn Left", u"Movement" },
  game_action{ game_action::analog, "+turnright", u"Turn Right", u"Movement" },
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "+attack2", u"Melee Attack", u"Combat" },
  game_action{ game_action::digital, "sb_previtem", u"Previous Item", u"Combat" },
  game_action{ game_action::digital, "sb_nextitem", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "sb_useitem", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "sb_dropitem", u"Drop Item", u"Combat" },
  game_action{ game_action::digital, "impulse 1", u"Spell Level 1", u"Combat" },
  game_action{ game_action::digital, "impulse 2", u"Spell Level 2", u"Combat" },
  game_action{ game_action::digital, "impulse 3", u"Spell Level 3", u"Combat" },
  game_action{ game_action::digital, "impulse 10", u"Special Attack", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 0", u"Use Artifact of Healing", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 1", u"Use StarStone", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 2", u"Use Medallion of Speed", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 3", u"Use Dust of Invisibility", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 4", u"Use Shield of Protection", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 5", u"Use Artifact of Summoning", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 6", u"Use TimeStop", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 12", u"Use Dynamite Bundle", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 13", u"Use Proximity Bomb", u"Combat" },
  game_action{ game_action::digital, "sb_usespecificitem 31", u"Use Relic Piece", u"Combat" },
  game_action{ game_action::digital, "zoomtoggle", u"Zoom Toggle", u"Camera" },
  game_action{ game_action::digital, "+look", u"Camera Look", u"Camera" },
  game_action{ game_action::digital, "+raisecamera", u"Raise Camera", u"Camera" },
  game_action{ game_action::digital, "+midcamera", u"Mid Camera", u"Camera" },
  game_action{ game_action::digital, "+lowercamera", u"Lower Camera", u"Camera" },
  game_action{ game_action::digital, "+look", u"Camera Look", u"Camera" },
  game_action{ game_action::digital, "ToggleHelp", u"Show Help", u"Interface" },
} };

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x4e2bac) },
  { "concmds"sv, std::size_t(0x4e2bbc) },
  { "cl_showactors"sv, std::size_t(0x4eafc4) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 9> function_name_ranges{ {
  { "-mAim"sv, "looktoggle"sv },
  { "-look"sv, "+yawpos"sv },
  { "-6DOF"sv, "+use"sv },
  { "suicide"sv, "impulse"sv },
  { "-lowercamera"sv, "+raisecamera"sv },
  { "-speed"sv, "+strafeleft"sv },
  { "A_LoadPosMarkFile"sv, "A_MoveActor"sv },
  { "vp_rendertorches"sv, "vp_enginecmd"sv },
  { "changerule"sv, "transwarp"sv },
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

  if (mapping->vkey == VK_LSHIFT || mapping->vkey == VK_RSHIFT)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == VK_LCONTROL || mapping->vkey == VK_RCONTROL)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == VK_LMENU || mapping->vkey == VK_RMENU)
  {
    return std::errc::not_supported;
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

  if (auto config = load_config_from_file(L"VAMPIRE.CFG"))
  {
    upsert_inputs(*config);
  }

  if (auto config = load_config_from_file(fs::path(L"MAGE") / L"userdef.cfg"))
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

  if (auto config = load_config_from_file(L"VAMPIRE.CFG"))
  {
    upsert_inputs(*config);
  }

  if (auto config = load_config_from_file(fs::path(L"MAGE") / L"userdef.cfg"))
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_TRIGGER, "impulse 10"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_SHOULDER, "sb_usespecificitem 12"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_SHOULDER, "sb_usespecificitem 13"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_A, "+jump"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_B, "+down"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_X, "sb_useitem"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "sb_nextitem"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, "+speed"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_UP, "+forward"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, "+backward"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, "+strafeleft"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, "+straferight"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, "+turnleft"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, "+turnright"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, "+attack2"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_UP, "impulse 3"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_DOWN, "zoomtoggle"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "impulse 1"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "impsule 3"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_VIEW, "+look"),
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


static std::string VirtualDriveLetter;
constexpr static std::string_view MageslayerDisc = "MAGESLAY";
static auto* TrueGetLogicalDrives = GetLogicalDrives;
static auto* TrueGetDriveTypeA = GetDriveTypeA;
static auto* TrueGetVolumeInformationA = GetVolumeInformationA;
static auto* TrueGetLogicalDriveStringsA = GetLogicalDriveStringsA;

DWORD WINAPI WrappedGetLogicalDrives()
{
  auto result = TrueGetLogicalDrives();
  std::bitset<sizeof(DWORD) * 8> bits(result);

  int driveLetter = static_cast<int>('A');

  for (auto i = 2; i < bits.size(); ++i)
  {
    if (bits[i] == false)
    {
      driveLetter += i;
      bits[i] = true;

      if (VirtualDriveLetter.empty())
      {
        VirtualDriveLetter = static_cast<char>(driveLetter) + std::string(":\\");
      }
      break;
    }
  }


  return bits.to_ulong();
}

DWORD WINAPI WrappedGetLogicalDriveStringsA(DWORD nBufferLength, LPSTR lpBuffer)
{
  // the game doesn't call GetLogicalDrives, so we should call it ourselves
  static auto drive_types = WrappedGetLogicalDrives();

  if (!VirtualDriveLetter.empty())
  {
    std::string drives;
    drives.resize(TrueGetLogicalDriveStringsA(0, drives.data()));
    TrueGetLogicalDriveStringsA((DWORD)drives.size(), drives.data());

    drives.push_back('\0');
    drives.append(VirtualDriveLetter);

    DWORD size = drives.size() < nBufferLength ? (DWORD)drives.size() : nBufferLength;
    std::memcpy(lpBuffer, drives.data(), size);
    return size;
  }

  return TrueGetLogicalDriveStringsA(nBufferLength, lpBuffer);
}

UINT WINAPI WrappedGetDriveTypeA(LPCSTR lpRootPathName)
{
  if (lpRootPathName && !VirtualDriveLetter.empty() && VirtualDriveLetter[0] == lpRootPathName[0])
  {
    return DRIVE_CDROM;
  }

  return TrueGetDriveTypeA(lpRootPathName);
}

BOOL WINAPI WrappedGetVolumeInformationA(
  LPCSTR lpRootPathName,
  LPSTR lpVolumeNameBuffer,
  DWORD nVolumeNameSize,
  LPDWORD lpVolumeSerialNumber,
  LPDWORD lpMaximumComponentLength,
  LPDWORD lpFileSystemFlags,
  LPSTR lpFileSystemNameBuffer,
  DWORD nFileSystemNameSize)
{
  if (lpRootPathName && lpRootPathName[0] == VirtualDriveLetter[0])
  {
    std::vector<char> data(nVolumeNameSize, '\0');
    std::copy(MageslayerDisc.begin(), MageslayerDisc.end(), data.begin());
    std::copy(data.begin(), data.end(), lpVolumeNameBuffer);
    return TRUE;
  }

  return TrueGetVolumeInformationA(lpRootPathName,
    lpVolumeNameBuffer,
    nVolumeNameSize,
    lpVolumeSerialNumber,
    lpMaximumComponentLength,
    lpFileSystemFlags,
    lpFileSystemNameBuffer,
    nFileSystemNameSize);
}

static std::array<std::pair<void**, void*>, 4> detour_functions{ {
  { &(void*&)TrueGetLogicalDrives, WrappedGetLogicalDrives },
  { &(void*&)TrueGetLogicalDriveStringsA, WrappedGetLogicalDriveStringsA },
  { &(void*&)TrueGetDriveTypeA, WrappedGetDriveTypeA },
  { &(void*&)TrueGetVolumeInformationA, WrappedGetVolumeInformationA },
} };

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
  if constexpr (sizeof(void*) != sizeof(std::uint32_t))
  {
    return TRUE;
  }

  if (DetourIsHelperProcess())
  {
    return TRUE;
  }

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

    auto value = app_module.GetProcAddress<std::uint32_t*>("DisableSiegeExtensionModule");

    if (value && *value == -1)
    {
      return TRUE;
    }
  }

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      try
      {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourAttach(func.first, func.second); });

        DetourTransactionCommit();
      }
      catch (...)
      {
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
    }
  }

  return TRUE;
}
}
