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
using game_action = siege::platform::game_action;
using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
namespace fs = std::filesystem;

extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { L"gl_mode" } },
  .string_settings = { { L"name", L"map", L"gl_driver" } },
  .player_name_setting = L"name",
  .render_backend_setting = L"gl_driver",
  .selected_map_setting = L"map",
  .predefined_resolution_setting = L"gl_mode",
};

// TODO when it comes to controller support
// the game doesn't have it, so we will just do keyboard + mouse emulation
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
  game_action{ game_action::digital, "+actiona", u"Primary Action", u"Combat" },
  game_action{ game_action::digital, "+actionb", u"Secondary Action", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "itemnext", u"Crouch", u"Combat" },
  game_action{ game_action::digital, "itemuse", u"Crouch", u"Combat" },
  game_action{ game_action::digital, "+use", u"Crouch", u"Combat" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"none" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{
  { { "exec"sv, std::size_t(0x461274) },
    { "cmdlist"sv, std::size_t(0x46126c) },
    { "cl_minfps"sv, std::size_t(0x45e6c0) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 3> function_name_ranges{ { { "+moveup"sv, "-actionb"sv },
  { "+mlook"sv, "-mlook"sv },
  { "echo"sv, "echo"sv } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 1> variable_name_ranges{ { { "in_initmouse"sv, "in_joystick"sv } } };

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

  auto config = load_config_from_file(L"anoxdata\\CONFIGS\\anox.cfg")
                  .or_else([]() { return load_config_from_pak(L"anoxdata\\CONFIGS\\Default.cfg", L"anoxdata/SCRIPTS.dat", L"anoxdata/SCRIPTS.dat"); })
                  .value_or(siege::configuration::text_game_config(siege::configuration::id_tech::id_tech_2::save_config));

  unbind_joystick_for_quake_2_config(config);

  save_bindings_to_config(*args, config);


  std::ofstream custom_bindings("anoxdata/CONFIGS/anox.cfg", std::ios::binary | std::ios::trunc);
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
  auto config = load_config_from_pak(L"anoxdata\\CONFIGS\\Default.cfg", L"anoxdata/SCRIPTS.dat", L"anoxdata/SCRIPTS.dat");

  if (config)
  {
    upsert_mouse_bindings(*config, *binding);
  }

  return std::errc{};
}

std::errc init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  auto config = load_config_from_pak(L"anoxdata\\CONFIGS\\Default.cfg", L"anoxdata/SCRIPTS.dat", L"anoxdata/SCRIPTS.dat");

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
      { vkey::gamepad_right_thumbstick_button, "+melee-attack"},
      { vkey::gamepad_x, "+use-plus-special"},
      { vkey::gamepad_y, "weapnext"},
      { vkey::gamepad_left_shoulder, "itemnext"},
      { vkey::gamepad_right_shoulder, "itemuse"},
      { vkey::gamepad_dpad_down, "weapondrop"},
      { vkey::gamepad_dpad_left, "weapprev"},
      { vkey::gamepad_dpad_right, "weapnext"},
      { vkey::gamepad_view, "score"},
      { vkey::gamepad_menu, "menu objectives"},
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
    return get_predefined_id_tech_2_map_command_line_settings(L"anoxdata", true);
  }

  return nullptr;
}

std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_2_5_game_settings(args, {
    .packed_default_cfg = FSL"anoxdata\\CONFIGS\\Default.cfg",
    .packed_default_cfg_pak = FSL"anoxdata/SCRIPTS.dat",
    .packed_default_cfg_pak_folder = FSL"anoxdata/SCRIPTS.dat",
    .loose_user_cfg = FSL"anoxdata\\CONFIGS\\anox.cfg",
  });
}

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  if (filename && std::filesystem::path(filename).wstring().contains(L"anox"))
  {
    return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
  }

  return std::errc::not_supported;
}
}
