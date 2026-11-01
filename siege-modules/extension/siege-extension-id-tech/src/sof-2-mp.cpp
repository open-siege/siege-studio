import std;

import siege.configuration.id_tech;
import siege.configuration.shared;
import siege.extension.id_tech;
import siege.extension.id_tech.v3_0;
import siege.extension.shared;
import siege.platform;
import siege.platform.shared;

extern "C" {
using hardware_context = siege::platform::hardware_context;
using game_action = siege::platform::game_action;
using keyboard_binding = siege::platform::keyboard_binding;
using mouse_binding = siege::platform::mouse_binding;
using controller_binding = siege::platform::controller_binding;
using vkey = siege::platform::vkey;
using game_action = siege::platform::game_action;
using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
using key_type = siege::configuration::key_type;

extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { L"dedicated", L"r_customwidth", L"r_customheight", L"r_mode", L"net_noipx", L"r_fullscreen", L"cg_autoReload", L"cg_drawGun", L"cg_fov", L"scorelimit", L"timelimit", L"in_joystick" } },
  .string_settings = { { L"name", L"connect", L"map", L"r_glDriver", L"g_gametype" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"net_noipx",
  .dedicated_setting = L"dedicated",
  .render_backend_setting = L"r_glDriver",
  .controller_enabled_setting = L"in_joystick",
  .selected_map_setting = L"map",
  .max_time_limit_setting = L"timelimit",
  .max_score_limit_setting = L"scorelimit",
  .multiplayer_game_mode_setting = L"g_gametype",
  .predefined_resolution_setting = L"r_mode",
  .custom_width_setting = L"r_customwidth",
  .custom_height_setting = L"r_customheight",
  .fullscreen_setting = L"r_fullscreen",
  .fov_setting = L"cg_fov"
};

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+back", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+moveleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+moveright", u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, "+moveup", u"Jump", u"Movement" },
  game_action{ game_action::analog, "+movedown", u"Crouch", u"Movement" },
  game_action{ game_action::digital, "+lean", u"Lean", u"Movement" },
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
  game_action{ game_action::digital, "weapprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "drop", u"Drop Weapon", u"Combat" },
  game_action{ game_action::digital, "+firemode", u"Change Fire Mode", u"Combat" },
  game_action{ game_action::digital, "+scores", u"Score", u"Interface" },
  game_action{ game_action::digital, "ui_objectives", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "ui_outfitting", u"Weapon Selection", u"Interface" },
  game_action{ game_action::digital, "ui_team", u"Team Selection", u"Interface" },
  game_action{ game_action::digital, "+automap", u"Automap", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"wsock32" } };

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
    { "pistol-toggle-stop", "weaplast;set pistol-toggle vstr pistol-toggle-start" } }
};

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

  auto config = load_config_from_file(L"base\\sof2mp.cfg")
                  .or_else([]() { return load_config_from_pk3(L"base\\sof2mp_default.cfg", L"base/mp.pk3", L"base/mp.pk3"); })
                  .value_or(siege::configuration::text_game_config(siege::configuration::id_tech::id_tech_2::save_config));
  unbind_joystick_for_quake_3_config(config);

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

  save_bindings_to_config(*args, config, q3_mapping_context{});

  std::ofstream custom_bindings("base/sof2mp.cfg", std::ios::binary | std::ios::trunc);
  config.save(custom_bindings);
  insert_string_setting_once(*args, L"console", L"1");

  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (auto config = load_config_from_pk3(L"base\\sof2mp_default.cfg", L"base/mp.pk3", L"base/mp.pk3"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"base\\sof2mp.cfg"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<vkey, std::string_view>, 4> axes{
    {
      { vkey::up, "+lookup"},
      { vkey::down, "+lookdown"},
      { vkey::left, "+left"},
      { vkey::right, "+right"},
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

  if (auto config = load_config_from_pk3(L"base\\sof2mp_default.cfg", L"base/mp.pk3", L"base/mp.pk3"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"base\\sof2mp.cfg"))
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
      { vkey::gamepad_right_trigger, "+attack"},
      { vkey::gamepad_left_trigger, "+altattack"},
      { vkey::gamepad_a, "+moveup"},
      { vkey::gamepad_b, "+movedown"},
      { vkey::gamepad_left_thumbstick_button, "+speed"},
      { vkey::gamepad_left_thumbstick_up, "+forward"},
      { vkey::gamepad_left_thumbstick_down, "+back"},
      { vkey::gamepad_left_thumbstick_left, "+moveleft"},
      { vkey::gamepad_left_thumbstick_right, "+moveright"},
      { vkey::gamepad_right_thumbstick_left, "+left"},
      { vkey::gamepad_right_thumbstick_right, "+right"},
      { vkey::gamepad_right_thumbstick_up, "+lookup"},
      { vkey::gamepad_right_thumbstick_down, "+lookdown"},
      { vkey::gamepad_right_thumbstick_button, "vstr melee-toggle"},
      { vkey::gamepad_x, "+lean"},
      { vkey::gamepad_y, "vstr pistol-toggle"},
      { vkey::gamepad_left_shoulder, "+firemode"},
      { vkey::gamepad_right_shoulder, "vstr grenade-toggle"},
      { vkey::gamepad_dpad_down, "drop"},
      { vkey::gamepad_dpad_left, "weapprev"},
      { vkey::gamepad_dpad_right, "weapnext"},
      { vkey::gamepad_view, "+automap"},
      { vkey::gamepad_menu, "ui_objectives"},
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
std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_3_0_game_settings(args, {
    .packed_default_cfg = FSL"base\\sof2mp_default.cfg",
    .packed_default_cfg_pak = FSL"base/mp.pk3",
    .packed_default_cfg_pak_folder = FSL"base/mp.pk3",
    .loose_user_cfg = FSL"base\\sof2mp.cfg",
  });
}
}
