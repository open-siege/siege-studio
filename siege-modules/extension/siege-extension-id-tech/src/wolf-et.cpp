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
  .int_settings = { { L"dedicated", L"r_mode", L"in_joystick" } },
  .string_settings = { { L"name", L"connect", L"map", L"r_glDriver" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"connect",
  .dedicated_setting = L"dedicated",
  .render_backend_setting = L"r_glDriver",
  .controller_enabled_setting = L"in_joystick",
  .selected_map_setting = L"map",
  .predefined_resolution_setting = L"r_mode"
};

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+back", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+moveleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+moveright", u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, "+moveup", u"Jump", u"Movement" },
  game_action{ game_action::analog, "+movedown", u"Crouch", u"Movement" },
  game_action{ game_action::digital, "+sprint", u"Spring", u"Movement" },
  game_action{ game_action::analog, "+left", u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, "+right", u"Turn Right", u"Aiming" },
  game_action{ game_action::analog, "+lookup", u"Look Up", u"Aiming" },
  game_action{ game_action::analog, "+lookdown", u"Look Down", u"Aiming" },
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "weapalt", u"Alt Attack", u"Combat" },
  game_action{ game_action::digital, "vstr melee-toggle", u"Toggle Melee", u"Combat" },
  game_action{ game_action::digital, "vstr grenade-toggle", u"Toggle Grenade", u"Combat" },
  game_action{ game_action::digital, "vstr pistol-toggle", u"Toggle Pistol", u"Combat" },
  game_action{ game_action::digital, "+zoom", u"Binocular Zoom", u"Combat" },
  game_action{ game_action::digital, "+dropweapon", u"Drop Weapon", u"Combat" },
  game_action{ game_action::digital, "+reload", u"Drop Weapon", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "+scores", u"Score", u"Interface" },
  game_action{ game_action::digital, "OpenLimboMenu", u"Limbo Menu", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

constexpr static auto wolf_aliases = std::array<std::array<std::string_view, 3>, 9>{
  { { "melee-toggle", "vstr melee-toggle-start" },
    { "melee-toggle-start", "weaponbank 1;set melee-toggle vstr melee-toggle-stop;" },
    { "melee-toggle-stop", "weaplastused;set melee-toggle vstr melee-toggle-start" },
    { "grenade-toggle", "vstr grenade-toggle-start" },
    { "grenade-toggle-start", "weaponbank 4;set grenade-toggle vstr grenade-toggle-stop" },
    { "grenade-toggle-stop", "weaplastused;set grenade-toggle vstr grenade-toggle-start" },
    { "pistol-toggle", "vstr pistol-toggle-start" },
    { "pistol-toggle-start", "weaponbank 2;set pistol-toggle vstr pistol-toggle-stop" },
    { "pistol-toggle-stop", "weaplastused;set pistol-toggle vstr pistol-toggle-start" } }
};

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"wsock32" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x55d2e0) },
  { "cmdlist"sv, std::size_t(0x55d2d8) },
  { "cl_pitchspeed"sv, std::size_t(0x55add4) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 4> function_name_ranges{ {
  { "+moveup"sv, "help"sv },
  { "condump"sv, "clear"sv },
  { "killserv"sv, "sectorlist"sv },
  { "bind"sv, "bindlist"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 5> variable_name_ranges{ {
  { "com_buildScript"sv, "com_recommendedSet"sv },
  { "dedicated"sv, "sv_running"sv },
  { "com_showtrace"sv, "com_cameraMode"sv },
  { "cg_wolfparticles"sv, "cl_doubletapdelay"sv },
  { "cm_noAreas"sv, "cm_optimize"sv },
} };

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

  std::ofstream custom_bindings("etmain/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);
  unbind_joystick_for_quake_3_config(config);

  for (auto& alias : wolf_aliases)
  {
    config.emplace(key_type({ "seta", alias[0] }), (alias[1]));
  }

  save_bindings_to_config(*args, config, q3_mapping_context{});

  config.save(custom_bindings);

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

  if (auto config = load_config_from_pk3(L"etmain\\default.cfg", L"etmain/pak0.pk3", L"etmain/pak0.pk3"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"etmain\\etconfig.cfg"))
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

  if (auto config = load_config_from_pk3(L"etmain\\default.cfg", L"etmain/pak0.pk3", L"etmain/pak0.pk3"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"etmain\\etconfig.cfg"))
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
  std::array<std::pair<vkey, std::string_view>, 23> actions{
    {
      { vkey::gamepad_right_trigger, "+attack"},
      { vkey::gamepad_left_trigger, "weapalt"},
      { vkey::gamepad_a, "+moveup"},
      { vkey::gamepad_b, "+movedown"},
      { vkey::gamepad_x, "+reload"},
      { vkey::gamepad_y, "vstr pistol-toggle"},
      { vkey::gamepad_left_thumbstick_button, "+sprint"},
      { vkey::gamepad_left_thumbstick_up, "+forward"},
      { vkey::gamepad_left_thumbstick_down, "+back"},
      { vkey::gamepad_left_thumbstick_left, "+moveleft"},
      { vkey::gamepad_left_thumbstick_right, "+moveright"},
      { vkey::gamepad_right_thumbstick_left, "+left"},
      { vkey::gamepad_right_thumbstick_right, "+right"},
      { vkey::gamepad_right_thumbstick_up, "+lookup"},
      { vkey::gamepad_right_thumbstick_down, "+lookdown"},
      { vkey::gamepad_right_thumbstick_button, "vstr melee-toggle"},
      { vkey::gamepad_left_shoulder, "+zoom"},
      { vkey::gamepad_right_shoulder, "vstr grenade-toggle"},
      { vkey::gamepad_dpad_down, "+dropweapon"},
      { vkey::gamepad_dpad_left, "weapprev"},
      { vkey::gamepad_dpad_right, "weapnext"},
      { vkey::gamepad_view, "+scores"},
      { vkey::gamepad_menu, "OpenLimboMenu"},
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
    static auto modes = std::array<predefined_int, 8>{
      predefined_int{ .label = L"640x480", .value = 1 },
      predefined_int{ .label = L"800x600", .value = 1 },
      predefined_int{ .label = L"960x720", .value = 1 },
      predefined_int{ .label = L"1024x768", .value = 1 },
      predefined_int{ .label = L"1152x864", .value = 1 },
      predefined_int{ .label = L"1280x960", .value = 1 },
      predefined_int{ .label = L"1600x1200", .value = 1 },
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
    return get_predefined_id_tech_3_map_command_line_settings(L"etmain");
  }

  return nullptr;
}
std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_3_0_game_settings(args, {
    .packed_default_cfg = FSL"etmain\\default.cfg",
    .packed_default_cfg_pak = FSL"etmain/pak0.pk3",
    .packed_default_cfg_pak_folder = FSL"etmain/pak0.pk3",
    .loose_user_cfg = FSL"etmain\\etconfig.cfg",
  });
}
}
