#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"


extern "C" {
using hardware_context = siege::platform::hardware_context;
using game_action = siege::platform::game_action;
using keyboard_binding = siege::platform::keyboard_binding;
using mouse_binding = siege::platform::mouse_binding;
using controller_binding = siege::platform::controller_binding;
using game_action = siege::platform::game_action;
using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;

extern auto command_line_caps = game_command_line_caps{
  .flags = { { L"listen", L"dedicated" } },
  .int_settings = { { L"width", L"height", L"vid_mode" } },// width + height are GL Quake only
  .string_settings = { { L"name", L"connect", L"map", L"game", L"preferred_exe" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"listen",
  .dedicated_setting = L"dedicated",
  .selected_game_setting = L"game",
  .preferred_exe_setting = L"preferred_exe",
};

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+back", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+moveleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+moveright", u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, "+jump", u"Jump", u"Movement" },
  game_action{ game_action::analog, "+movedown", u"Crouch", u"Movement" },
  game_action{ game_action::digital, "+speed", u"Run", u"Movement" },
  game_action{ game_action::analog, "+left", u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, "+right", u"Turn Right", u"Aiming" },
  game_action{ game_action::analog, "+lookup", u"Look Up", u"Aiming" },
  game_action{ game_action::analog, "+lookdown", u"Look Down", u"Aiming" },
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "+melee-attack", u"Melee Attack", u"Combat" },
  game_action{ game_action::digital, "impulse 10", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "impulse 12", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "+showscores", u"Score", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

constexpr static auto quake_aliases = std::array<std::array<std::string_view, 2>, 2>{ { 
  { "+melee-attack", "impulse 1; +attack" },
  { "-melee-attack", "impulse 12; -attack" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 4> verification_strings = { {
  // win quake
  std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "WinQuake"sv, std::size_t(0x468e40) },
    { "exec"sv, std::size_t(0x470e84) },
    { "cmd"sv, std::size_t(0x470e9c) },
    { "cl_pitchspeed"sv, std::size_t(0x475af8) } } },
  // gl quake
  std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "WinQuake"sv, std::size_t(0x44f89c) },
    { "exec"sv, std::size_t(0x449594) },
    { "cmd"sv, std::size_t(0x449580) },
    { "cl_pitchspeed"sv, std::size_t(0x448704) } } },

  // quake world
  std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "WinQuake"sv, std::size_t(0x485530) },
    { "exec"sv, std::size_t(0x47c6c8) },
    { "cmd"sv, std::size_t(0x47c6ac) },
    { "cl_pitchspeed"sv, std::size_t(0x47a348) } } },

  // gl quake world
  std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "WinQuake"sv, std::size_t(0x45573c) },
    { "exec"sv, std::size_t(0x44eae4) },
    { "cmd"sv, std::size_t(0x44eb04) },
    { "cl_pitchspeed"sv, std::size_t(0x44c990) } } },
} };

