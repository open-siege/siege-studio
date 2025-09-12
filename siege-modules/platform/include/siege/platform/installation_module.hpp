#ifndef SIEGE_INSTALLATION_MODULE_HPP
#define SIEGE_INSTALLATION_MODULE_HPP

#include <optional>
#include <filesystem>
#include <array>
#include <list>
#include <span>
#include <siege/platform/win/module.hpp>
#include <siege/platform/shared.hpp>

namespace siege::platform
{
  struct game_storage_properties
  {
    std::size_t struct_size = sizeof(game_storage_properties);
    std::uint32_t number_of_discs = 0;
    std::array<const fs_char*, 16> disc_names{};
    std::array<fs_char, 1024> default_install_path{};
  };

  struct path_rule
  {
    const fs_char* source;
    const fs_char* destination;
    enum path_enforcement : std::uint32_t
    {
      optional = 0,
      required = 1
    };
    path_enforcement enforcement = required;
  };

  struct installation_variable
  {
    const fs_char* name;
    const fs_char* label;
  };

  using installation_option = installation_variable;

  // installation_option* get_options_for_variable(const fs_char* variable_name) noexcept
  // std::errc apply_post_install_steps() noexcept
  using get_options_for_variable = installation_option*(const siege::fs_char* variable_name) noexcept;
  using apply_post_install_steps = std::errc() noexcept;

  // TODO Port this code to linux using a "platform::module" instead of a "win32::module"
  class installation_module : public win32::module
  {
    using base = win32::module;
    get_options_for_variable* get_options_for_variable_proc = nullptr;

    std::vector<siege::fs_string_view> get_string_vector(auto name)
    {
      auto values = GetProcAddress<siege::fs_char**>(name);
      auto results = std::vector<siege::fs_string_view>();
      if (!values)
      {
        return results;
      }

      results.reserve(8);

      for (auto i = 0; i < 64; ++i)
      {
        if (!values[i])
        {
          break;
        }
        results.emplace_back(values[i]);
      }

      return results;
    };

  public:
    const game_storage_properties* storage_properties = nullptr;
    const std::span<path_rule> directory_mappings;
    const std::vector<siege::fs_string_view> name_variations;
    const std::vector<siege::fs_string_view> associated_extensions;
    const std::span<installation_variable> installation_variables;

    apply_post_install_steps* apply_post_install_steps_proc = nullptr;

    installation_module(std::filesystem::path module_path) : base(module_path),
                                                             storage_properties(GetProcAddress<game_storage_properties*>("storage_properties")),
                                                             directory_mappings([this] {
                                                               auto mappings = GetProcAddress<path_rule*>("directory_mappings");
                                                               if (!mappings)
                                                               {
                                                                 return std::span<path_rule>();
                                                               }

                                                               auto size = 0;
                                                               for (auto i = 0; i < 64; ++i)
                                                               {
                                                                 if (mappings[i].source == nullptr || mappings[i].destination == nullptr)
                                                                 {
                                                                   size = i;
                                                                   break;
                                                                 }
                                                               }

                                                               return std::span(mappings, size);
                                                             }()),
                                                             name_variations(get_string_vector("name_variations")),
                                                             associated_extensions(get_string_vector("associated_extensions")),
                                                             installation_variables([this] {
                                                               auto variables = GetProcAddress<installation_variable*>("installation_variables");
                                                               if (!variables)
                                                               {
                                                                 return std::span<installation_variable>();
                                                               }

                                                               auto size = 0;
                                                               for (auto i = 0; i < 64; ++i)
                                                               {
                                                                 if (variables[i].name == nullptr || variables[i].label == nullptr)
                                                                 {
                                                                   size = i;
                                                                   break;
                                                                 }
                                                               }

                                                               return std::span(variables, size);
                                                             }())
    {
      if (!this->storage_properties || directory_mappings.empty())
      {
        throw std::runtime_error("Could not find install module info");
      }

      get_options_for_variable_proc = GetProcAddress<decltype(get_options_for_variable_proc)>("get_options_for_variable");
      apply_post_install_steps_proc = GetProcAddress<decltype(apply_post_install_steps_proc)>("apply_post_install_steps");
    }

    std::span<installation_option> get_options_for_variable(siege::fs_string name)
    {
      if (!get_options_for_variable_proc)
      {
        return {};
      }

      auto results = get_options_for_variable_proc(name.c_str());

      if (!results)
      {
        return {};
      }

      auto size = 0;
      for (auto i = 0; i < 64; ++i)
      {
        if (results[i].name == nullptr || results[i].label == nullptr)
        {
          size = i;
          break;
        }
      }

      return std::span(results, size);
    }

    static std::list<installation_module> load_modules(std::filesystem::path search_path)
    {
      std::list<installation_module> loaded_modules;

      std::set<std::filesystem::path> dll_paths;

      for (auto const& dir_entry : std::filesystem::directory_iterator{ search_path })
      {
        if ((dir_entry.path().extension() == ".dll" || dir_entry.path().extension() == ".DLL")
            && dir_entry.path().stem().wstring().find(L"siege-installation") != std::wstring::npos)
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

#endif