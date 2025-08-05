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
using key_type = siege::configuration::key_type;

extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { L"dedicated", L"r_customwidth", L"r_customheight", L"r_mode", L"net_noipx", L"r_fullscreen", L"cg_autoReload", L"cg_drawGun", L"cg_fov", L"scorelimit", L"timelimit" } },
  .string_settings = { { L"name", L"connect", L"map", L"r_glDriver", L"g_gametype" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"net_noipx",
  .dedicated_setting = L"dedicated",
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
  game_action{ game_action::digital, "+altattack", u"Alt Attack", u"Combat" },
  game_action{ game_action::digital, "vstr melee-toggle", u"Toggle Knife", u"Combat" },
  game_action{ game_action::digital, "vstr grenade-toggle", u"Toggle Grenade", u"Combat" },
  game_action{ game_action::digital, "vstr pistol-toggle", u"Toggle Pistol", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "+firemode", u"Change Fire Mode", u"Combat" },
  game_action{ game_action::digital, "+scores", u"Score", u"Interface" },
  game_action{ game_action::digital, "ui_objectives", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x55e8bc) },
  { "cmdlist"sv, std::size_t(0x55e8c4) },
  { "com_maxfps"sv, std::size_t(0x55f7a0) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 8> function_name_ranges{ {
  { "-mlook"sv, "+mlook"sv },
  { "-button14"sv, "+button0"sv },
  { "-goggles"sv, "+firemode"sv },
  { "-zoomout"sv, "+leanleft"sv },
  { "-altattack"sv, "centerview"sv },
  { "bindlist"sv, "bind"sv },
  { "killserver"sv, "banclient"sv },
  { "vminfo"sv, "vmprofile"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 8> variable_name_ranges{ {
  { "com_speeds"sv, "com_maxfps"sv },
  { "cl_motdString"sv, "cl_noprint"sv },
  { "graphshift"sv, "timegraph"sv },
  { "net_qport"sv, "showpackets"sv },
  { "sof2mp_game"sv, "sof2mp_game"sv },
  { "bot_interbreedwrite"sv, "bot_enable"sv },
  { "RMG_textseed"sv, "RMG_map"sv },
  { "r_noserverghoul2"sv, "r_noserverghoul2"sv },
} };

constexpr static auto sof_aliases = std::array<std::array<std::string_view, 3>, 9>{
  { { "melee-toggle", "vstr melee-toggle-start" },
    { "melee-toggle-start", "vstr grenade-toggle-stop;vstr pistol-toggle-stop;weapon 1;set melee-toggle vstr melee-toggle-stop;" },
    { "melee-toggle-stop", "weaplast;set melee-toggle vstr melee-toggle-start" },
    { "grenade-toggle", "vstr grenade-toggle-start" },
    { "grenade-toggle-start", "vstr pistol-toggle-stop;vstr melee-toggle-stop;weapon 8;set grenade-toggle vstr grenade-toggle-stop" },
    { "grenade-toggle-stop", "weaplast;set grenade-toggle vstr grenade-toggle-start" },
    { "pistol-toggle", "vstr pistol-toggle-start" },
    { "pistol-toggle-start", "vstr grenade-toggle-stop;vstr melee-toggle-stop;weapon 2;set pistol-toggle vstr pistol-toggle-stop" },
    { "pistol-toggle-stop", "weaplast;set pistol-toggle vstr pistol-toggle-start" }     
}};

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

std::errc apply_prelaunch_settings(const wchar_t* exe_path_str, siege::platform::game_command_line_args* args)
{
  if (auto result = apply_dpi_awareness(exe_path_str); result != std::errc{})
  {
    return result;
  }

  if (!args)
  {
    return std::errc::bad_address;
  }

  std::ofstream custom_bindings("base/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);


  for (auto& alias : sof_aliases)
  {
    config.emplace(key_type({ "seta", alias[0] }), (alias[1]));
  }

  config.emplace(key_type({ "set", "cg_weaponMenuFast" }), key_type("1"));
  config.emplace(key_type({ "set", "cg_autoswitch" }), key_type("0"));
  // TODO fix issue with CTRL and LCTRL bindings
  config.emplace(key_type({ "bind", "CTRL" }), key_type("+movedown"));

  // TODO fix issue with lower case bindings that do not exist in default.cfg
  config.emplace(key_type({ "bind", "z" }), key_type("vstr pistol-toggle"));

  bool enable_controller = save_bindings_to_config(*args, config, q3_mapping_context{});

  if (enable_controller)
  {
    config.emplace(siege::configuration::key_type({ "seta", "in_joystick" }), siege::configuration::key_type("1"));
  }

  config.save(custom_bindings);

  bind_controller_send_input_fallback(*args, hardware_context::controller_xbox, VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, VK_LEFT);
  bind_controller_send_input_fallback(*args, hardware_context::controller_xbox, VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, VK_RIGHT);

  auto iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name == nullptr; });

  if (iter != args->string_settings.end())
  {
    iter->name = L"exec";
    iter->value = L"siege_studio_inputs.cfg";
  }

  std::advance(iter, 1);
  iter->name = L"console";
  iter->value = L"1";


  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }
  auto config = load_config_from_pk3(L"base\\sof2mp_default.cfg", L"base/mp.pk3", L"base/mp.pk3");

  if (config)
  {
    load_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 2> actions{
    { std::make_pair<WORD, std::string_view>(VK_RBUTTON, "+altattack"),
      std::make_pair<WORD, std::string_view>(VK_MBUTTON, "+firemode") }
  };

  upsert_mouse_defaults(game_actions, actions, *binding);


  return std::errc{};
}

std::errc init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  auto config = load_config_from_pk3(L"base\\sof2mp_default.cfg", L"base/mp.pk3", L"base/mp.pk3");

  if (config)
  {
    load_keyboard_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 6> actions{
    {
      std::make_pair<WORD, std::string_view>(VK_RETURN, "+use"),
      std::make_pair<WORD, std::string_view>(VK_SPACE, "+moveup"),
      std::make_pair<WORD, std::string_view>(VK_LCONTROL, "+movedown"),
      std::make_pair<WORD, std::string_view>('f', "vstr melee-toggle"),
      std::make_pair<WORD, std::string_view>('g', "vstr grenade-toggle"),
      std::make_pair<WORD, std::string_view>('z', "vstr pistol-toggle"),
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, "vstr melee-toggle"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_X, "inven"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_SHOULDER, "invnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_SHOULDER, "+throw-grenade"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_DOWN, "weapondrop"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "weapprev"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_VIEW, "rmg_automap"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_MENU, "ui_objectives"),
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

  if (name && std::wstring_view(name) == L"g_gametype")
  {
    static auto modes = std::array<predefined_string, 6>{
      predefined_string{ .label = L"Deathhmatch", .value = L"dm" },
      predefined_string{ .label = L"Team Deathhmatch", .value = L"tdm" },
      predefined_string{ .label = L"Capture the Flag", .value = L"ctf" },
      predefined_string{ .label = L"Elimination", .value = L"elim" },
      predefined_string{ .label = L"Infiltration", .value = L"inf" },
      predefined_string{},
    };

    return modes.data();
  }

  return nullptr;
}
}
