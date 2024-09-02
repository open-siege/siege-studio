#ifndef SIEGE_SFX_CONTROLLER_HPP
#define SIEGE_SFX_CONTROLLER_HPP

#include <string_view>
#include <array>
#include <optional>
#include <filesystem>
#include <istream>
#include <map>
#include <set>
#include <list>
#include <siege/platform/win/core/module.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/shared.hpp>

namespace siege::views
{
  struct menu_item : ::MENUITEMINFOW
  {
    menu_item() : ::MENUITEMINFOW()
    {
    }

    menu_item(std::wstring text) : ::MENUITEMINFOW(), text(std::move(text))
    {
      this->dwTypeData = text.data();
    }

    std::wstring text;
    std::vector<menu_item> sub_items;
  };

  struct menu_info : ::MENUINFO
  {
    std::vector<menu_item> menu_items;
  };

  class exe_controller
  {
  public:
    constexpr static auto exe_formats = std::array<siege::fs_string_view, 2>{ { FSL ".exe", FSL ".com" } };
    constexpr static auto lib_formats = std::array<siege::fs_string_view, 6>{ {
      FSL ".dll",
      FSL ".ocx",
      FSL ".olb",
      FSL ".lib",
      FSL ".asi",
      FSL ".ovl",
    } };

    static bool is_exe(std::istream& image_stream);

    std::map<std::wstring, std::set<std::wstring>> get_resource_names() const;

    std::set<std::string_view> get_function_names() const;

    std::set<std::string_view> get_variable_names() const;

    std::optional<menu_info> get_resource_menu_items(std::wstring type, std::wstring name) const;

    std::vector<std::wstring> get_resource_strings(std::wstring type, std::wstring name) const;

    std::size_t load_executable(std::istream& image_stream, std::optional<std::filesystem::path>) noexcept;

  private:
    std::list<siege::platform::game_extension_module> extensions;
    std::filesystem::path loaded_path;
    win32::module loaded_module;
    std::list<siege::platform::game_extension_module>::iterator matching_extension;
  };
}// namespace siege::views

#endif// DARKSTARDTSCONVERTER_PAL_VIEW_HPP
