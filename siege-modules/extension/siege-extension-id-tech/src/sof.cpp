import std;

import siege.configuration.id_tech;
import siege.configuration.shared;
import siege.extension.id_tech;
import siege.extension.id_tech.v2_5;
import siege.extension.shared;
import siege.platform;
import siege.platform.shared;

using hardware_context = siege::platform::hardware_context;
using game_action = siege::platform::game_action;
using keyboard_binding = siege::platform::keyboard_binding;
using mouse_binding = siege::platform::mouse_binding;
using controller_binding = siege::platform::controller_binding;
using vkey = siege::platform::vkey;
using fs_char = siege::fs_char;
using fs_string_view = siege::fs_string_view;

using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const fs_char*>;
namespace fs = std::filesystem;

extern "C" {
extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { FSL "dedicated", FSL "deathmatch", FSL "timelimit", FSL "fraglimit", FSL "maxclients", FSL "numbots", FSL "gl_mode", FSL "in_joystick" } },
  .string_settings = { { FSL "name", FSL "connect", FSL "map", FSL "gl_driver", FSL "game" } },
  .ip_connect_setting = FSL "connect",
  .player_name_setting = FSL "name",
  .listen_setting = FSL "connect",
  .dedicated_setting = FSL "dedicated",
  .render_backend_setting = FSL "gl_driver",
  .selected_game_setting = FSL "game",
  .controller_enabled_setting = FSL "in_joystick",
  .selected_map_setting = FSL "map",
  .max_players_setting = FSL "maxclients",
  .bot_count_setting = FSL "numbots",
  .max_time_limit_setting = FSL "timelimit",
  .max_score_limit_setting = FSL "fraglimit",
  .multiplayer_game_mode_setting = FSL "deathmatch",
  .predefined_resolution_setting = FSL "gl_mode"
};

extern auto controller_input_backends = std::array<const fs_char*, 2>{ { FSL "winmm" } };

