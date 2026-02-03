#ifndef SIEGE_EXTENSION_MODULE_HPP
#define SIEGE_EXTENSION_MODULE_HPP

#include <optional>
#include <filesystem>
#include <list>
#include <span>
#include <functional>
#include <map>
#include <variant>
#include <siege/platform/win/module.hpp>
#include <siege/platform/shared.hpp>

namespace siege::platform
{
  enum struct controller_input_type
  {
    unknown,
    button,
    axis,
    hat
  };

  template<size_t InputSize, typename Context>
  struct input_binding
  {
    struct action_binding
    {
      std::uint16_t virtual_key{};
      controller_input_type input_type;
      std::uint16_t input_index{};
      Context context;
      std::array<char, 32> action_name{};
    };

    int controller_index{};
    const char16_t* controller_backend = nullptr;
    std::array<action_binding, InputSize> inputs{};
  };


  enum struct keyboard_context : std::uint16_t
  {
    keyboard = 1,
    keyboard_shifted,
    keypad
  };
  using keyboard_binding = input_binding<128, keyboard_context>;

  enum struct mouse_context : std::uint16_t
  {
    mouse = 4,
    mouse_wheel
  };

  using mouse_binding = input_binding<16, mouse_context>;

  enum struct controller_context : std::uint16_t
  {
    controller_xbox = 6,
    controller_playstation_3,
    controller_playstation_4,
    controller_nintendo,
    joystick,
    throttle,
    steering_wheel,
    pedal,
    custom = 64
  };
  using controller_binding = input_binding<32, controller_context>;

  enum struct hardware_context : std::uint16_t
  {
    global,
    keyboard = static_cast<std::uint16_t>(keyboard_context::keyboard),
    keyboard_shifted,
    keypad,
    mouse = static_cast<std::uint16_t>(mouse_context::mouse),
    mouse_wheel,
    controller_xbox = static_cast<std::uint16_t>(controller_context::controller_xbox),
    controller_playstation_3,
    controller_playstation_4,
    controller_nintendo,
    joystick,
    throttle,
    steering_wheel,
    pedal,
    custom = 64
  };

  struct hardware_context_caps
  {
    std::size_t caps_size = sizeof(hardware_context_caps);
    hardware_context context;
    std::uint32_t hardware_index;
    std::uint32_t button_count;
    std::uint32_t axis_count;
    std::uint32_t hat_count;
  };

  inline bool is_for_controller(hardware_context context)
  {
    return static_cast<int>(context) >= static_cast<int>(hardware_context::controller_xbox);
  }

  struct game_action
  {
    enum
    {
      unknown,
      digital,
      analog,
    } type;
    std::array<char, 32> action_name;
    std::array<char16_t, 64> action_display_name;
    std::array<char16_t, 64> group_display_name;
  };

  struct game_command_line_caps
  {
    std::size_t caps_size = sizeof(game_command_line_caps);
    std::array<const fs_char*, 32> flags;
    std::array<const fs_char*, 32> int_settings;
    std::array<const fs_char*, 32> float_settings;
    std::array<const fs_char*, 32> string_settings;
    const fs_char* ip_connect_setting = FSL "";
    const fs_char* port_connect_setting = FSL "";
    const fs_char* player_name_setting = FSL "";
    const fs_char* listen_setting = FSL "";
    const fs_char* dedicated_setting = FSL "";
    const fs_char* render_backend_setting = FSL "";
    const fs_char* selected_game_setting = FSL "";
    const fs_char* preferred_exe_setting = FSL "";
    const fs_char* controller_enabled_setting = FSL "";
  };

  template<typename TValue>
  struct game_command_line_predefined_setting
  {
    const fs_char* label;
    const fs_char* description;
    const TValue value;
    bool is_default = false;
  };

  struct game_command_line_args
  {
    std::array<const fs_char*, 32> flags;

    struct int_setting
    {
      const fs_char* name;
      int value;
    };
    std::array<int_setting, 32> int_settings;

    struct float_setting
    {
      const fs_char* name;
      float value;
    };
    std::array<float_setting, 32> float_settings;

