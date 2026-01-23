#include <siege/extension/shared.hpp>
#include <siege/configuration/uprising.hpp>
#include <utility>

namespace fs = std::filesystem;

using namespace siege::configuration::cyclone;

using action_str = std::array<char, 32>;

constexpr auto str(uprising_1_key_map::action action)
{
  action_str result{};

  std::to_chars(result.data(), result.data() + result.size(), static_cast<std::int32_t>(action));

  return result;
}
static_assert(str(uprising_1_key_map::action::move_back) == action_str{ "2" });
static_assert(str(uprising_1_key_map::action::auto_call_in) == action_str{ "16" });

extern "C" {
using hardware_context = siege::platform::hardware_context;
using game_action = siege::platform::game_action;
using keyboard_binding = siege::platform::keyboard_binding;
using mouse_binding = siege::platform::mouse_binding;
using controller_binding = siege::platform::controller_binding;
using game_action = siege::platform::game_action;
using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;


extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { L"joystick_layout" } },
  .string_settings = { { L"name" } },
  .player_name_setting = L"name",
};

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

using action = uprising_1_key_map::action;

extern auto game_actions = std::array<game_action, 50>{ {
  game_action{ game_action::digital, str(action::move_forward), u"Move Forward", u"Movement" },
  game_action{ game_action::digital, str(action::move_back), u"Move Backward", u"Movement" },
  game_action{ game_action::digital, str(action::strafe_left), u"Strafe Left", u"Movement" },
  game_action{ game_action::digital, str(action::strafe_right), u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, str(action::turn_left), u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, str(action::turn_right), u"Turn Right", u"Aiming" },
  game_action{ game_action::analog, str(action::look_up), u"Look Up", u"Aiming" },
  game_action{ game_action::analog, str(action::look_down), u"Look Down", u"Aiming" },
  game_action{ game_action::digital, str(action::fire_weapon_1), u"Fire Weapon 1", u"Combat" },
  game_action{ game_action::digital, str(action::fire_weapon_2), u"Fire Weapon 2", u"Combat" },
  game_action{ game_action::digital, str(action::weapon_1_up), u"Cycle Up - Weapon 1", u"Combat" },
  game_action{ game_action::digital, str(action::weapon_1_down), u"Cycle Down - Weapon 1", u"Combat" },
  game_action{ game_action::digital, str(action::weapon_2_up), u"Cycle Up - Weapon 2", u"Combat" },
  game_action{ game_action::digital, str(action::weapon_2_down), u"Cycle Down - Weapon 2", u"Combat" },
  game_action{ game_action::digital, str(action::weapon_2_down), u"Cycle Down - Weapon 2", u"Combat" },
  game_action{ game_action::digital, str(action::weapon_2_down), u"Cycle Down - Weapon 2", u"Combat" },
  game_action{ game_action::digital, str(action::power_triangle), u"Power Triangle Menu", u"Combat" },
  game_action{ game_action::digital, str(action::cycle_power_triangle), u"Cycle Power Triangle", u"Combat" },
  game_action{ game_action::digital, str(action::gun_lock), u"Toggle Gun Lock", u"Combat" },
  game_action{ game_action::digital, str(action::turret_lock), u"Toggle Turret Lock", u"Combat" },
  game_action{ game_action::digital, str(action::toggle_target_lock), u"Toggle Target Lock", u"Combat" },
  game_action{ game_action::digital, str(action::increase_radar), u"Increase Radar", u"Combat" },
  game_action{ game_action::digital, str(action::decrease_radar), u"Decrease Radar", u"Combat" },
  game_action{ game_action::digital, str(action::deploy_soldier), u"Deploy Soldier", u"Combat" },
  game_action{ game_action::digital, str(action::deploy_tank), u"Deploy Tank", u"Combat" },
  game_action{ game_action::digital, str(action::deploy_aav), u"Deploy AAV", u"Combat" },
  game_action{ game_action::digital, str(action::deploy_bomber), u"Deploy Bomber", u"Combat" },
  game_action{ game_action::digital, str(action::deploy_killersat), u"Deploy K-SAT", u"Combat" },
  game_action{ game_action::digital, str(action::change_tank_view), u"Change Tank View", u"Interface" },
  game_action{ game_action::digital, str(action::toggle_hud_type), u"Toggle HUD Type", u"Interface" },
  game_action{ game_action::digital, str(action::overhead_map), u"Overhead Map", u"Interface" },
  game_action{ game_action::digital, str(action::select_ai_unit), u"Select AI Unit", u"Interface" },
  game_action{ game_action::digital, str(action::sell_back), u"Sell Back Unit", u"Interface" },
  game_action{ game_action::digital, str(action::auto_call_in), u"Auto Call-in/Contextual Command", u"Interface" },
  game_action{ game_action::digital, str(action::call_in_citadel), u"Call-in Citadel", u"Interface" },
  game_action{ game_action::digital, str(action::repair_wraith), u"Repair Wraith (at Citadel)", u"Interface" },
  game_action{ game_action::digital, str(action::call_soldier_building), u"Call-in Soldier Building", u"Interface" },
  game_action{ game_action::digital, str(action::call_tank_building), u"Call-in Tank Building", u"Interface" },
  game_action{ game_action::digital, str(action::call_aav_building), u"Call-in AAV Building", u"Interface" },
  game_action{ game_action::digital, str(action::call_bomber_building), u"Call-in Bomber Building", u"Interface" },
  game_action{ game_action::digital, str(action::call_ksat_building), u"Call-in K-SAT Building", u"Interface" },
  game_action{ game_action::digital, str(action::call_power_building), u"Call-in Power Building", u"Interface" },
  game_action{ game_action::digital, str(action::call_in_turret), u"Call-in Gun Turret", u"Interface" },
  game_action{ game_action::digital, str(action::call_in_sams), u"Call-in Anti-Air Turret", u"Interface" },
  game_action{ game_action::digital, str(action::object_view), u"Object View", u"Interface" },
  game_action{ game_action::digital, str(action::jump_to_citadel), u"Jump to Citadel", u"Interface" },
  game_action{ game_action::digital, str(action::pause_menu), u"Pause Menu/Jump to Wraith", u"Interface" },
  game_action{ game_action::digital, str(action::screenshot), u"Take Screenshot", u"Interface" },
  game_action{ game_action::digital, str(action::chat), u"Chat", u"Interface" },
} };

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  if (filename == nullptr)
  {
    return std::errc::bad_address;
  }

  std::error_code last_error;

  if (!fs::exists(filename, last_error))
  {
    return std::errc::invalid_argument;
  }

  auto exe_path = fs::path(filename);
  auto parent_path = exe_path.parent_path();

  if (exe_path.stem() == "uprising" && exe_path.extension() == ".exe" && fs::exists(parent_path / "uprising.cln", last_error) && fs::exists(parent_path / "ramlockC.VXD", last_error))
  {
    return std::errc{};
  }

  return std::errc::not_supported;
}