// the order changes between regular quake and glquake, so each value has to be checked by itself
constexpr static std::array<std::pair<std::string_view, std::string_view>, 6> function_name_ranges{ {
  { "timerefresh"sv, "timerefresh"sv },
  { "pointfile"sv, "pointfile"sv },
  { "joyadvancedupdate"sv, "joyadvancedupdate"sv },
  { "force_centerview"sv, "force_centerview"sv },
  { "stuffcmds"sv, "stuffcmds"sv },
  { "wait"sv, "wait"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 7> variable_name_ranges{ {
  { "joyadvanced"sv, "joyadvanced"sv },
  { "joyadvaxisx"sv, "joyadvaxisx"sv },
  { "joyadvaxisy"sv, "joyadvaxisy"sv },
  { "joyadvaxisz"sv, "joyadvaxisz"sv },
  { "joyadvaxisr"sv, "joyadvaxisr"sv },
  { "joyadvaxisu"sv, "joyadvaxisu"sv },
  { "joyadvaxisv"sv, "joyadvaxisv"sv },
} };

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
  if (filename)
  {
    auto lower = siege::platform::to_lower(filename);

    auto is_valid = lower.contains(L"winquake.exe") || lower.contains(L"glquake.exe") || lower.contains(L"qwcl.exe") || lower.contains(L"glqwcl.exe");

    if (!is_valid)
    {
      return std::errc::not_supported;
    }
  }
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
}
std::errc apply_prelaunch_settings(const wchar_t* exe_path_str, siege::platform::game_command_line_args* args)
{
  if (auto result = apply_dpi_awareness(exe_path_str); result != std::errc{})
  {
    return result;
  }

  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  std::ofstream custom_bindings("Id1/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  for (auto& alias : quake_aliases)
  {
    config.emplace(siege::configuration::key_type({ "alias", alias[0] }), siege::configuration::key_type(alias[1]));
  }

  bool enable_controller = save_bindings_to_config(*args, config, mapping_context{ .index_to_axis = hardware_index_to_joystick_axis_id_tech_2_0, .axis_set_prefix = "" });

  if (enable_controller)
  {
    // engine bug - mouse needs to be enabled for the right analog stick to work
    config.emplace(siege::configuration::key_type({ "joystick" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joyadvanced" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joysidesensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joypitchsensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joyadvancedupdate" }), siege::configuration::key_type{});
  }

  config.emplace(siege::configuration::key_type({ "+mlook" }), siege::configuration::key_type{});
  config.save(custom_bindings);

  bind_axis_to_send_input(*args, "+lookup", "+mlook");
  bind_axis_to_send_input(*args, "+lookdown", "+mlook");

  insert_string_setting_once(*args, L"exec", L"siege_studio_inputs.cfg");
  insert_string_setting_once(*args, L"console", L"1");

  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }
  auto config = load_config_from_pak(L"Id1\\default.cfg", L"Id1/PAK0.PAK", L"Id1/PAK0.PAK");

  if (config)
  {
    load_mouse_bindings(*config, *binding);
  }

  return std::errc{};
}

std::errc init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  auto config = load_config_from_pak(L"Id1\\default.cfg", L"Id1/PAK0.PAK", L"Id1/PAK0.PAK");

  if (config)
  {
    load_keyboard_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 8> actions{
    {
      std::make_pair<WORD, std::string_view>('w', "+forward"),
      std::make_pair<WORD, std::string_view>('a', "+moveleft"),
      std::make_pair<WORD, std::string_view>('s', "+back"),
      std::make_pair<WORD, std::string_view>('d', "+moveright"),
      std::make_pair<WORD, std::string_view>('f', "+melee-attack"),
      std::make_pair<WORD, std::string_view>(VK_SPACE, "+jump"),
      std::make_pair<WORD, std::string_view>(VK_LCONTROL, "+movedown"),
      std::make_pair<WORD, std::string_view>(VK_OEM_5, "+mlook"),
    }
  };

  upsert_keyboard_defaults(game_actions, actions, *binding);

  return std::errc{};
}

std::errc init_controller_inputs(controller_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }
  std::array<std::pair<WORD, std::string_view>, 18> actions{
    {
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_TRIGGER, "+attack"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_A, "+jump"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_B, "+movedown"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, "+speed"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_UP, "+forward"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, "+back"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, "+moveleft"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, "+moveright"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, "+left"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, "+right"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_UP, "+lookup"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, "+lookdown"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, "+melee-attack"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "impulse 10"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_SHOULDER, "invnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "impulse 12"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "impulse 10"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_VIEW, "+showscores")
    }
  };

  append_controller_defaults(game_actions, actions, *binding);

  return std::errc{};
}

predefined_int*
  get_predefined_int_command_line_settings(const wchar_t* name) noexcept
{
  if (name == nullptr)
  {
    return nullptr;
  }

  if (std::wstring_view(name) == L"vid_mode")
  {
    static auto modes = std::array<predefined_int, 8>{
      predefined_int{ .label = L"640x480", .value = 8 },
      predefined_int{ .label = L"800x600", .value = 9 },
      predefined_int{ .label = L"1024x768", .value = 10 },
      predefined_int{ .label = L"1280x1024", .value = 11 },
      predefined_int{},
    };

    return modes.data();
  }

  return nullptr;
}

predefined_string*
  get_predefined_id_tech_2_map_command_line_settings(const wchar_t* base_dir, bool include_zip) noexcept;

predefined_string*
  get_predefined_string_command_line_settings(const wchar_t* name) noexcept
{
  if (name && std::wstring_view(name) == L"map")
  {
    return get_predefined_id_tech_2_map_command_line_settings(L"Id1", false);
  }

  if (name && std::wstring_view(name) == L"preferred_exe")
  {
    static auto modes = std::array<predefined_string, 5>{
      predefined_string{ .label = L"WinQuake", .value = L"Winquake.exe" },
      predefined_string{ .label = L"GLQuake", .value = L"Glquake.exe" },
      predefined_string{ .label = L"QuakeWorld", .value = L"qwcl.exe" },
      predefined_string{ .label = L"GLQuakeWorld", .value = L"glqwcl.exe" },
      predefined_string{},
    };

    return modes.data();
  }

  return nullptr;
}
}
