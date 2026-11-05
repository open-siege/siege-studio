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

extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { L"dedicated", L"r_customwidth", L"r_customheight", L"r_mode", L"in_joystick", L"g_gametype", L"mp_gametype" } },
  .string_settings = { { L"name", L"connect", L"map", L"r_glDriver" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"g_gametype",
  .dedicated_setting = L"dedicated",
  .render_backend_setting = L"r_glDriver",
  .controller_enabled_setting = L"in_joystick",
  .selected_map_setting = L"map",
  .multiplayer_game_mode_setting = L"mp_gametype",
  .predefined_resolution_setting = L"r_mode",
  .custom_width_setting = L"r_customwidth",
  .custom_height_setting = L"r_customheight"
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

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"wsock32" } };

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

  auto config = load_config_from_file(L"base\\global\\defaultoptions.cfg")
                  .or_else([]() { return load_config_from_pk3(L"base\\global\\default.cfg", L"base/pak0.pk3", L"base/pak0.pk3/global"); })
                  .value_or(siege::configuration::text_game_config(siege::configuration::id_tech::id_tech_2::save_config));
  unbind_joystick_for_id_tech_3_0_config(config);

  save_bindings_to_config(*args, config, q3_mapping_context{});

  std::ofstream custom_bindings("base/global/defaultoptions.cfg", std::ios::binary | std::ios::trunc);
  config.save(custom_bindings);
  insert_string_setting_once(*args, L"console", L"1");
  auto iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name == nullptr; });
  auto connect_iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name != nullptr && setting.value != nullptr && setting.value[0] != '\0' && std::wstring_view(setting.name) == L"connect"; });

  if (connect_iter != args->string_settings.end())
  {
    // TODO do this at the top level
    static std::wstring address_port{};
    address_port = connect_iter->value;
    address_port += L":29254";
    connect_iter->value = address_port.c_str();
  }

  // TODO update to use new setting functions
  auto server_iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name != nullptr && setting.value != nullptr && setting.value[0] != '\0' && (std::wstring_view(setting.name) == L"dedicated" || std::wstring_view(setting.name) == L"connect" || std::wstring_view(setting.name) == L"map"); });

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

  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (auto config = load_config_from_pk3(L"base\\global\\default.cfg", L"base/pak0.pk3", L"main/pak0.pk3/global"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  // TODO load player config dynamically

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

  if (auto config = load_config_from_pk3(L"base\\global\\default.cfg", L"base/pak0.pk3", L"main/pak0.pk3/global"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

    // TODO load player config dynamically

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
      { vkey::gamepad_right_trigger, "+attackleft"},
      { vkey::gamepad_left_trigger, "attackright"},
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
      { vkey::gamepad_right_thumbstick_button, "+melee-attack"},
      { vkey::gamepad_x, "inven"},
      { vkey::gamepad_y, "nextinv"},
      { vkey::gamepad_left_shoulder, "invnext"},
      { vkey::gamepad_right_shoulder, "+throw-grenade"},
      { vkey::gamepad_dpad_down, "weapondrop"},
      { vkey::gamepad_dpad_left, "weapprev"},
      { vkey::gamepad_dpad_right, "weapnext"},
      { vkey::gamepad_view, "+objectives_score"},
      { vkey::gamepad_menu, "togglemenu"},
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

  if (name_str == L"mp_gametype")
  {
    static auto modes = std::array<predefined_int, 5>{
      predefined_int{ .label = L"Holomatch", .value = 0 },
      predefined_int{ .label = L"Team Holomatch", .value = 1 },
      predefined_int{ .label = L"Capture the Flag", .value = 2 },
      predefined_int{ .label = L"Bomb Defusion", .value = 3 },
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

  return nullptr;
}
std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_3_0_game_settings(args, {
    .packed_default_cfg = FSL"base\\global\\default.cfg",
    .packed_default_cfg_pak = FSL"base/pak0.pk3",
    .packed_default_cfg_pak_folder = FSL"base/pak0.pk3/global",
    .loose_user_cfg = FSL"base\\global\\defaultoptions.cfg",
  });
}
}
