#ifndef SIEGE_EXTENSION_MODULE_HPP
#define SIEGE_EXTENSION_MODULE_HPP

#include <optional>
#include <filesystem>
#include <list>
#include <span>
#include <mutex>
#include <execution>
#include <siege/platform/win/core/module.hpp>
#include <siege/platform/shared.hpp>

namespace siege::platform
{
  struct controller_binding
  {
    struct action_binding
    {
      int virtual_key;
      int input_type;
      int input_index;
      std::array<char, 32> action_name;
    };

    int controller_id;
    std::array<action_binding, 32> inputs;
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

    std::array<controller_binding, 32> input_bindings;
  };
  // TODO replace HRESULT with std::errc for the cross-platform functions
  using executable_is_supported = HRESULT(const siege::fs_char* filename) noexcept;
  using get_function_name_ranges = HRESULT(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using get_variable_name_ranges = HRESULT(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
  using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
  using get_predefined_string_command_line_settings = predefined_string*(const siege::fs_char* name) noexcept;
  using get_predefined_int_command_line_settings = predefined_int*(const siege::fs_char* name) noexcept;

#if WIN32
  using bind_virtual_key_to_action_for_process = HRESULT(DWORD process_id, controller_binding* inputs, std::size_t inputs_size);
  using launch_game_with_extension = HRESULT(const wchar_t* exe_path_str, const game_command_line_args*, PROCESS_INFORMATION*) noexcept;
#endif

  // TODO Port this code to linux using a "platform::module" instead of a "win32::module"
  class game_extension_module : public win32::module
  {
    using base = win32::module;
    executable_is_supported* executable_is_supported_proc = nullptr;
    get_function_name_ranges* get_function_name_ranges_proc = nullptr;
    get_variable_name_ranges* get_variable_name_ranges_proc = nullptr;

  public:
    get_predefined_string_command_line_settings* get_predefined_string_command_line_settings_proc = nullptr;
    get_predefined_int_command_line_settings* get_predefined_int_command_line_settings_proc = nullptr;

#if WIN32
    launch_game_with_extension* launch_game_with_extension_proc = nullptr;
#endif

    const std::span<game_action> game_actions;
    const std::span<const wchar_t*> controller_input_backends;
    const std::span<const wchar_t*> keyboard_input_backends;
    const std::span<const wchar_t*> mouse_input_backends;
    const std::span<const wchar_t*> configuration_extensions;
    const std::span<const wchar_t*> template_configuration_paths;
    const std::span<const wchar_t*> autoexec_configuration_paths;
    const std::span<const wchar_t*> profile_configuration_paths;
    const game_command_line_caps* caps = nullptr;

    inline std::span<const wchar_t*> update_span(const char* key) {
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
        keyboard_input_backends(update_span("keyboard_input_backends")),
        mouse_input_backends(update_span("mouse_input_backends")),
        configuration_extensions(update_span("configuration_extensions")),
        template_configuration_paths(update_span("template_configuration_paths")),
        autoexec_configuration_paths(update_span("autoexec_configuration_paths")),
        profile_configuration_paths(update_span("profile_configuration_paths")),
        caps(GetProcAddress<game_command_line_caps*>("command_line_caps"))
    {
      executable_is_supported_proc = GetProcAddress<decltype(executable_is_supported_proc)>("executable_is_supported");
      get_function_name_ranges_proc = GetProcAddress<decltype(get_function_name_ranges_proc)>("get_function_name_ranges");
      get_variable_name_ranges_proc = GetProcAddress<decltype(get_variable_name_ranges_proc)>("get_variable_name_ranges");
      get_predefined_string_command_line_settings_proc = GetProcAddress<decltype(get_predefined_string_command_line_settings_proc)>("get_predefined_string_command_line_settings");
      get_predefined_int_command_line_settings_proc = GetProcAddress<decltype(get_predefined_int_command_line_settings_proc)>("get_predefined_int_command_line_settings");

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

    std::optional<bool> executable_is_supported(std::filesystem::path exe_path)
    {
      if (executable_is_supported_proc)
      {
        return executable_is_supported_proc(exe_path.c_str()) == S_OK;
      }

      return std::nullopt;
    }

    HRESULT launch_game_with_extension(const wchar_t* exe_path_str, const game_command_line_args* game_args, PROCESS_INFORMATION* process_info) noexcept
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

      using apply_prelaunch_settings = HRESULT(const wchar_t* exe_path_str, const siege::platform::game_command_line_args* args);
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

      std::mutex path_lock;

      std::for_each(std::execution::par_unseq, dll_paths.begin(), dll_paths.end(), [&](auto path) {
        try
        {
          const std::lock_guard<std::mutex> lock(path_lock);
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
