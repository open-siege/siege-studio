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
  .int_settings = { { L"r_customwidth", L"r_customheight", L"r_mode", L"in_joystick" } },
  .string_settings = { { L"name", L"connect", L"map" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .controller_enabled_setting = L"in_joystick",
  .selected_map_setting = L"map",
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
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "+secondary", u"Alt Attack", u"Combat" },
  game_action{ game_action::digital, "+reload", u"Reload", u"Combat" },
  game_action{ game_action::digital, "changeweapons", u"Change Loadout", u"Combat" },
  game_action{ game_action::digital, "togglerts", u"Toggle RTS Mode", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weapprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "lastweapon", u"Quick Swap", u"Combat" },
  game_action{ game_action::digital, "+scores", u"Score", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"wsock32" } };

using namespace std::literals;

constexpr static std::array<std::pair<std::string_view, std::string_view>, 0> function_name_ranges{};

constexpr static std::array<std::pair<std::string_view, std::string_view>, 0> variable_name_ranges{};

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  if (filename == nullptr)
  {
    return std::errc::bad_address;
  }

  std::error_code last_error;

  if (!std::filesystem::exists(filename, last_error))
  {
    return std::errc::invalid_argument;
  }

  auto exe_path = std::filesystem::path(filename);
  auto parent_path = exe_path.parent_path();

  if (exe_path.stem().string().starts_with("igwarlord") && exe_path.extension() == ".exe" && std::filesystem::is_directory(parent_path / "base", last_error))
  {
    return std::errc{};
  }

  return std::errc::not_supported;
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

  std::ofstream custom_bindings("base/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);
  unbind_joystick_for_quake_3_config(config);

  bool enable_controller = save_bindings_to_config(*args, config, q3_mapping_context{});

  if (enable_controller)
  {
    config.emplace(siege::configuration::key_type({ "set", "in_joystick" }), siege::configuration::key_type("1"));
  }

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

  if (auto config = load_config_from_pak(L"base\\default.cfg", L"base/resources.pk3", L"base/resources.pk3"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"base\\config.cfg"))
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

  if (auto config = load_config_from_pak(L"base\\default.cfg", L"base/resources.pk3", L"base/resources.pk3"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"base\\config.cfg"))
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

  std::array<std::pair<vkey, std::string_view>, 24> actions{
    {
      { vkey::gamepad_right_trigger, "+attack"},
      { vkey::gamepad_left_trigger, "+secondary"},
      { vkey::gamepad_a, "+moveup"},
      { vkey::gamepad_b, "+movedown"},
      { vkey::gamepad_x, "+use"},
      { vkey::gamepad_y, "lastweapon"},
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
      { vkey::gamepad_left_shoulder, "togglerts"},
      { vkey::gamepad_right_shoulder, "+reload"},
      { vkey::gamepad_dpad_up, "vote yes"},
      { vkey::gamepad_dpad_down, "vote no"},
      { vkey::gamepad_dpad_left, "weapprev"},
      { vkey::gamepad_dpad_right, "weapnext"},
      { vkey::gamepad_view, "+scores"},
      { vkey::gamepad_menu, "changeweapons"},
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
    return get_predefined_id_tech_3_map_command_line_settings(L"base");
  }

  return nullptr;
}
std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_3_0_game_settings(args, {
    .packed_default_cfg = FSL"base\\default.cfg",
    .packed_default_cfg_pak = FSL"base/resources.pk3",
    .packed_default_cfg_pak_folder = FSL"base/resources.pk3",
    .loose_user_cfg = FSL"base\\config.cfg",
  });
}
}