std::errc is_input_mapping_valid(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping)
{
  using namespace siege::platform;
  if (mapping == nullptr)
  {
    return std::errc::bad_address;
  }

  if (caps == nullptr)
  {
    return std::errc::bad_address;
  }

  if (mapping->mapping_size < sizeof(input_mapping_ex))
  {
    return std::errc::bad_message;
  }

  if (mapping->context == hardware_context::mouse)
  {
    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 0
        && !(mapping->action_name == str(action::turn_left) || mapping->action_name == str(action::turn_right)))
    {
      return std::errc::not_supported;
    }
    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 1
        && !(mapping->action_name == str(action::look_up) || mapping->action_name == str(action::look_down)))
    {
      return std::errc::not_supported;
    }
    return std::errc{};
  }

  if (is_for_controller(caps->context))
  {
    if (caps->hardware_index != 0)
    {
      return std::errc::not_supported;
    }

    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index >= 2)
    {
      return std::errc::not_supported;
    }

    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 0
        && !(mapping->action_name == str(action::turn_left) || mapping->action_name == str(action::turn_right)))
    {
      return std::errc::not_supported;
    }

    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 1
        && !(mapping->action_name == str(action::look_up) || mapping->action_name == str(action::look_down)))
    {
      return std::errc::not_supported;
    }

    if (mapping->hardware_input_type == controller_input_type::hat && mapping->hardware_index == 0
        && !(mapping->action_name == str(action::strafe_left) || mapping->action_name == str(action::strafe_right) || mapping->action_name == str(action::move_forward) || mapping->action_name == str(action::move_forward)))
    {
      return std::errc::not_supported;
    }
  }

  return std::errc{};
}

