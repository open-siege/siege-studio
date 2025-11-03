#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <sstream>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/configuration/id_tech.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>

#include "id-tech-shared.hpp"

using hardware_context = siege::platform::hardware_context;
using game_action = siege::platform::game_action;
using keyboard_binding = siege::platform::keyboard_binding;
using mouse_binding = siege::platform::mouse_binding;
using controller_binding = siege::platform::controller_binding;

using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
namespace fs = std::filesystem;

extern "C" {
extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { L"dedicated", L"deathmatch", L"timelimit", L"fraglimit", L"maxclients", L"numbots", L"gl_mode", L"in_joystick" } },
  // fov
  .string_settings = { { L"name", L"connect", L"map", L"gl_driver", L"game" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"connect",
  .dedicated_setting = L"dedicated",
  .selected_game_setting = L"game"
  // skin
  // teamname
  // bestweap
  // welcome_mess
  // joy_name
  // set adr0 "" to adr19
  //
  //
  // bool settings
  // s_musicenabled
  // s_nosound

  // float settings
  // s_musicvolume
  // s_volume
};

extern auto game_actions = std::array<game_action, 33>{ {
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
  game_action{ game_action::digital, "+melee-attack", u"Melee Attack", u"Combat" },
  game_action{ game_action::digital, "+use", u"Interact", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weapprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "itemnext", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "itemuse", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "weapondrop", u"Drop Weapon", u"Combat" },
  game_action{ game_action::digital, "score", u"Score", u"Interface" },
  game_action{ game_action::digital, "menu objectives", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
  game_action{ game_action::digital, "+weapon-extra", u"Weapon Fiddle", u"Misc" },
} };

constexpr static auto sof_aliases = std::array<std::array<std::string_view, 2>, 8>{ { { "+melee-attack", "weaponselect 1; +attack" },
  { "-melee-attack", "weaponbestsafe; -attack" },
  { "+weapon-extra1", "+weaponextra1;" },
  { "-weapon-extra1", "-weaponextra1;alias +weapon-extra +weapon-extra2; alias -weapon-extra -weapon-extra2" },
  { "+weapon-extra2", "+weaponextra2;" },
  { "-weapon-extra2", "-weaponextra2;alias +weapon-extra +weapon-extra1; alias -weapon-extra -weapon-extra1" },
  { "+weapon-extra", "+weapon-extra1" },
  { "-weapon-extra", "-weapon-extra1" } } };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };
using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x20120494) },
  { "cmdlist"sv, std::size_t(0x2012049c) },
  { "cl_minfps"sv, std::size_t(0x2011e600) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 3> function_name_ranges{ { { "-klook"sv, "centerview"sv },
  { "joy_advancedupdate"sv, "+mlook"sv },
  { "rejected_violence"sv, "print"sv } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 1> variable_name_ranges{ { { "joy_yawsensitivity"sv, "in_mouse"sv } } };

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
  if (exe_path_str == nullptr)
  {
    return std::errc::bad_address;
  }

  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  std::ofstream custom_bindings("user/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  for (auto& alias : sof_aliases)
  {
    config.emplace(siege::configuration::key_type({ "alias", alias[0] }), siege::configuration::key_type(alias[1]));
  }

  bool enable_controller = save_bindings_to_config(*args, config);

  if (enable_controller)
  {
    // engine bug - mouse needs to be enabled for the right analog stick to work
    config.emplace(siege::configuration::key_type({ "set", "in_mouse" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "set", "joy_advanced" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "set", "joy_sidesensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "set", "joy_pitchsensitivity" }), siege::configuration::key_type("1"));
  }

  config.save(custom_bindings);

  bind_axis_to_send_input(*args, "+lookup", "+mlook");
  bind_axis_to_send_input(*args, "+lookdown", "+mlook");

  insert_string_setting_once(*args, L"exec", L"siege_studio_inputs.cfg");
  insert_string_setting_once(*args, L"console", L"1");

  auto int_iter = std::find_if(args->int_settings.begin(), args->int_settings.end(), [](auto& setting) { return setting.name != nullptr && setting.name == std::wstring_view(L"numbots"); });

  if (int_iter != args->int_settings.end())
  {
    std::ofstream custom_bindings("user/add_bots.cfg", std::ios::binary | std::ios::trunc);

    siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

    static std::string command;
    command.clear();

    for (auto i = 0; i < int_iter->value; ++i)
    {
      if (!command.empty())
      {
        command.push_back(';');
      }
      command.append("bot_add");
    }

    config.save(custom_bindings);

    insert_string_setting_once(*args, L"exec", L"add_bots.cfg");
  }

  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }
  auto config = load_config_from_pak(L"base\\configs\\DEFAULT_KEYS.cfg", L"base/pak0.pak", L"base/pak0.pak/configs");

  if (config)
  {
    load_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 2> actions{
    { std::make_pair<WORD, std::string_view>(VK_RBUTTON, "+altattack"),
      std::make_pair<WORD, std::string_view>(VK_MBUTTON, "+weapon-extra") }
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

  auto config = load_config_from_pak(L"base\\configs\\DEFAULT_KEYS.cfg", L"base/pak0.pak", L"base/pak0.pak/configs");

  if (config)
  {
    load_keyboard_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 9> actions{
    {
      std::make_pair<WORD, std::string_view>('F', "+melee-attack"),
      std::make_pair<WORD, std::string_view>(VK_RETURN, "+use"),
      std::make_pair<WORD, std::string_view>('E', "+use"),
      std::make_pair<WORD, std::string_view>('G', "itemuse"),
      std::make_pair<WORD, std::string_view>('H', "itemnext"),
      std::make_pair<WORD, std::string_view>('Q', "+weapon-extra"),
      std::make_pair<WORD, std::string_view>(VK_TAB, "score"),
      std::make_pair<WORD, std::string_view>(VK_SPACE, "+moveup"),
      std::make_pair<WORD, std::string_view>(VK_LCONTROL, "+movedown"),
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_TRIGGER, "+altattack"),
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_X, "+use"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_SHOULDER, "itemnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_SHOULDER, "itemuse"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_DOWN, "weapondrop"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "weapprev"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_VIEW, "score"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_MENU, "menu objectives"),
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

  if (name_str == L"gl_mode")
  {
    static auto modes = std::array<predefined_int, 8>{
      predefined_int{ .label = L"640x480", .value = 3 },
      predefined_int{ .label = L"800x600", .value = 4 },
      predefined_int{ .label = L"960x720", .value = 5 },
      predefined_int{ .label = L"1024x768", .value = 6 },
      predefined_int{ .label = L"1152x864", .value = 7 },
      predefined_int{ .label = L"1280x960", .value = 8 },
      predefined_int{ .label = L"1600x1200", .value = 9 },
      predefined_int{},
    };

    return modes.data();
  }

  if (name_str == L"deathmatch")
  {
    static auto modes = std::array<predefined_int, 8>{
      predefined_int{ .label = L"Deathmatch", .value = 1 },
      predefined_int{ .label = L"Assassin", .value = 2 },
      predefined_int{ .label = L"Arsenal (aka Gun Game)", .value = 3 },
      predefined_int{ .label = L"Capture the Flag", .value = 4 },
      predefined_int{ .label = L"Realistic", .value = 5 },
      predefined_int{ .label = L"Control", .value = 6 },
      predefined_int{ .label = L"Conquer the Bunker", .value = 7 },
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
    return get_predefined_id_tech_2_map_command_line_settings(L"base", false);
  }

  return nullptr;
}
}

/*
      constexpr static auto heretic2_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 10> {{
        {playstation::l2, "+defend" },
        {playstation::l1, "defprev" },
        {playstation::r1, "defnext"},
        {playstation::circle, "+creep"},
        {playstation::square, "+action"},
        {playstation::start, "menu_objectives"},
        {playstation::select, "menu_city_map"}
    }};

    constexpr static auto heretic2_aliases = std::array<std::array<std::string_view, 2>, 2> {{
            {"+melee-attack", "use staff; +attack"},
            {"-melee-attack", "weapprev; -attack"}
    }};
*/
