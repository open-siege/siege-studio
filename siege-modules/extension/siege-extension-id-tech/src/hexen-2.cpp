import std;

import siege.configuration.id_tech;
import siege.configuration.shared;
import siege.extension.id_tech;
import siege.extension.id_tech.v2_0;
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
  .flags = { { L"listen", L"dedicated" } },
  .int_settings = { { L"width", L"height", L"vid_mode", L"chase_active", L"chase_back", L"chase_up", L"chase_right", L"joystick" } },// width and height are GL Hexen only
  .string_settings = { { L"name", L"connect", L"map", L"preferred_exe", L"game" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"listen",
  .dedicated_setting = L"dedicated",
  .selected_game_setting = L"game",
  .preferred_exe_setting = L"preferred_exe",
  .controller_enabled_setting = L"joystick",
  .selected_map_setting = L"map",
  .predefined_resolution_setting = L"vid_mode",
  .custom_width_setting = L"width",
  .custom_height_setting = L"height"
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

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"wsock32" } };

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

  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  auto config = load_config_from_file(L"data1\\Config.cfg")
                  .or_else([]() { return load_config_from_pak(L"data1\\default.cfg", L"data1/pak0.pak", L"data1/pak0.pak"); })
                  .value_or(siege::configuration::text_game_config(siege::configuration::id_tech::id_tech_2::save_config));

  unbind_joystick_for_quake_1_config(config);

  bool enable_controller = save_bindings_to_config(*args, config, mapping_context{ .index_to_axis = hardware_index_to_joystick_axis_id_tech_2_0, .axis_set_prefix = "" });

  if (enable_controller)
  {
    // engine bug - mouse needs to be enabled for the right analog stick to work
    config.emplace(siege::configuration::key_type({ "joyadvanced" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joysidesensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joypitchsensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joyadvancedupdate" }), siege::configuration::key_type(""));
  }

  std::ofstream custom_bindings("data1/Config.cfg", std::ios::binary | std::ios::trunc);
  config.save(custom_bindings);

  bind_axis_to_send_input(*args, "+lookup", "+mlook");
  bind_axis_to_send_input(*args, "+lookdown", "+mlook");

  insert_string_setting_once(*args, L"console", L"1");

  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (auto config = load_config_from_pak(L"data1\\default.cfg", L"data1/pak0.pak", L"data1/pak0.pak"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"data1\\Config.cfg"))
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

  if (auto config = load_config_from_pak(L"data1\\default.cfg", L"data1/pak0.pak", L"data1/pak0.pak"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"data1\\Config.cfg"))
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
      { vkey::gamepad_left_trigger, "invuse"},
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
      { vkey::gamepad_x, "inven"},
      { vkey::gamepad_y, "weapnext"},
      { vkey::gamepad_left_shoulder, "invright"},
      { vkey::gamepad_dpad_left, "impulse 12"},
      { vkey::gamepad_dpad_right, "impulse 10"},
      { vkey::gamepad_view, "+showdm"},
      { vkey::gamepad_menu, "+showinfo"},
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

std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_2_0_game_settings(args, {
    .packed_default_cfg = FSL"data1\\default.cfg",
    .packed_default_cfg_pak = FSL"data1/pak0.pak",
    .packed_default_cfg_pak_folder = FSL"data1/pak0.pak",
    .loose_user_cfg = FSL"data1\\Config.cfg",
  });
}
}