    struct string_setting
    {
      const fs_char* name;
      const fs_char* value;
    };

    std::array<string_setting, 32> string_settings;
    std::array<string_setting, 32> environment_settings;

    struct input_mapping
    {
      std::uint16_t vkey;
      std::uint16_t hardware_index;
      hardware_context context = {};
      std::array<char, 32> action_name;
    };

    std::array<input_mapping, 256> action_bindings;

    struct controller_to_send_input_mapping
    {
      std::uint16_t from_vkey;
      hardware_context from_context = {};
      std::uint16_t to_vkey;
      hardware_context to_context = {};
    };

    std::array<controller_to_send_input_mapping, 256> controller_to_send_input_mappings;
  };

  struct input_mapping_ex
  {
    std::size_t mapping_size = sizeof(input_mapping_ex);
    std::uint16_t vkey;
    std::uint16_t hardware_index;
    controller_input_type hardware_input_type;
    hardware_context context = {};
    std::array<char, 32> action_name;
  };

  struct packaged_args
  {
    std::span<const fs_char*> flags;
    std::span<game_command_line_args::int_setting> int_settings;
    std::span<game_command_line_args::float_setting> float_settings;
    std::span<game_command_line_args::string_setting> string_settings;
    std::span<game_command_line_args::string_setting> environment_settings;
    std::span<game_command_line_args::controller_to_send_input_mapping> controller_to_send_input_mappings;

    std::vector<input_mapping_ex> action_bindings;
  };

  struct owning_packaged_args
  {
    std::vector<const fs_char*> flags;
    std::vector<game_command_line_args::int_setting> int_settings;
    std::vector<game_command_line_args::float_setting> float_settings;
    std::vector<game_command_line_args::string_setting> string_settings;
    std::vector<game_command_line_args::string_setting> environment_settings;
    std::vector<game_command_line_args::controller_to_send_input_mapping> controller_to_send_input_mappings;
    std::vector<input_mapping_ex> action_bindings;

    inline operator packaged_args()
    {
      return packaged_args{
        .flags = flags,
        .int_settings = int_settings,
        .float_settings = float_settings,
        .string_settings = string_settings,
        .environment_settings = environment_settings,
        .controller_to_send_input_mappings = controller_to_send_input_mappings,
        .action_bindings = action_bindings
      };
    }
  };

  struct game_command_line_args_ex
  {
    std::size_t args_size = sizeof(game_command_line_args_ex);
    std::size_t flags_size = 0;
    const fs_char** flags;

    std::size_t int_settings_size = 0;
    game_command_line_args::int_setting* int_settings = nullptr;

    std::size_t float_settings_size = 0;
    game_command_line_args::float_setting* float_settings = nullptr;

    std::size_t string_settings_size = 0;
    game_command_line_args::string_setting* string_settings = nullptr;

    std::size_t environment_settings_size = 0;
    game_command_line_args::string_setting* environment_settings = nullptr;

    std::size_t input_mapping_struct_size = sizeof(input_mapping_ex);
    std::size_t action_bindings_size = 0;
    std::byte* action_bindings = nullptr;

    std::size_t controller_to_send_input_mappings_size = 0;
    game_command_line_args::controller_to_send_input_mapping* controller_to_send_input_mappings = nullptr;

    inline operator packaged_args() const
    {
      std::vector<input_mapping_ex> action_bindings;
      action_bindings.reserve(action_bindings_size);

      for (auto i = 0; i < action_bindings_size; ++i)
      {
        auto& back = action_bindings.emplace_back();
        std::memcpy(&back, this->action_bindings + (input_mapping_struct_size * i), input_mapping_struct_size);
      }

      return packaged_args{
        .int_settings = { int_settings, int_settings_size },
        .float_settings = { float_settings, float_settings_size },
        .string_settings = { string_settings, string_settings_size },
        .environment_settings = { environment_settings, environment_settings_size },
        .controller_to_send_input_mappings = { controller_to_send_input_mappings, controller_to_send_input_mappings_size },
        .action_bindings = std::move(action_bindings)
      };
    }
  };
}// namespace siege::platform

#endif// !SIEGE_EXTENSION_MODULE_HPP
