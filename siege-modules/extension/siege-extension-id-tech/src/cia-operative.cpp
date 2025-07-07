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
  .int_settings = { { L"width", L"height" } },
  .string_settings = { { L"name", L"map"} },
  .player_name_setting = L"name"
};

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+back", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+moveleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+moveright", u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, "+moveup", u"Jump", u"Movement" },
  game_action{ game_action::analog, "+movedown", u"Crouch", u"Movement" },
  game_action{ game_action::digital, "+speed", u"Run", u"Movement" },
  game_action{ game_action::analog, "+left", u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, "+right", u"Turn Right", u"Aiming" },
  game_action{ game_action::analog, "+lookup", u"Look Up", u"Aiming" },
  game_action{ game_action::analog, "+lookdown", u"Look Down", u"Aiming" },
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "+zoom", u"Zoom", u"Combat" },
  game_action{ game_action::digital, "+weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "reload", u"Reload", u"Combat" },
  game_action{ game_action::digital, "+use", u"Use/Interact", u"Combat" },
  game_action{ game_action::digital, "toggleinfo", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "SoloMissions"sv, std::size_t(0x487fe0) },
  { "exec"sv, std::size_t(0x486278) },
  { "cmdline"sv, std::size_t(0x486564) },
  { "cl_pitchspeed"sv, std::size_t(0x47f95c) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 2> function_name_ranges{ {
  { "toggleinfo"sv, "timedemo"sv },
  { "name"sv, "mcache"sv },
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

  std::ofstream custom_bindings("main/autoexec.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  bool enable_controller = save_bindings_to_config(*args, config, mapping_context{ .index_to_axis = hardware_index_to_joystick_axis_id_tech_2_0, .axis_set_prefix = "" });

  if (enable_controller)
  {
    config.emplace(siege::configuration::key_type({ "mouse" }), siege::configuration::key_type("1"));
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

  auto free_iter = std::find_if(args->flags.begin(), args->flags.end(), [](auto& setting) { return setting == nullptr; });
  *free_iter = L"-notwarezed";

  return S_OK;
}

HRESULT init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return E_POINTER;
  }
  auto config = load_config_from_pak(L"main\\default.cfg", L"main/pak0.pak", L"main/pak0.pak");

  if (config)
  {
    load_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 1> actions{
    { std::make_pair<WORD, std::string_view>(VK_MBUTTON, "+use") }
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

  auto config = load_config_from_pak(L"main\\default.cfg", L"main/pak0.pak", L"main/pak0.pak");

  if (config)
  {
    load_keyboard_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 3> actions{
    {
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_TRIGGER, "+zoom"),
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_X, "+use"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "weapprev"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_MENU, "toggleinfo"),
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

  return nullptr;
}

predefined_string*
  get_predefined_id_tech_2_map_command_line_settings(const wchar_t* base_dir, bool include_zip) noexcept;

predefined_string*
  get_predefined_string_command_line_settings(const wchar_t* name) noexcept
{
  if (name && std::wstring_view(name) == L"map")
  {
    return get_predefined_id_tech_2_map_command_line_settings(L"main", false);
  }

  return nullptr;
}
}
