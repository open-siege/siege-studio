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
  .int_settings = { { L"r_mode" } },
  .string_settings = { { L"name", L"connect", L"map" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .selected_map_setting = L"map",
  .predefined_resolution_setting = L"r_mode",
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
  game_action{ game_action::digital, "+melee-attack", u"Melee Attack", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "itemnext", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "itemuse", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "score", u"Score", u"Interface" },
  game_action{ game_action::digital, "menu-objectives", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"wsock32" } };

using namespace std::literals;

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

  if (exe_path.stem().string().starts_with("quakelive") && exe_path.extension() == ".exe" && std::filesystem::is_directory(parent_path / "baseq3", last_error))
  {
    return std::errc{};
  }

  return std::errc::not_supported;
}

std::errc apply_prelaunch_settings_ex(const wchar_t* exe_path_str, siege::platform::game_command_line_args_ex* args)
{
  if (exe_path_str == nullptr)
  {
    return std::errc::bad_address;
  }

  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  auto config = load_config_from_file(L"baseq3\\qzconfig.cfg")
                  .or_else([]() { return load_config_from_pk3(L"baseq3\\default.cfg", L"baseq3/pak00.pk3", L"baseq3/pak00.pk3"); })
                  .value_or(siege::configuration::text_game_config(siege::configuration::id_tech::id_tech_2::save_config));

  unbind_joystick_for_id_tech_3_0_config(config);
  save_bindings_to_config(*args, config);

  std::ofstream custom_bindings("baseq3/qzconfig.cfg", std::ios::binary | std::ios::trunc);
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
  auto config = load_config_from_pk3(L"baseq3\\default.cfg", L"baseq3/pak0.pak", L"baseq3/pak0.pak");

  if (config)
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

  auto config = load_config_from_pk3(L"baseq3\\default.cfg", L"baseq3/pak0.pak", L"baseq3/pak0.pak");

  if (config)
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

  if (name_str == L"r_mode")
  {
    static auto modes = std::array<predefined_int, 9>{
      predefined_int{ .label = L"640x480", .value = 1 },
      predefined_int{ .label = L"800x600", .value = 1 },
      predefined_int{ .label = L"960x720", .value = 1 },
      predefined_int{ .label = L"1024x768", .value = 1 },
      predefined_int{ .label = L"1152x864", .value = 1 },
      predefined_int{ .label = L"1280x960", .value = 1 },
      predefined_int{ .label = L"1600x1200", .value = 1 },
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
    return get_predefined_id_tech_3_map_command_line_settings(L"main");
  }

  return nullptr;
}
std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_3_0_game_settings(args, {
    .packed_default_cfg = FSL"baseq3\\default.cfg",
    .packed_default_cfg_pak = FSL"baseq3/pak00.pk3",
    .packed_default_cfg_pak_folder = FSL"baseq3/pak00.pk3",
    .loose_user_cfg = FSL"baseq3\\qzconfig.cfg",
  });
}
}
