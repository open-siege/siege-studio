#ifndef SIEGE_CONTENT_MODULE_HPP
#define SIEGE_CONTENT_MODULE_HPP

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <expected>
#include <list>
#include <string>
#include <vector>
#include <set>
#include <siege/platform/win/core/com/collection.hpp>
#include <siege/platform/win/desktop/window_module.hpp>

namespace siege::platform
{
  // TODO cross platform versions of this code will use "xcom" (cross com) and char16_t instead of wchar_t
  using GetSupportedExtensions = HRESULT __stdcall(win32::com::IReadOnlyCollection** formats);
  using GetSupportedFormatCategories = HRESULT __stdcall(LCID, win32::com::IReadOnlyCollection** formats);
  using GetSupportedExtensionsForCategory = HRESULT __stdcall(const wchar_t* category, win32::com::IReadOnlyCollection** formats);
  using IsStreamSupported = HRESULT __stdcall(::IStream* data);
  using GetWindowClassForStream = HRESULT __stdcall(::IStream* data, wchar_t**);

  // TODO Port this code to linux using a "platform::module" instead of a "win32::module"
  class content_module : public win32::window_module
  {
    using base = win32::window_module;
    GetSupportedExtensions* GetSupportedExtensionsProc = nullptr;
    GetSupportedFormatCategories* GetSupportedFormatCategoriesProc = nullptr;
    GetSupportedExtensionsForCategory* GetSupportedExtensionsForCategoryProc = nullptr;
    IsStreamSupported* IsStreamSupportedProc = nullptr;
    GetWindowClassForStream* GetWindowClassForStreamProc = nullptr;
    uint32_t DefaultIcon = 0;

  public:
    content_module(std::filesystem::path module_path) : base(module_path)
    {
      GetSupportedExtensionsProc = GetProcAddress<decltype(GetSupportedExtensionsProc)>("GetSupportedExtensions");
      GetSupportedFormatCategoriesProc = GetProcAddress<decltype(GetSupportedFormatCategoriesProc)>("GetSupportedFormatCategories");
      GetSupportedExtensionsForCategoryProc = GetProcAddress<decltype(GetSupportedExtensionsForCategoryProc)>("GetSupportedExtensionsForCategory");
      IsStreamSupportedProc = GetProcAddress<decltype(IsStreamSupportedProc)>("IsStreamSupported");
      GetWindowClassForStreamProc = GetProcAddress<decltype(GetWindowClassForStreamProc)>("GetWindowClassForStream");

      auto default_icon = GetProcAddress<std::uint32_t*>("DefaultFileIcon");

      if (default_icon)
      {
        DefaultIcon = *default_icon;
      }

      if (!(GetSupportedExtensionsProc || GetSupportedFormatCategoriesProc || GetSupportedExtensionsForCategoryProc || IsStreamSupportedProc
            || GetWindowClassForStreamProc))
      {
        throw std::runtime_error("Could not find module functions");
      }
    }

    static std::list<content_module> load_modules(std::filesystem::path search_path)
    {
      std::list<content_module> loaded_modules;

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

    auto GetDefaultFileIcon() const noexcept
    {
      return DefaultIcon;
    }

    std::vector<std::wstring> GetSupportedExtensions() const noexcept
    {
      std::vector<std::wstring> results;

      win32::com::IReadOnlyCollection* raw = nullptr;

      IEnumString* value = nullptr;
      if (GetSupportedExtensionsProc(&raw) == S_OK)
      {
        auto enumerator = raw->NewEnum<IEnumString>();

        if (enumerator)
        {
          value = enumerator->get();
          std::vector<win32::com::com_string> raw_results(64);

          ULONG fetched = 0u;
          enumerator->get()->Next(raw_results.size(), raw_results.data()->put(), &fetched);

          results.reserve(fetched);
          std::copy(raw_results.begin(), raw_results.begin() + fetched, std::back_inserter(results));
        }

        assert(raw->Release() == 0);
      }

      return results;
    }

    std::set<std::wstring> GetSupportedFormatCategories(LCID locale_id) const noexcept
    {
      std::set<std::wstring> results;

      win32::com::IReadOnlyCollection* raw = nullptr;

      if (GetSupportedFormatCategoriesProc(locale_id, &raw) == S_OK)
      {
        auto enumerator = raw->NewEnum<IEnumString>();

        if (enumerator)
        {
          std::vector<win32::com::com_string> raw_results(16);

          ULONG fetched = 0u;
          enumerator->get()->Next(raw_results.size(), raw_results.data()->put(), &fetched);

          std::copy(raw_results.begin(), raw_results.begin() + fetched, std::inserter(results, results.begin()));
        }

        assert(raw->Release() == 0);
      }

      return results;
    }

    std::set<std::wstring> GetSupportedExtensionsForCategory(const std::wstring& category) const noexcept
    {
      std::set<std::wstring> results;

      win32::com::IReadOnlyCollection* raw = nullptr;

      if (GetSupportedExtensionsForCategoryProc(category.c_str(), &raw) == S_OK)
      {
        auto enumerator = raw->NewEnum<IEnumString>();

        if (enumerator)
        {
          std::vector<win32::com::com_string> raw_results(64);

          ULONG fetched = 0u;
          enumerator->get()->Next(raw_results.size(), raw_results.data()->put(), &fetched);

          std::copy(raw_results.begin(), raw_results.begin() + fetched, std::inserter(results, results.begin()));
        }

        assert(raw->Release() == 0);
      }

      return results;
    }

    bool IsStreamSupported(IStream& data) const noexcept
    {
      return IsStreamSupportedProc(&data) == S_OK;
    }

    std::wstring GetWindowClassForStream(IStream& data) const noexcept
    {
      wchar_t* result;

      if (GetWindowClassForStreamProc(&data, &result) == S_OK)
      {
        return result;
      }

      return L"";
    }
  };

}// namespace siege


#endif// !SIEGEPLUGINHPP
