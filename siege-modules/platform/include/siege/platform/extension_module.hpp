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

  using executable_is_supported = std::errc(const siege::fs_char* filename) noexcept;
  using get_function_name_ranges = std::errc(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using get_variable_name_ranges = std::errc(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
  using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
  using get_predefined_string_command_line_settings = predefined_string*(const siege::fs_char* name) noexcept;
  using get_predefined_int_command_line_settings = predefined_int*(const siege::fs_char* name) noexcept;
  using init_keyboard_inputs = std::errc(keyboard_binding* binding) noexcept;
  using init_mouse_inputs = std::errc(mouse_binding* binding) noexcept;
  using init_controller_inputs = std::errc(controller_binding* binding) noexcept;

  using apply_prelaunch_settings = std::errc(const siege::fs_char* exe_path_str, siege::platform::game_command_line_args*);
  using format_command_line = const siege::fs_char**(const siege::platform::game_command_line_args*, std::uint32_t* new_size);

  using apply_prelaunch_settings_ex = std::errc(const siege::fs_char* exe_path_str, siege::platform::game_command_line_args_ex*);
  using format_command_line_ex = const siege::fs_char**(const siege::platform::game_command_line_args_ex*, std::uint32_t* new_size);
  using is_input_mapping_valid = std::errc(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping);

  // TODO Port this code to linux using a "platform::module" instead of a "win32::module"
  class game_extension_module : public win32::module
  {
    using base = win32::module;
    executable_is_supported* executable_is_supported_proc = nullptr;
    get_function_name_ranges* get_function_name_ranges_proc = nullptr;
    get_variable_name_ranges* get_variable_name_ranges_proc = nullptr;
    init_keyboard_inputs* init_keyboard_inputs_proc = nullptr;
    init_mouse_inputs* init_mouse_inputs_proc = nullptr;
    init_controller_inputs* init_controller_inputs_proc = nullptr;
    is_input_mapping_valid* is_input_mapping_valid_proc = nullptr;

    std::variant<std::monostate, apply_prelaunch_settings*, apply_prelaunch_settings_ex*> apply_prelaunch_settings_proc;
    std::variant<std::monostate, format_command_line*, format_command_line_ex*> format_command_line_proc;

  public:
    get_predefined_string_command_line_settings* get_predefined_string_command_line_settings_proc = nullptr;
    get_predefined_int_command_line_settings* get_predefined_int_command_line_settings_proc = nullptr;

    const std::span<game_action> game_actions;
    const std::span<const wchar_t*> controller_input_backends;

    const std::optional<game_command_line_caps> caps = std::nullopt;

    using base::base;

    game_extension_module(std::filesystem::path module_path) : base(module_path),
                                                               game_actions([this] {
                                                                 auto* actions = GetProcAddress<game_action*>("game_actions");

                                                                 if (!actions)
                                                                 {
                                                                   return std::span<game_action>();
                                                                 }

                                                                 auto size = 0;
                                                                 for (auto i = 0; i < 64; ++i)
                                                                 {
                                                                   if (actions[i].type == game_action::unknown)
                                                                   {
                                                                     size = i;
                                                                     break;
                                                                   }
                                                                 }

                                                                 return std::span(actions, size);
                                                               }()),
                                                               controller_input_backends(create_span("controller_input_backends")),
                                                               caps([this]() -> std::optional<game_command_line_caps> {
                                                                 auto raw_caps = GetProcAddress<game_command_line_caps*>("command_line_caps");

                                                                 if (raw_caps)
                                                                 {
                                                                   game_command_line_caps results{};
                                                                   auto size = raw_caps->caps_size < results.caps_size ? raw_caps->caps_size : results.caps_size;
                                                                   std::memcpy(&results, raw_caps, size);
                                                                   results.caps_size = size;
                                                                   return results;
                                                                 }

                                                                 return std::nullopt;
                                                               }())
    {
      executable_is_supported_proc = GetProcAddress<decltype(executable_is_supported_proc)>("executable_is_supported");
      get_function_name_ranges_proc = GetProcAddress<decltype(get_function_name_ranges_proc)>("get_function_name_ranges");
      get_variable_name_ranges_proc = GetProcAddress<decltype(get_variable_name_ranges_proc)>("get_variable_name_ranges");
      get_predefined_string_command_line_settings_proc = GetProcAddress<decltype(get_predefined_string_command_line_settings_proc)>("get_predefined_string_command_line_settings");
      get_predefined_int_command_line_settings_proc = GetProcAddress<decltype(get_predefined_int_command_line_settings_proc)>("get_predefined_int_command_line_settings");
      init_keyboard_inputs_proc = GetProcAddress<decltype(init_keyboard_inputs_proc)>("init_keyboard_inputs");
      init_mouse_inputs_proc = GetProcAddress<decltype(init_mouse_inputs_proc)>("init_mouse_inputs");
      init_controller_inputs_proc = GetProcAddress<decltype(init_controller_inputs_proc)>("init_controller_inputs");
      is_input_mapping_valid_proc = GetProcAddress<decltype(is_input_mapping_valid_proc)>("is_input_mapping_valid");

      apply_prelaunch_settings_proc = [this] -> decltype(apply_prelaunch_settings_proc) {
        auto* ex_proc = GetProcAddress<apply_prelaunch_settings_ex*>("apply_prelaunch_settings_ex");

        if (ex_proc)
        {
          return ex_proc;
        }

        auto* proc = GetProcAddress<siege::platform::apply_prelaunch_settings*>("apply_prelaunch_settings");

        if (proc)
        {
          return proc;
        }

        return {};
      }();

      format_command_line_proc = [this] -> decltype(format_command_line_proc) {
        auto* ex_proc = GetProcAddress<format_command_line_ex*>("format_command_line_ex");

        if (ex_proc)
        {
          return ex_proc;
        }

        auto* proc = GetProcAddress<siege::platform::format_command_line*>("format_command_line");

        if (proc)
        {
          return proc;
        }

        return {};
      }();

      if (!this->executable_is_supported_proc)
      {
        throw std::runtime_error("Could not find module functions");
      }
    }

    inline bool is_input_mapping_valid(const siege::platform::hardware_context_caps& caps, const siege::platform::input_mapping_ex& mapping) const
    {
      if (!is_input_mapping_valid_proc)
      {
        return true;
      }

      return is_input_mapping_valid_proc(&caps, &mapping) == std::errc{};
    }

    inline std::optional<std::unique_ptr<siege::platform::controller_binding>> init_controller_inputs() const
    {
      if (!init_controller_inputs_proc)
      {
        return std::nullopt;
      }

      auto binding = std::make_unique<siege::platform::controller_binding>();

      if (init_controller_inputs_proc(binding.get()) != std::errc{})
      {
        return std::nullopt;
      }

      return binding;
    }

    inline std::optional<std::unique_ptr<siege::platform::keyboard_binding>> init_keyboard_inputs() const
    {
      if (!init_keyboard_inputs_proc)
      {
        return std::nullopt;
      }

      auto binding = std::make_unique<siege::platform::keyboard_binding>();

      if (init_keyboard_inputs_proc(binding.get()) != std::errc{})
      {
        return std::nullopt;
      }

      return binding;
    }

    inline std::optional<std::unique_ptr<siege::platform::mouse_binding>> init_mouse_inputs() const
    {
      if (!init_mouse_inputs_proc)
      {
        return std::nullopt;
      }

      auto binding = std::make_unique<siege::platform::mouse_binding>();

      if (init_mouse_inputs_proc(binding.get()) != std::errc{})
      {
        return std::nullopt;
      }

      return binding;
    }

    inline std::optional<bool> executable_is_supported(std::filesystem::path exe_path) const noexcept
    {
      if (executable_is_supported_proc)
      {
        return executable_is_supported_proc(exe_path.c_str()) == std::errc{};
      }

      return std::nullopt;
    }

    inline std::optional<bool> supports_format_command_line_ex() const noexcept
    {
      if (format_command_line_proc.index() == 0)
      {
        return std::nullopt;
      }

      return format_command_line_proc.index() == 2;
    }

    inline std::span<const siege::fs_char*> format_command_line(const siege::platform::game_command_line_args_ex& args) const noexcept
    {
      if (supports_format_command_line_ex() != true)
      {
        return {};
      }

      auto* proc = std::get<siege::platform::format_command_line_ex*>(format_command_line_proc);

      std::uint32_t size = 0;
      auto** result = proc(&args, &size);

      if (result == nullptr || size == 0)
      {
        return {};
      }

      return std::span<const siege::fs_char*>(result, size);
    }

    inline std::span<const siege::fs_char*> format_command_line(const siege::platform::game_command_line_args& args) const noexcept
    {
      if (supports_format_command_line_ex() != false)
      {
        return {};
      }

      auto* proc = std::get<siege::platform::format_command_line*>(format_command_line_proc);

      std::uint32_t size = 0;
      auto** result = proc(&args, &size);

      if (result == nullptr || size == 0)
      {
        return {};
      }

      return std::span<const siege::fs_char*>(result, size);
    }

    inline std::optional<bool> supports_apply_prelaunch_settings_ex() const noexcept
    {
      if (apply_prelaunch_settings_proc.index() == 0)
      {
        return std::nullopt;
      }

      return apply_prelaunch_settings_proc.index() == 2;
    }

    inline std::errc apply_prelaunch_settings(std::filesystem::path exe_path, siege::platform::game_command_line_args_ex args) const noexcept
    {
      if (supports_apply_prelaunch_settings_ex() != true)
      {
        return std::errc::not_supported;
      }

      auto* proc = std::get<siege::platform::apply_prelaunch_settings_ex*>(apply_prelaunch_settings_proc);
      return proc(exe_path.c_str(), &args);
    }

    inline std::errc apply_prelaunch_settings(std::filesystem::path exe_path, siege::platform::game_command_line_args& args) const noexcept
    {
      if (supports_apply_prelaunch_settings_ex() != false)
      {
        return std::errc::not_supported;
      }

      auto* proc = std::get<siege::platform::apply_prelaunch_settings*>(apply_prelaunch_settings_proc);
      return proc(exe_path.c_str(), &args);
    }

    static std::list<game_extension_module> load_modules(std::filesystem::path search_path, std::move_only_function<bool(const std::filesystem::path&)> sort_condition = nullptr, std::move_only_function<bool(const game_extension_module&)> condition = nullptr)
    {
      std::list<game_extension_module> loaded_modules;

      std::vector<std::filesystem::path> dll_paths;

      for (auto const& dir_entry : std::filesystem::directory_iterator{ search_path })
      {
        if ((dir_entry.path().extension() == ".dll" || dir_entry.path().extension() == ".DLL")
            && dir_entry.path().stem().wstring().find(L"siege-extension") != std::wstring::npos)
        {
          dll_paths.emplace_back(dir_entry.path());
        }
      }

      if (sort_condition)
      {
        std::stable_partition(dll_paths.begin(), dll_paths.end(), std::move(sort_condition));
      }

      std::for_each(dll_paths.begin(), dll_paths.end(), [&loaded_modules, condition = std::move(condition)](auto path) mutable {
        try
        {
          game_extension_module temp{ path };
          if (condition && !condition(temp))
          {
            return;
          }

          loaded_modules.emplace_back(std::move(temp));
        }
        catch (...)
        {
        }
      });

      return loaded_modules;
    }

    inline bool is_named_export(void* export_ptr) const
    {
      if (!export_ptr)
      {
        return false;
      }

      return export_ptr == executable_is_supported_proc || export_ptr == get_function_name_ranges_proc || export_ptr == get_variable_name_ranges_proc || export_ptr == init_keyboard_inputs_proc || export_ptr == init_mouse_inputs_proc || export_ptr == get_predefined_string_command_line_settings_proc || export_ptr == get_predefined_int_command_line_settings_proc || export_ptr == init_controller_inputs_proc || export_ptr == GetProcAddress<game_command_line_caps*>("command_line_caps") || export_ptr == GetProcAddress<game_command_line_caps*>("controller_input_backends") || export_ptr == GetProcAddress<game_command_line_caps*>("apply_prelaunch_settings") || export_ptr == GetProcAddress<game_command_line_caps*>("game_actions") || export_ptr == GetProcAddress<game_command_line_caps*>("format_command_line");
    }

    inline std::vector<std::pair<std::string, std::string>> get_function_name_ranges() const
    {
      std::vector<std::pair<std::string, std::string>> results;

      if (get_function_name_ranges_proc)
      {
        std::size_t count;

        if (auto hresult = get_function_name_ranges_proc(0, nullptr, &count); hresult == std::errc{})
        {
          std::vector<std::array<const char*, 2>> raw;
          raw.resize(count);

          if (hresult = get_function_name_ranges_proc(raw.size(), raw.data(), &count); hresult == std::errc{})
          {
            results.reserve(raw.size());

            std::transform(raw.begin(), raw.end(), std::back_inserter(results), [](auto& item) {
              return std::make_pair<std::string, std::string>(item[0] ? item[0] : "", item[1] ? item[1] : "");
            });
          }
        }
      }
      return results;
    }

    inline std::vector<std::pair<std::string, std::string>> get_variable_name_ranges() const
    {
      std::vector<std::pair<std::string, std::string>> results;

      if (get_variable_name_ranges_proc)
      {
        std::size_t count;

        if (auto hresult = get_variable_name_ranges_proc(0, nullptr, &count); hresult == std::errc{})
        {
          std::vector<std::array<const char*, 2>> raw;
          raw.resize(count);

          if (hresult = get_variable_name_ranges_proc(raw.size(), raw.data(), &count); hresult == std::errc{})
          {
            results.reserve(raw.size());

            for (auto& item : raw)
            {
              if (!(item[0] || item[1]))
              {
                break;
              }
              results.emplace_back(std::make_pair<std::string, std::string>(item[0], item[1]));
            }
          }
        }
      }
      return results;
    }

  private:
    inline std::span<const wchar_t*> create_span(const char* key)
    {
      auto* storage = this->GetProcAddress<const wchar_t**>(key);

      if (storage)
      {
        int end = 0;

        for (auto i = 0; i < 64; ++i)
        {
          if (storage[i] == nullptr)
          {
            end = i;
            break;
          }
        }

        return std::span<const wchar_t*>(storage, end);
      }
      return std::span<const wchar_t*>();
    }
  };


}// namespace siege::platform

#endif// !SIEGE_EXTENSION_MODULE_HPP
