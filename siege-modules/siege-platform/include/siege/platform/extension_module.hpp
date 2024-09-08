#ifndef SIEGE_EXTENSION_MODULE_HPP
#define SIEGE_EXTENSION_MODULE_HPP

#include <optional>
#include <filesystem>
#include <list>
#include <siege/platform/win/core/module.hpp>
#include <siege/platform/shared.hpp>

namespace siege::platform
{
  // TODO replace HRESULT with std::errc for the cross-platform functions
  using executable_is_supported = HRESULT(const siege::fs_char* filename) noexcept;
  using get_function_name_ranges = HRESULT(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using get_variable_name_ranges = HRESULT(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
#if WIN32
  using launch_game_with_extension = HRESULT(const wchar_t* exe_path_str, std::uint32_t argc, const wchar_t** argv, PROCESS_INFORMATION*) noexcept;
  using get_game_script_host = HRESULT(const wchar_t* game, ::IDispatch** host) noexcept;
#endif

  // TODO Port this code to linux using a "platform::module" instead of a "win32::module"
  class game_extension_module : public win32::module
  {
    using base = win32::module;
    executable_is_supported* executable_is_supported_proc = nullptr;
    get_function_name_ranges* get_function_name_ranges_proc = nullptr;
    get_variable_name_ranges* get_variable_name_ranges_proc = nullptr;

  public:
#if WIN32
    get_game_script_host* get_game_script_host = nullptr;
    launch_game_with_extension* launch_game_with_extension = nullptr;
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
      this->get_game_script_host = GetProcAddress<decltype(game_extension_module::get_game_script_host)>("get_game_script_host");
      this->launch_game_with_extension = GetProcAddress<decltype(game_extension_module::launch_game_with_extension)>("launch_game_with_extension");
#endif

      bool is_generic_extension = module_path.string().find("extension-generic") != std::string::npos;
     
      if (is_generic_extension && !this->launch_game_with_extension)
      {
        throw std::runtime_error("Could not find module functions");
      }
      else if (!(this->get_game_script_host || this->launch_game_with_extension))
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

    static std::list<game_extension_module> load_modules(std::filesystem::path search_path)
    {
      std::list<game_extension_module> loaded_modules;

      for (auto const& dir_entry : std::filesystem::directory_iterator{ search_path })
      {
        if (dir_entry.path().extension() == ".dll")
        {
          try
          {
            loaded_modules.emplace_back(dir_entry.path());
          }
          catch (...)
          {
          }
        }
      }

      return loaded_modules;
    }
  };


}// namespace siege::platform

#endif// !SIEGE_EXTENSION_MODULE_HPP
