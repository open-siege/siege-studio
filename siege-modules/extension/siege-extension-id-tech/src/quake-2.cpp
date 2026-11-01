import std;

import siege.configuration.id_tech;
import siege.configuration.shared;
import siege.extension.id_tech;
import siege.extension.id_tech.v2_5;
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
  .int_settings = { { L"gl_mode", L"dedicated", L"in_joystick" } },
  .string_settings = { { L"name", L"connect", L"map", L"gl_driver", L"game" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"connect",
  .dedicated_setting = L"dedicated",
  .render_backend_setting = L"gl_driver",
  .selected_game_setting = L"game",
  .controller_enabled_setting = L"in_joystick",
  .selected_map_setting = L"map",
  .predefined_resolution_setting = L"gl_mode"
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
  game_action{ game_action::digital, "+throw-grenade", u"Throw Grenade", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "invnext", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "inven", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "score", u"Score", u"Interface" },
  game_action{ game_action::digital, "cmd help", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

constexpr static auto quake2_aliases = std::array<std::array<std::string_view, 2>, 2>{ { { "+throw-grenade", "use grenades; +attack" },
  { "-throw-grenade", "-attack;weaplast" } } };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"wsock32" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x44ec2c) },
  { "cmdlist"sv, std::size_t(0x44ec34) },
  { "cl_pitchspeed"sv, std::size_t(0x44d5a8) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 6> function_name_ranges{ {
  { "-klook"sv, "centerview"sv },
  { "echo"sv, "print"sv },
  { "gun_model"sv, "gun_next"sv },
  { "wait"sv, "cmdlist"sv },
  { "-mlook"sv, "+mlook"sv },
  { "menu_quit"sv, "menu_quit"sv },

} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 1> variable_name_ranges{ { { "in_joystick"sv, "in_mouse"sv } } };

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  if (filename && std::filesystem::path(filename).wstring().contains(L"quake2"))
  {
    return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
  }

  return std::errc::not_supported;
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

  std::ofstream custom_bindings("baseq2/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);
  unbind_joystick_for_quake_2_config(config);

  for (auto& alias : quake2_aliases)
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

    config.emplace(siege::configuration::key_type({ "joy_advancedupdate" }), siege::configuration::key_type{});
  }

  config.save(custom_bindings);

  bind_axis_to_send_input(*args, "+lookup", "+mlook");
  bind_axis_to_send_input(*args, "+lookdown", "+mlook");

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

  if (auto config = load_config_from_pak(L"baseq2\\default.cfg", L"baseq2/pak0.pak", L"baseq2/pak0.pak"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"baseq2\\config.cfg"))
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

  if (auto config = load_config_from_pak(L"baseq2\\default.cfg", L"baseq2/pak0.pak", L"baseq2/pak0.pak"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"baseq2\\config.cfg"))
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
      { vkey::gamepad_right_thumbstick_button, "+melee-attack"},
      { vkey::gamepad_x, "inven"},
      { vkey::gamepad_y, "weapnext"},
      { vkey::gamepad_left_shoulder, "invnext"},
      { vkey::gamepad_right_shoulder, "+throw-grenade"},
      { vkey::gamepad_dpad_down, "weapondrop"},
      { vkey::gamepad_dpad_left, "weapprev"},
      { vkey::gamepad_dpad_right, "weapnext"},
      { vkey::gamepad_view, "score"},
      { vkey::gamepad_menu, "cmd help"},
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

  return nullptr;
}

predefined_string*
  get_predefined_string_command_line_settings(const wchar_t* name) noexcept
{
  if (name && std::wstring_view(name) == L"map")
  {
    return get_predefined_id_tech_2_map_command_line_settings(L"baseq2", false);
  }

  return nullptr;
}

std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_2_5_game_settings(args, {
    .packed_default_cfg = FSL"baseq2\\default.cfg",
    .packed_default_cfg_pak = FSL"baseq2/pak0.pak",
    .packed_default_cfg_pak_folder = FSL"baseq2/pak0.pak",
    .loose_user_cfg = FSL"baseq2\\config.cfg",
  });
}
}