extern auto network_backends = std::array<const fs_char*, 2>{ { FSL "wsock32" } };

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

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x20120494) },
  { "cmdlist"sv, std::size_t(0x2012049c) },
  { "cl_minfps"sv, std::size_t(0x2011e600) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 3> function_name_ranges{ { { "-klook"sv, "centerview"sv },
  { "joy_advancedupdate"sv, "+mlook"sv },
  { "rejected_violence"sv, "print"sv } } };

constexpr static auto sof_aliases = std::array<std::array<std::string_view, 2>, 8>{ { { "+melee-attack", "weaponselect 1; +attack" },
  { "-melee-attack", "weaponbestsafe; -attack" },
  { "+weapon-extra1", "+weaponextra1;" },
  { "-weapon-extra1", "-weaponextra1;alias +weapon-extra +weapon-extra2; alias -weapon-extra -weapon-extra2" },
  { "+weapon-extra2", "+weaponextra2;" },
  { "-weapon-extra2", "-weaponextra2;alias +weapon-extra +weapon-extra1; alias -weapon-extra -weapon-extra1" },
  { "+weapon-extra", "+weapon-extra1" },
  { "-weapon-extra", "-weapon-extra1" } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 1> variable_name_ranges{ { { "joy_yawsensitivity"sv, "in_mouse"sv } } };

std::errc executable_is_supported(const fs_char* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
}

std::errc apply_prelaunch_settings_ex(const fs_char* exe_path_str, siege::platform::game_command_line_args_ex* args)
{
  if (exe_path_str == nullptr)
  {
    return std::errc::bad_address;
  }

  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  auto config = load_config_from_file(FSL "user\\config.cfg")
                  .or_else([]() { return load_config_from_pak(FSL "base\\configs\\default.cfg", FSL "base/pak0.pak", FSL "base/pak0.pak"); })
                  .value_or(siege::configuration::text_game_config(siege::configuration::id_tech::id_tech_2::save_config));

  unbind_joystick_for_id_tech_2_5_config(config);


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


  std::ofstream custom_bindings("user/config.cfg", std::ios::binary | std::ios::trunc);
  config.save(custom_bindings);

  write_aliases_to_autoexec(FSL"user\\autoexec.cfg", sof_aliases);

  bind_axis_to_send_input(*args, "+lookup", "+mlook");
  bind_axis_to_send_input(*args, "+lookdown", "+mlook");

  insert_string_setting_once(*args, FSL "console", FSL "1");

  auto int_iter = std::find_if(args->int_settings, args->int_settings + args->int_settings_size, [](auto& setting) { return setting.name != nullptr && setting.name == fs_string_view(FSL "numbots"); });

  if (int_iter != args->int_settings + args->int_settings_size)
  {
    std::ofstream bots_stream("user/add_bots.cfg", std::ios::binary | std::ios::trunc);

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

    config.save(bots_stream);

    insert_string_setting_once(*args, FSL "exec", FSL "add_bots.cfg");
  }

  return std::errc{};
}

std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_2_5_game_settings(args, {
                                                .packed_default_cfg = FSL "base\\default.cfg",
                                                .packed_default_cfg_pak = FSL "base/pak0.pak",
                                                .packed_default_cfg_pak_folder = FSL "base/pak0.pak",
                                                .loose_user_cfg = FSL "user\\config.cfg",
                                              });
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (auto config = load_config_from_pak(FSL "base\\configs\\DEFAULT_KEYS.cfg", FSL "base/pak0.pak", FSL "base/pak0.pak/configs"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(FSL "user\\config.cfg"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<vkey, std::string_view>, 4> axes{
    {
      { vkey::up, "+lookup" },
      { vkey::down, "+lookdown" },
      { vkey::left, "+left" },
      { vkey::right, "+right" },
    }
  };

  upsert_mouse_axis_defaults(game_actions, axes, *binding);

  return std::errc{};
}

std::errc init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (auto config = load_config_from_pak(FSL "base\\configs\\DEFAULT_KEYS.cfg", FSL "base/pak0.pak", FSL "base/pak0.pak/configs"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(FSL "user\\config.cfg"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

  return std::errc{};
}

std::errc default_controller_inputs(controller_binding* binding, std::uint32_t layout_index)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (layout_index > 0)
  {
    return std::errc::invalid_argument;
  }

  std::array<std::pair<vkey, std::string_view>, 23> actions{
    {
      { vkey::gamepad_right_trigger, "+attack" },
      { vkey::gamepad_left_trigger, "+altattack" },
      { vkey::gamepad_a, "+moveup" },
      { vkey::gamepad_b, "+movedown" },
      { vkey::gamepad_left_thumbstick_button, "+speed" },
      { vkey::gamepad_left_thumbstick_up, "+forward" },
      { vkey::gamepad_left_thumbstick_down, "+back" },
      { vkey::gamepad_left_thumbstick_left, "+moveleft" },
      { vkey::gamepad_left_thumbstick_right, "+moveright" },
      { vkey::gamepad_right_thumbstick_left, "+left" },
      { vkey::gamepad_right_thumbstick_right, "+right" },
      { vkey::gamepad_right_thumbstick_up, "+lookup" },
      { vkey::gamepad_right_thumbstick_down, "+lookdown" },
      { vkey::gamepad_right_thumbstick_button, "+melee-attack" },
      { vkey::gamepad_x, "+use" },
      { vkey::gamepad_y, "weapnext" },
      { vkey::gamepad_left_shoulder, "itemnext" },
      { vkey::gamepad_right_shoulder, "itemuse" },
      { vkey::gamepad_dpad_down, "weapondrop" },
      { vkey::gamepad_dpad_left, "weapprev" },
      { vkey::gamepad_dpad_right, "weapnext" },
      { vkey::gamepad_view, "score" },
      { vkey::gamepad_menu, "menu objectives" },
    }
  };

  append_controller_defaults(game_actions, actions, *binding);

  return std::errc{};
}

predefined_int*
  get_predefined_int_command_line_settings(const fs_char* name) noexcept
{
  if (name == nullptr)
  {
    return nullptr;
  }

  auto name_str = fs_string_view(name);

  if (name_str == FSL "gl_mode")
  {
    static auto modes = std::array<predefined_int, 8>{
      predefined_int{ .label = FSL "640x480", .value = 3 },
      predefined_int{ .label = FSL "800x600", .value = 4 },
      predefined_int{ .label = FSL "960x720", .value = 5 },
      predefined_int{ .label = FSL "1024x768", .value = 6 },
      predefined_int{ .label = FSL "1152x864", .value = 7 },
      predefined_int{ .label = FSL "1280x960", .value = 8 },
      predefined_int{ .label = FSL "1600x1200", .value = 9 },
      predefined_int{},
    };

    return modes.data();
  }

  if (name_str == FSL "deathmatch")
  {
    static auto modes = std::array<predefined_int, 8>{
      predefined_int{ .label = FSL "Deathmatch", .value = 1 },
      predefined_int{ .label = FSL "Assassin", .value = 2 },
      predefined_int{ .label = FSL "Arsenal (aka Gun Game)", .value = 3 },
      predefined_int{ .label = FSL "Capture the Flag", .value = 4 },
      predefined_int{ .label = FSL "Realistic", .value = 5 },
      predefined_int{ .label = FSL "Control", .value = 6 },
      predefined_int{ .label = FSL "Conquer the Bunker", .value = 7 },
      predefined_int{},
    };

    return modes.data();
  }

  return nullptr;
}

predefined_string*
  get_predefined_string_command_line_settings(const fs_char* name) noexcept
{
  if (name && fs_string_view(name) == FSL "map")
  {
    return get_predefined_id_tech_2_map_command_line_settings(FSL "base", false);
  }

  return nullptr;
}
}
