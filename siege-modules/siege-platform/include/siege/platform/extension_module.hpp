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

  struct game_command_line_caps
  {
    bool supports_ip_connect = false;
    bool supports_ip_host = false;
    bool supports_custom_mod_folder = false;
    bool supports_custom_configurations = false;
    std::array<const char*, 32> flags;
    std::array<const char*, 32> int_settings;
    std::array<const char*, 32> float_settings;
    std::array<const char*, 32> string_settings;
  };

  struct game_command_line_args
  {
    struct ip_address
    {
      std::array<char, 64> ip_address;
      std::uint32_t port_number;
    } connect_ip_address;

    ip_address host_ip_address;
    std::array<char, 64> mod_folder;
    std::array<const char*, 8> configurations;
    std::array<const char*, 32> flags;

    struct int_setting
    {
      const char* name;
      int value;
    };
    std::array<int_setting, 32> int_settings;

    struct float_setting
    {
      const char* name;
      float value;
    };
    std::array<float_setting, 32> float_settings;

    struct string_setting
    {
      const char* name;
      float value;
    };
    std::array<string_setting, 32> string_settings;
  };
  // TODO replace HRESULT with std::errc for the cross-platform functions
  using executable_is_supported = HRESULT(const siege::fs_char* filename) noexcept;
  using get_function_name_ranges = HRESULT(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using get_variable_name_ranges = HRESULT(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using bind_virtual_key_to_action_for_file = HRESULT(const siege::fs_char* filename, controller_binding* inputs, std::size_t inputs_size);

#if WIN32
  using bind_virtual_key_to_action_for_process = HRESULT(DWORD process_id, controller_binding* inputs, std::size_t inputs_size);
  using update_action_intensity_for_process = HRESULT(DWORD process_id, DWORD thread_id, const char* action, float intensity);
  using flatten_game_command_line_args = HRESULT(game_command_line_args*, std::uint32_t argc, const wchar_t** argv, std::uint32_t*) noexcept;
  using launch_game_with_extension = HRESULT(const wchar_t* exe_path_str, std::uint32_t argc, const wchar_t** argv, PROCESS_INFORMATION*) noexcept;
#endif

  // TODO Port this code to linux using a "platform::module" instead of a "win32::module"
  class game_extension_module : public win32::module
  {
    using base = win32::module;
    executable_is_supported* executable_is_supported_proc = nullptr;
    get_function_name_ranges* get_function_name_ranges_proc = nullptr;
    get_variable_name_ranges* get_variable_name_ranges_proc = nullptr;
    std::span<game_action> game_actions;
    std::span<const wchar_t*> controller_input_backends;
    std::span<const wchar_t*> keyboard_input_backends;
    std::span<const wchar_t*> mouse_input_backends;
    std::span<const wchar_t*> configuration_extensions;
    std::span<const wchar_t*> template_configuration_paths;
    std::span<const wchar_t*> autoexec_configuration_paths;
    std::span<const wchar_t*> profile_configuration_paths;

  public:
#if WIN32
    launch_game_with_extension* launch_game_with_extension = nullptr;
    update_action_intensity_for_process* update_action_intensity_for_process = nullptr;
#endif

    game_extension_module(std::filesystem::path module_path) : base(module_path)
    {
      // In theory, it's optional, because dlls may be injected that don't come from this project.
      // In practice, all siege extension modules should have this.
      executable_is_supported_proc = GetProcAddress<decltype(executable_is_supported_proc)>("executable_is_supported");
      get_function_name_ranges_proc = GetProcAddress<decltype(get_function_name_ranges_proc)>("get_function_name_ranges");
      get_variable_name_ranges_proc = GetProcAddress<decltype(get_variable_name_ranges_proc)>("get_variable_name_ranges");

      // These functions are very Windows specific because the games being launched would all be Windows-based.
#if WIN32
      this->launch_game_with_extension = GetProcAddress<decltype(game_extension_module::launch_game_with_extension)>("launch_game_with_extension");
      this->update_action_intensity_for_process = GetProcAddress<decltype(game_extension_module::update_action_intensity_for_process)>("update_action_intensity_for_process");
#endif

      if (!this->launch_game_with_extension)
      {
        throw std::runtime_error("Could not find module functions");
      }

      auto update_span = [this](auto* key, auto& span) {
        auto* storage = GetProcAddress<const wchar_t**>(key);

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

          span = std::span(storage, end);
        }
      };

      update_span("controller_input_backends", controller_input_backends);
      update_span("keyboard_input_backends", keyboard_input_backends);
      update_span("mouse_input_backends", mouse_input_backends);
      update_span("configuration_extensions", configuration_extensions);
      update_span("template_configuration_paths", template_configuration_paths);
      update_span("autoexec_configuration_paths", autoexec_configuration_paths);
      update_span("profile_configuration_paths", profile_configuration_paths);

      auto* actions = GetProcAddress<game_action*>("game_actions");

      if (actions)
      {
        auto size = 0;
        for (auto i = 0; i < 64; ++i)
        {
          if (actions[i].type == game_action::unknown)
          {
            size = i;
            break;
          }
        }

        this->game_actions = std::span(actions, size);
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

    static std::list<game_extension_module> load_modules(std::filesystem::path search_path)
    {
      std::list<game_extension_module> loaded_modules;

      std::set<std::filesystem::path> dll_paths;

      for (auto const& dir_entry : std::filesystem::directory_iterator{ search_path })
      {
        if (dir_entry.path().extension() == ".dll" || dir_entry.path().extension() == ".DLL")
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
