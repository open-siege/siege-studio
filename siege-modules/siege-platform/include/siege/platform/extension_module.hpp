#ifndef SIEGE_EXTENSION_MODULE_HPP
#define SIEGE_EXTENSION_MODULE_HPP

#include <optional>
#include <filesystem>
#include <list>
#include <span>
#include <siege/platform/win/module.hpp>
#include <siege/platform/shared.hpp>

namespace siege::platform
{
  template<size_t InputSize, typename Context>
  struct input_binding
  {
    struct action_binding
    {
      std::uint16_t virtual_key{};
      enum
      {
        unknown,
        button,
        axis
      } input_type;
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
    throttle
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
    throttle
  };

  struct game_action
  {
    enum
    {
      unknown,
      digital,
      analog
    } type;
    std::array<char, 32> action_name;
    std::array<char16_t, 64> action_display_name;
    std::array<char16_t, 64> group_display_name;
  };

  struct persistent_game_settings
  {
    std::array<fs_char, 64> last_player_name;
    std::array<fs_char, 64> last_ip_address;
  };

  struct game_command_line_caps
  {
    enum type
    {
      unknown,
      string_setting,
      flag_setting,
      int_setting,
      float_setting
    };

    const fs_char* ip_connect_setting = FSL "";
    const fs_char* player_name_setting = FSL "";
    std::array<const fs_char*, 32> flags;
    std::array<const fs_char*, 32> int_settings;
    std::array<const fs_char*, 32> float_settings;
    std::array<const fs_char*, 32> string_settings;
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
  // TODO replace HRESULT with std::errc for the cross-platform functions
  using executable_is_supported = HRESULT(const siege::fs_char* filename) noexcept;
  using get_function_name_ranges = HRESULT(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using get_variable_name_ranges = HRESULT(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
  using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
  using get_predefined_string_command_line_settings = predefined_string*(const siege::fs_char* name) noexcept;
  using get_predefined_int_command_line_settings = predefined_int*(const siege::fs_char* name) noexcept;
  using launch_game_with_extension = HRESULT(const wchar_t* exe_path_str, const game_command_line_args*, PROCESS_INFORMATION*) noexcept;
  using init_keyboard_inputs = HRESULT(keyboard_binding* binding) noexcept;
  using init_mouse_inputs = HRESULT(mouse_binding* binding) noexcept;
  using init_controller_inputs = HRESULT(controller_binding* binding) noexcept;

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

  public:
    get_predefined_string_command_line_settings* get_predefined_string_command_line_settings_proc = nullptr;
    get_predefined_int_command_line_settings* get_predefined_int_command_line_settings_proc = nullptr;

#if WIN32
    launch_game_with_extension* launch_game_with_extension_proc = nullptr;
#endif

    const std::span<game_action> game_actions;
    const std::span<const wchar_t*> controller_input_backends;
    const game_command_line_caps* caps = nullptr;

    inline std::span<const wchar_t*> update_span(const char* key)
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
    };

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
                                                               controller_input_backends(update_span("controller_input_backends")),
                                                               caps(GetProcAddress<game_command_line_caps*>("command_line_caps"))
    {
      executable_is_supported_proc = GetProcAddress<decltype(executable_is_supported_proc)>("executable_is_supported");
      get_function_name_ranges_proc = GetProcAddress<decltype(get_function_name_ranges_proc)>("get_function_name_ranges");
      get_variable_name_ranges_proc = GetProcAddress<decltype(get_variable_name_ranges_proc)>("get_variable_name_ranges");
      get_predefined_string_command_line_settings_proc = GetProcAddress<decltype(get_predefined_string_command_line_settings_proc)>("get_predefined_string_command_line_settings");
      get_predefined_int_command_line_settings_proc = GetProcAddress<decltype(get_predefined_int_command_line_settings_proc)>("get_predefined_int_command_line_settings");
      init_keyboard_inputs_proc = GetProcAddress<decltype(init_keyboard_inputs_proc)>("init_keyboard_inputs");
      init_mouse_inputs_proc = GetProcAddress<decltype(init_mouse_inputs_proc)>("init_mouse_inputs");
      init_controller_inputs_proc = GetProcAddress<decltype(init_controller_inputs_proc)>("init_controller_inputs");
      // These functions are very Windows specific because the games being launched would all be Windows-based.
#if WIN32
      this->launch_game_with_extension_proc = GetProcAddress<decltype(launch_game_with_extension_proc)>("launch_game_with_extension");
#endif

      if (!this->executable_is_supported_proc)
      {
        throw std::runtime_error("Could not find module functions");
      }
    }

    std::vector<std::pair<std::string, std::string>> get_function_name_ranges()
    {
      std::vector<std::pair<std::string, std::string>> results;

      if (get_function_name_ranges_proc)
      {
        std::size_t count;

        if (auto hresult = get_function_name_ranges_proc(0, nullptr, &count); hresult == S_OK)
        {
          std::vector<std::array<const char*, 2>> raw;
          raw.resize(count);

          if (hresult = get_function_name_ranges_proc(raw.size(), raw.data(), &count); hresult == S_OK)
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

    std::vector<std::pair<std::string, std::string>> get_variable_name_ranges()
    {
      std::vector<std::pair<std::string, std::string>> results;

      if (get_variable_name_ranges_proc)
      {
        std::size_t count;

        if (auto hresult = get_variable_name_ranges_proc(0, nullptr, &count); hresult == S_OK)
        {
          std::vector<std::array<const char*, 2>> raw;
          raw.resize(count);

          if (hresult = get_variable_name_ranges_proc(raw.size(), raw.data(), &count); hresult == S_OK)
          {
            results.reserve(raw.size());

            std::transform(raw.begin(), raw.end(), std::back_inserter(results), [](auto& item) {
              return std::make_pair<std::string, std::string>(item[0], item[1]);
            });
          }
        }
      }
      return results;
    }

    std::optional<std::unique_ptr<siege::platform::controller_binding>> init_controller_inputs()
    {
      if (!init_controller_inputs_proc)
      {
        return std::nullopt;
      }

      auto binding = std::make_unique<siege::platform::controller_binding>();

      if (init_controller_inputs_proc(binding.get()) != S_OK)
      {
        return std::nullopt;
      }

      return binding;
    }

    std::optional<std::unique_ptr<siege::platform::keyboard_binding>> init_keyboard_inputs()
    {
      if (!init_keyboard_inputs_proc)
      {
        return std::nullopt;
      }

      auto binding = std::make_unique<siege::platform::keyboard_binding>();

      if (init_keyboard_inputs_proc(binding.get()) != S_OK)
      {
        return std::nullopt;
      }

      return binding;
    }

    std::optional<std::unique_ptr<siege::platform::mouse_binding>> init_mouse_inputs()
    {
      if (!init_mouse_inputs_proc)
      {
        return std::nullopt;
      }

      auto binding = std::make_unique<siege::platform::mouse_binding>();

      if (init_mouse_inputs_proc(binding.get()) != S_OK)
      {
        return std::nullopt;
      }

      return binding;
    }

    std::optional<bool> executable_is_supported(std::filesystem::path exe_path)
    {
      if (executable_is_supported_proc)
      {
        return executable_is_supported_proc(exe_path.c_str()) == S_OK;
      }

      return std::nullopt;
    }

    HRESULT launch_game_with_extension(const wchar_t* exe_path_str, game_command_line_args* game_args, PROCESS_INFORMATION* process_info) noexcept
    {
      if (launch_game_with_extension_proc)
      {
        return launch_game_with_extension_proc(exe_path_str, game_args, process_info);
      }

      if (!exe_path_str)
      {
        return E_POINTER;
      }

      std::error_code last_errorc;

      std::filesystem::path exe_path(exe_path_str);

      if (!std::filesystem::exists(exe_path, last_errorc))
      {
        return E_INVALIDARG;
      }

      if (!game_args)
      {
        return E_POINTER;
      }

      if (!process_info)
      {
        return E_POINTER;
      }

      std::string extension_path = this->GetModuleFileName<char>();

      using apply_prelaunch_settings = HRESULT(const wchar_t* exe_path_str, siege::platform::game_command_line_args* args);
      auto* apply_prelaunch_settings_func = this->GetProcAddress<std::add_pointer_t<apply_prelaunch_settings>>("apply_prelaunch_settings");

      if (apply_prelaunch_settings_func)
      {
        if (apply_prelaunch_settings_func(exe_path.c_str(), game_args) != S_OK)
        {
          return E_ABORT;
        }
      }

      using format_command_line = const wchar_t**(const siege::platform::game_command_line_args*, std::uint32_t*);

      auto* format_command_line_func = this->GetProcAddress<std::add_pointer_t<format_command_line>>("format_command_line");

      const wchar_t** argv = nullptr;
      std::uint32_t argc = 0;


      if (format_command_line_func)
      {
        argv = format_command_line_func(game_args, &argc);
      }


      std::wstring args;
      args.reserve(argc + 3 * sizeof(std::wstring) + 3);
      args.append(1, L'"');
      args.append(exe_path.wstring());
      args.append(1, L'"');

      if (argv && argc > 0)
      {
        args.append(1, L' ');
        for (auto i = 0u; i < argc; ++i)
        {
          args.append(argv[i]);

          if (i < (argc - 1))
          {
            args.append(1, L' ');
          }
        }
      }

      STARTUPINFOW startup_info{ .cb = sizeof(STARTUPINFOW) };
      if (::CreateProcessW(exe_path.c_str(),
            args.data(),
            nullptr,
            nullptr,
            FALSE,
            DETACHED_PROCESS,
            nullptr,
            exe_path.parent_path().c_str(),
            &startup_info,
            process_info))
      {
        return S_OK;
      }

      auto last_error = ::GetLastError();
      return HRESULT_FROM_WIN32(last_error);
    }

    static std::list<game_extension_module> load_modules(std::filesystem::path search_path)
    {
      std::list<game_extension_module> loaded_modules;

      std::set<std::filesystem::path> dll_paths;

      for (auto const& dir_entry : std::filesystem::directory_iterator{ search_path })
      {
        if ((dir_entry.path().extension() == ".dll" || dir_entry.path().extension() == ".DLL")
            && dir_entry.path().stem().wstring().find(L"siege-extension") != std::wstring::npos)
        {
          dll_paths.insert(dir_entry.path());
        }
      }

      std::for_each(dll_paths.begin(), dll_paths.end(), [&](auto path) {
        try
        {
          loaded_modules.emplace_back(path);
        }
        catch (...)
        {
        }
      });

      return loaded_modules;
    }
  };


}// namespace siege::platform

#endif// !SIEGE_EXTENSION_MODULE_HPP
