#ifndef SIEGE_PRESENTATION_MODULE_HPP
#define SIEGE_PRESENTATION_MODULE_HPP

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <expected>
#include <list>
#include <string>
#include <vector>
#include <set>
#include <siege/platform/stream.hpp>
#include <siege/platform/shared.hpp>

// This is not yet cross platform code. But the plan is to make it so.
#if WIN32
#include <siege/platform/win/window_module.hpp>
#endif

namespace siege::platform
{
  using get_supported_extensions = std::errc(std::size_t, siege::fs_char**, std::size_t*);
  using get_supported_format_categories = std::errc(std::size_t, char16_t**, std::size_t*);
  using get_supported_extensions_for_category = std::errc(const char16_t*, std::size_t count, const siege::fs_char** strings, std::size_t* fetched);
  using is_stream_supported = std::errc(storage_info* data);

// Win32 only methods get to return HRESULT. This helps C# interop as well.
#if WIN32
  using get_window_class_for_stream = HRESULT(storage_info* data, wchar_t**);
#endif

  // TODO Port this code to linux using a "platform::module" instead of a "win32::module"
  class presentation_module : public win32::window_module
  {
    using base = win32::window_module;
    get_supported_extensions* get_supported_extensionsProc = nullptr;
    get_supported_format_categories* get_supported_format_categoriesProc = nullptr;
    get_supported_extensions_for_category* get_supported_extensions_for_categoryProc = nullptr;
    is_stream_supported* is_stream_supportedProc = nullptr;
    get_window_class_for_stream* get_window_class_for_streamProc = nullptr;
    uint32_t DefaultIcon = 0;

  public:
    presentation_module(std::filesystem::path module_path) : base(module_path)
    {
      get_supported_extensionsProc = GetProcAddress<decltype(get_supported_extensionsProc)>("get_supported_extensions");
      get_supported_format_categoriesProc = GetProcAddress<decltype(get_supported_format_categoriesProc)>("get_supported_format_categories");
      get_supported_extensions_for_categoryProc = GetProcAddress<decltype(get_supported_extensions_for_categoryProc)>("get_supported_extensions_for_category");
      is_stream_supportedProc = GetProcAddress<decltype(is_stream_supportedProc)>("is_stream_supported");
      get_window_class_for_streamProc = GetProcAddress<decltype(get_window_class_for_streamProc)>("get_window_class_for_stream");

      auto default_icon = GetProcAddress<std::uint32_t*>("default_file_icon");

      if (default_icon)
      {
        DefaultIcon = *default_icon;
      }

      if (!(get_supported_extensionsProc || get_supported_format_categoriesProc || get_supported_extensions_for_categoryProc || is_stream_supportedProc
            || get_window_class_for_streamProc))
      {
        throw std::runtime_error("Could not find module functions");
      }
    }

    static std::list<presentation_module> load_modules(std::filesystem::path search_path)
    {
      std::list<presentation_module> loaded_modules;

      std::set<std::filesystem::path> dll_paths;

      for (auto const& dir_entry : std::filesystem::directory_iterator{ search_path })
      {
        if ((dir_entry.path().extension() == ".dll" || dir_entry.path().extension() == ".DLL") 
            && dir_entry.path().stem().wstring().find(L"siege-presentation") != std::wstring::npos)
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

    auto get_default_file_icon() const noexcept
    {
      return DefaultIcon;
    }

    std::vector<std::wstring> get_supported_extensions() const noexcept
    {
      std::vector<wchar_t*> temp(64, nullptr);
      std::size_t read = 0;
      if (get_supported_extensionsProc(temp.size(), temp.data(), &read) == std::errc(0))
      {
        temp.resize(read);
        return std::vector<std::wstring>(temp.begin(), temp.end());
      }

      return std::vector<std::wstring>{};
    }

    std::set<std::u16string> get_supported_format_categories() const noexcept
    {
      std::set<std::u16string> results;

      std::size_t read = 0;
      std::vector<char16_t*> temp(8, nullptr);

      if (get_supported_format_categoriesProc(temp.size(), temp.data(), &read) == std::errc(0))
      {
        std::copy(temp.begin(), temp.begin() + read, std::inserter(results, results.begin()));
      }

      return results;
    }

    std::set<siege::fs_string> get_supported_extensions_for_category(const std::u16string& category) const noexcept
    {
      std::set<siege::fs_string> results;

      std::vector<const siege::fs_char*> temp(64, nullptr);
      std::size_t read = 0;
      if (get_supported_extensions_for_categoryProc(category.c_str(), temp.size(), temp.data(), &read) == std::errc(0))
      {
        std::copy(temp.begin(), temp.begin() + read, std::inserter(results, results.begin()));
      }

      return results;
    }

    bool is_stream_supported(storage_info data) const noexcept
    {
      return is_stream_supportedProc(&data) == std::errc(0);
    }

#if WIN32
    std::wstring get_window_class_for_stream(storage_info data) const noexcept
    {
      wchar_t* result;

      if (get_window_class_for_streamProc(&data, &result) == S_OK)
      {
        return result;
      }

      return L"";
    }
#endif
  };

}// namespace siege::platform


#endif// !SIEGEPLUGINHPP
