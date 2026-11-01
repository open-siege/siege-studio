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
  .int_settings = { { L"width", L"height", L"joystick" } },
  .string_settings = { { L"name", L"map" } },
  .player_name_setting = L"name",
  .controller_enabled_setting = L"joystick",
  .selected_map_setting = L"map",
  .custom_width_setting = L"width",
  .custom_height_setting = L"height"
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
  game_action{ game_action::digital, "+zoom", u"Zoom", u"Combat" },
  game_action{ game_action::digital, "+weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "reload", u"Reload", u"Combat" },
  game_action{ game_action::digital, "+use", u"Use/Interact", u"Combat" },
  game_action{ game_action::digital, "toggleinfo", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"none" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "SoloMissions"sv, std::size_t(0x487fe0) },
  { "exec"sv, std::size_t(0x486278) },
  { "cmdline"sv, std::size_t(0x486564) },
  { "cl_pitchspeed"sv, std::size_t(0x47f95c) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 2> function_name_ranges{ {
  { "toggleinfo"sv, "timedemo"sv },
  { "name"sv, "mcache"sv },
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

  auto config = load_config_from_file(L"main\\config.cfg")
                  .or_else([]() { return load_config_from_pak(L"main\\default.cfg", L"main/pak0.pak", L"main/pak0.pak"); })
                  .value_or(siege::configuration::text_game_config(siege::configuration::id_tech::id_tech_2::save_config));

  unbind_joystick_for_quake_1_config(config);

  bool enable_controller = save_bindings_to_config(*args, config, mapping_context{ .index_to_axis = hardware_index_to_joystick_axis_id_tech_2_0, .axis_set_prefix = "" });

  if (enable_controller)
  {
    config.emplace(siege::configuration::key_type({ "joyadvanced" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joysidesensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joypitchsensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joyadvancedupdate" }), siege::configuration::key_type(""));
  }

  std::ofstream custom_bindings("main/config.cfg", std::ios::binary | std::ios::trunc);
  config.save(custom_bindings);

  bind_axis_to_send_input(*args, "+lookup", "+mlook");
  bind_axis_to_send_input(*args, "+lookdown", "+mlook");

  auto free_iter = std::find_if(args->flags.begin(), args->flags.end(), [](auto& setting) { return setting == nullptr; });
  *free_iter = L"notwarezed";

  std::advance(free_iter, 1);
  *free_iter = L"console";

  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (auto config = load_config_from_pak(L"main\\default.cfg", L"main/pak0.pak", L"main/pak0.pak"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"main\\config.cfg"))
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

  if (auto config = load_config_from_pak(L"main\\default.cfg", L"main/pak0.pak", L"main/pak0.pak"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

  if (auto config = load_config_from_file(L"main\\config.cfg"))
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
      { vkey::gamepad_left_trigger, "+zoom"},
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
      { vkey::gamepad_x, "+use"},
      { vkey::gamepad_y, "weapnext"},
      { vkey::gamepad_dpad_left, "weapprev"},
      { vkey::gamepad_dpad_right, "weapnext"},
      { vkey::gamepad_menu, "toggleinfo"},
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

  return nullptr;
}

predefined_string*
  get_predefined_string_command_line_settings(const wchar_t* name) noexcept
{
  if (name && std::wstring_view(name) == L"map")
  {
    return get_predefined_id_tech_2_map_command_line_settings(L"main", false);
  }

  return nullptr;
}

std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_2_0_game_settings(args, {
    .packed_default_cfg = FSL"main\\default.cfg",
    .packed_default_cfg_pak = FSL"main/pak0.pak",
    .packed_default_cfg_pak_folder = FSL"main/pak0.pak",
    .loose_user_cfg = FSL"main\\config.cfg",
  });
}
}
