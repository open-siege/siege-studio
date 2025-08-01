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
  // TODO figure out how to get hosting to work correctly
  // Apparently a special variable has to be set to not contact the auth server, but this needs more investigation
  //  .listen_setting = L"connect",
  //  .dedicated_setting = L"dedicated",
  //   .int_settings = { { L"dedicated", L"r_customwidth", L"r_customheight", L"r_mode" } },
  .int_settings = { { L"r_customwidth", L"r_customheight", L"r_mode" } },
  .string_settings = { { L"name", L"connect", L"map", L"r_glDriver" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
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
  game_action{ game_action::digital, "+attackleft", u"Attack", u"Combat" },
  game_action{ game_action::digital, "+attackright", u"Alt Attack", u"Combat" },
  game_action{ game_action::digital, "+melee-attack", u"Melee Attack", u"Combat" },
  game_action{ game_action::digital, "nextinv", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "previnv", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "+use", u"Interact", u"Combat" },
  game_action{ game_action::digital, "+objectives_score", u"Score", u"Interface" },
  game_action{ game_action::digital, "togglemenu", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

extern void(__cdecl* ConsoleEvalCdecl)(const char*);

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0xfb38d8) },
  { "cmdlist"sv, std::size_t(0xfb38d0) },
  { "cl_pitchspeed"sv, std::size_t(0xfb1a68) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 6> function_name_ranges{ {
  { "centerview"sv, "-attackleft"sv },
  { "+holster"sv, "-droprune"sv },
  { "+transferenergy"sv, "-missionstatus"sv },
  { "+cameralook"sv, "-aim"sv },
  { "flushui"sv, "warpinv"sv },
  { "ui_disableinventory"sv, "ui_displayserverinformation"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 5> variable_name_ranges{ {
  { "mp_map_instantkill"sv, "mp_map_specialties"sv },
  { "temp_sv_maxspeed"sv, "temp_mp_gametype"sv },
  { "temp_password"sv, "temp_dedicated"sv },
  { "temp_sv_pure"sv, "temp_mp_modifier_Specialties"sv },
  { "botenable"sv, "sv_strafeJumpingAllowed"sv },
} };

inline void set_gog_exports()
{
  ConsoleEvalCdecl = (decltype(ConsoleEvalCdecl))0x4bd548;
}

constexpr std::array<void (*)(), 5> export_functions = { {
  set_gog_exports,
} };

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
  if (exe_path_str == nullptr)
  {
    return E_POINTER;
  }

  if (args == nullptr)
  {
    return E_POINTER;
  }

  std::ofstream custom_bindings("base/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  bool enable_controller = save_bindings_to_config(*args, config, q3_mapping_context{});

  if (enable_controller)
  {
    config.emplace(siege::configuration::key_type({ "seta", "in_joystick" }), siege::configuration::key_type("1"));
  }

  config.save(custom_bindings);

  auto iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name == nullptr; });

  if (iter != args->string_settings.end())
  {
    iter->name = L"exec";
    iter->value = L"siege_studio_inputs.cfg";
  }

  std::advance(iter, 1);
  iter->name = L"console";
  iter->value = L"1";

  auto connect_iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name != nullptr && setting.value != nullptr && setting.value[0] != '\0' && std::wstring_view(setting.name) == L"connect"; });

  if (connect_iter != args->string_settings.end())
  {
    // TODO do this at the top level
    static std::wstring address_port{};
    address_port = connect_iter->value;
    address_port += L":29254";
    connect_iter->value = address_port.c_str();
  }

  auto server_iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return 
      setting.name != nullptr && 
      setting.value != nullptr && 
      setting.value[0] != '\0' && 
      (std::wstring_view(setting.name) == L"dedicated" || std::wstring_view(setting.name) == L"connect" || std::wstring_view(setting.name) == L"map"); });

  if (server_iter != args->string_settings.end())
  {
    std::advance(iter, 1);
    iter->name = L"com_introplayed";
    iter->value = L"1";

    std::advance(iter, 1);
    iter->name = L"introCommand";
    iter->value = L"wait";
  }
  else
  {
    std::advance(iter, 1);
    iter->name = L"introCommand";
    iter->value = L"";
  }

  auto mode_iter = std::find_if(args->int_settings.begin(), args->int_settings.end(), [](auto& setting) { return setting.name != nullptr && std::wstring_view(setting.name) == L"r_mode"; });

  if (mode_iter != args->int_settings.end())
  {
    auto value = mode_iter->value;
    std::advance(mode_iter, 1);
    mode_iter->name = L"ui_newvidmode";
    mode_iter->value = value;
  }

  return S_OK;
}

HRESULT init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return E_POINTER;
  }
  auto config = load_config_from_pk3(L"base\\global\\default.cfg", L"base/pak0.pk3", L"main/pak0.pk3/global");

  if (config)
  {
    load_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 2> actions{
    { std::make_pair<WORD, std::string_view>(VK_RBUTTON, "+attackright"),
      std::make_pair<WORD, std::string_view>(VK_MBUTTON, "+use") }
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

  auto config = load_config_from_pk3(L"base\\global\\default.cfg", L"base/pak0.pk3", L"main/pak0.pk3/global");

  if (config)
  {
    load_keyboard_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 5> actions{
    {
      std::make_pair<WORD, std::string_view>('G', "+throw-grenade"),
      std::make_pair<WORD, std::string_view>(VK_RETURN, "+use"),
      std::make_pair<WORD, std::string_view>(VK_SPACE, "+moveup"),
      std::make_pair<WORD, std::string_view>(VK_LCONTROL, "+movedown"),
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_TRIGGER, "+attackleft"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_TRIGGER, "attackright"),
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, "+melee-attack"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_X, "inven"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "nextinv"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_SHOULDER, "invnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_SHOULDER, "+throw-grenade"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_DOWN, "weapondrop"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "weapprev"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_VIEW, "+objectives_score"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_MENU, "togglemenu"),
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

  auto name_str = std::wstring_view(name);


  if (name_str == L"r_mode")
  {
    static auto modes = std::array<predefined_int, 9>{
      predefined_int{ .label = L"640x480", .value = 3 },
      predefined_int{ .label = L"800x600", .value = 4 },
      predefined_int{ .label = L"960x720", .value = 5 },
      predefined_int{ .label = L"1024x768", .value = 6 },
      predefined_int{ .label = L"1152x864", .value = 7 },
      predefined_int{ .label = L"1280x1024", .value = 8 },
      predefined_int{ .label = L"1600x1200", .value = 9 },
      predefined_int{ .label = L"Custom", .value = -1 },
      predefined_int{},
    };

    return modes.data();
  }

  return nullptr;
}

predefined_string*
  get_predefined_id_tech_3_map_command_line_settings(const wchar_t* base_dir) noexcept;

predefined_string*
  get_predefined_string_command_line_settings(const wchar_t* name) noexcept
{
  if (name && std::wstring_view(name) == L"map")
  {
    return get_predefined_id_tech_3_map_command_line_settings(L"base");
  }

  return nullptr;
}
}
