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
  .int_settings = { { L"width", L"height", L"vid_mode" } },// GL Hexen only
  .string_settings = { { L"name", L"connect", L"map", L"preferred_exe", L"game" } },
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
  game_action{ game_action::digital, "impulse 10", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "impulse 12", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "invright", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "invleft", u"Previous Item", u"Combat" },
  game_action{ game_action::digital, "invuse", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "impulse 44", u"Drop Item", u"Combat" },
  game_action{ game_action::digital, "+showdm", u"Score", u"Interface" },
  game_action{ game_action::digital, "+showinfo", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 2> verification_strings = { {
  // win hexen 2
  std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "HexenII"sv, std::size_t(0x488e44) },
    { "exec"sv, std::size_t(0x491de4) },
    { "cmd"sv, std::size_t(0x491dfc) },
    { "cl_pitchspeed"sv, std::size_t(0x48ef60) } } },
  // gl hexen 2
  std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "HexenII"sv, std::size_t(0x46ab6c) },
    { "exec"sv, std::size_t(0x465774) },
    { "cmd"sv, std::size_t(0x46578c) },
    { "cl_pitchspeed"sv, std::size_t(0x466190) } } },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 6> function_name_ranges{ {
  { "midi_play"sv, "midi_loop"sv },
  { "togglemenu"sv, "menu_class"sv },
  { "+showinfo"sv, "toggle_dm"sv },
  { "slist"sv, "port"sv },
  { "entities"sv, "sensitivity_save"sv },
  { "playerclass"sv, "mcache"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 2> variable_name_ranges{ { { "joyname"sv, "joyadvaxisv"sv },
  { "sv_idealpitchscale"sv, "sv_accelerate"sv } } };

HRESULT get_function_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(function_name_ranges, length, data, saved);
}

HRESULT get_variable_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(variable_name_ranges, length, data, saved);
}

HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
}

HRESULT apply_prelaunch_settings(const wchar_t* exe_path_str, siege::platform::game_command_line_args* args)
{
  if (auto result = apply_dpi_awareness(exe_path_str); result != S_OK)
  {
    return result;
  }

  if (args == nullptr)
  {
    return E_POINTER;
  }

  std::ofstream custom_bindings("data1/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  bool enable_controller = save_bindings_to_config(*args, config, mapping_context{ .index_to_axis = hardware_index_to_joystick_axis_id_tech_2_0, .axis_set_prefix = "" });

  if (enable_controller)
  {
    // engine bug - mouse needs to be enabled for the right analog stick to work
    config.emplace(siege::configuration::key_type({ "joystick" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joyadvanced" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joyadvancedupdate" }), siege::configuration::key_type(""));
  }

  config.save(custom_bindings);

  bind_axis_to_send_input(*args, "+lookup", "+mlook");
  bind_axis_to_send_input(*args, "+lookdown", "+mlook");

  auto iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name == nullptr; });

  std::advance(iter, 1);
  iter->name = L"console";
  iter->value = L"1";

  std::advance(iter, 1);
  iter->name = L"exec";
  iter->value = L"siege_studio_inputs.cfg";

  return S_OK;
}

HRESULT init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return E_POINTER;
  }
  auto config = load_config_from_pak(L"data1\\default.cfg", L"data1/pak0.pak", L"data1/pak0.pak");

  if (config)
  {
    load_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 1> actions{
    { std::make_pair<WORD, std::string_view>(VK_RBUTTON, "invuse") }
  };

  upsert_mouse_defaults(game_actions, actions, *binding);


  return S_OK;
}

HRESULT init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return E_POINTER;
  }

  auto config = load_config_from_pak(L"data1\\default.cfg", L"data1/pak0.pak", L"data1/pak0.pak");

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
      std::make_pair<WORD, std::string_view>('f', "invuse"),
      std::make_pair<WORD, std::string_view>(VK_SPACE, "+moveup"),
      std::make_pair<WORD, std::string_view>(VK_LCONTROL, "+movedown"),
      std::make_pair<WORD, std::string_view>(VK_OEM_5, "+mlook"),
    }
  };

  upsert_keyboard_defaults(game_actions, actions, *binding);

  return S_OK;
}

HRESULT init_controller_inputs(controller_binding* binding)
{
  if (binding == nullptr)
  {
    return E_POINTER;
  }
  std::array<std::pair<WORD, std::string_view>, 23> actions{
    {
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_TRIGGER, "+attack"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_TRIGGER, "invuse"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_A, "+moveup"),
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_X, "inven"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_SHOULDER, "invright"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "impulse 12"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "impulse 10"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_VIEW, "+showdm"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_MENU, "+showinfo"),
    }
  };

  append_controller_defaults(game_actions, actions, *binding);

  return S_OK;
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
    return get_predefined_id_tech_2_map_command_line_settings(L"data1", false);
  }

  if (name && std::wstring_view(name) == L"preferred_exe")
  {
    static auto modes = std::array<predefined_string, 3>{
      predefined_string{ .label = L"Hexen 2 (software rendered)", .value = L"h2.exe" },
      predefined_string{ .label = L"GLHexen 2", .value = L"glh2.exe" },
      predefined_string{},
    };

    return modes.data();
  }

  return nullptr;
}
}