std::errc apply_prelaunch_settings_ex(const siege::fs_char* exe_path_str, siege::platform::game_command_line_args_ex* args)
{
  if (exe_path_str == nullptr)
  {
    return std::errc::bad_address;
  }
  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  std::ifstream config_file("FONTS/settings.cfg", std::ios::binary);

  auto config = read_settings(config_file);
  if (!config)
  {
    return std::errc::bad_file_descriptor;
  }

  std::ifstream key_map_file("FONTS/keymap.cfg", std::ios::binary);
  auto key_map = read_key_map(key_map_file);
  if (!key_map)
  {
    return std::errc::bad_file_descriptor;
  }

  auto packaged_args = static_cast<siege::platform::packaged_args>(*args);

  for (auto& binding : packaged_args.action_bindings)
  {
    if (!(binding.context == hardware_context::keyboard || binding.context == hardware_context::keyboard_shifted || binding.context == hardware_context::keypad))
    {
      continue;
    }

    if (binding.hardware_index + 1 > key_map->keyboard_mappings.size())
    {
      continue;
    }

    try
    {
      auto action_str = std::string_view{ binding.action_name.begin(), binding.action_name.end() };
      action_str = action_str.substr(0, action_str.find('\0'));

      auto action_int = std::stoi(std::string{ action_str });

      if (action_int > static_cast<int>(action::weapon_bfm_9000))
      {
        continue;
      }

      key_map->keyboard_mappings[binding.hardware_index] = static_cast<action>(action_int);
    }
    catch (const std::invalid_argument&)
    {
    }
    catch (const std::out_of_range&)
    {
    }
  }

  {
    std::ofstream key_map_file("FONTS/keymap.cfg", std::ios::binary | std::ios::trunc);
    write_key_map(key_map_file, *key_map);
  }
  
  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  std::array<std::pair<WORD, action_str>, 2> buttons{
    { std::make_pair<WORD, action_str>(VK_LBUTTON, str(action::fire_weapon_1)),
      std::make_pair<WORD, action_str>(VK_RBUTTON, str(action::fire_weapon_2)) }
  };


  std::array<std::pair<WORD, action_str>, 4> axes{
    {
      std::make_pair<WORD, action_str>(VK_UP, str(action::look_up)),
      std::make_pair<WORD, action_str>(VK_DOWN, str(action::look_down)),
      std::make_pair<WORD, action_str>(VK_LEFT, str(action::turn_left)),
      std::make_pair<WORD, action_str>(VK_RIGHT, str(action::turn_right)),
    }
  };

  std::ifstream config_file("FONTS/settings.cfg", std::ios::binary);

  auto config = read_settings(config_file);
  if (config && config->flip_mouse == 1)
  {
    axes = {
      {
        std::make_pair<WORD, action_str>(VK_UP, str(action::look_down)),
        std::make_pair<WORD, action_str>(VK_DOWN, str(action::look_up)),
        std::make_pair<WORD, action_str>(VK_LEFT, str(action::turn_left)),
        std::make_pair<WORD, action_str>(VK_RIGHT, str(action::turn_right)),
      }
    };
  }

  for (auto& button : buttons)
  {
    auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

    if (first_available == binding->inputs.end())
    {
      break;
    }
    first_available->virtual_key = button.first;
    first_available->action_name = button.second;
    first_available->context = siege::platform::mouse_context::mouse;
    first_available->input_type = siege::platform::controller_input_type::button;
  }

  for (auto& axis : axes)
  {
    auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

    if (first_available == binding->inputs.end())
    {
      break;
    }

    first_available->virtual_key = axis.first;
    first_available->action_name = axis.second;
    first_available->context = siege::platform::mouse_context::mouse;
    first_available->input_type = siege::platform::controller_input_type::axis;
  }

  return std::errc{};
}

std::errc init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  std::ifstream key_map_file("FONTS/keymap.cfg", std::ios::binary);
  auto key_map = read_key_map(key_map_file);
  if (!key_map)
  {
    return std::errc::bad_file_descriptor;
  }

  int scan_code = 0;
  for (auto& action : key_map->keyboard_mappings)
  {
    if (scan_code == 0)
    {
      scan_code++;
      continue;
    }

    auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

    if (first_available == binding->inputs.end())
    {
      break;
    }
    first_available->input_type = siege::platform::controller_input_type::button;
    first_available->virtual_key = ::MapVirtualKeyW(scan_code, MAPVK_VSC_TO_VK_EX);
    first_available->action_name = str(action);
    scan_code++;
  }

  return std::errc{};
}


std::errc init_controller_inputs(controller_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  std::array<std::pair<WORD, action_str>, 24> actions{
    {
      std::make_pair<WORD, action_str>(VK_GAMEPAD_RIGHT_TRIGGER, str(action::fire_weapon_1)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_LEFT_TRIGGER, str(action::fire_weapon_2)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_LEFT_SHOULDER, str(action::call_power_building)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_RIGHT_SHOULDER, str(action::call_ksat_building)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, str(action::cycle_power_triangle)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, str(action::change_tank_view)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_A, str(action::select_ai_unit)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_B, str(action::sell_back)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_X, str(action::auto_call_in)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_Y, str(action::weapon_2_up)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_DPAD_UP, str(action::call_soldier_building)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_DPAD_DOWN, str(action::call_tank_building)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_DPAD_LEFT, str(action::call_aav_building)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_DPAD_RIGHT, str(action::call_bomber_building)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_VIEW, str(action::jump_to_citadel)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_MENU, str(action::pause_menu)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_LEFT_THUMBSTICK_UP, str(action::move_forward)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, str(action::move_back)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, str(action::strafe_left)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, str(action::strafe_right)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_RIGHT_THUMBSTICK_UP, str(action::look_up)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, str(action::look_down)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, str(action::turn_left)),
      std::make_pair<WORD, action_str>(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, str(action::turn_right)),
    }
  };

  for (auto& controller_default : actions)
  {
    auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

    if (first_available == binding->inputs.end())
    {
      break;
    }
    auto action = std::find_if(game_actions.begin(), game_actions.end(), [&](auto& action) { return action.action_name == controller_default.second; });

    if (action == game_actions.end())
    {
      continue;
    }
    std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());

    first_available->input_type = action->type == action->analog ? siege::platform::controller_input_type::axis : siege::platform::controller_input_type::button;
    first_available->virtual_key = controller_default.first;
  }
  return std::errc{};
}
}
