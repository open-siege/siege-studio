module;
#include <siege/platform/windows.hpp>
#include <versionhelpers.h>
#include <wincodec.h>

export module win32.capabilities;

import std;
import win32.com;

namespace win32
{
  export struct version
  {
    std::uint8_t major;
    std::uint8_t minor;
    std::uint8_t build;
    std::uint8_t revision;
  };

  export std::optional<std::pair<version, std::wstring_view>> get_xinput_version()
  {
    static std::optional<std::pair<version, std::wstring_view>> result = [] {
      std::optional<std::pair<version, std::wstring_view>> result = std::nullopt;

      auto module = ::LoadLibraryExW(L"xinput1_4.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

      if (module)
      {
        result = std::make_pair(version{
                                  .major = 1,
                                  .minor = 4 },
          L"xinput1_4.dll");
        ::FreeLibrary(module);
        return result;
      }

      module = ::LoadLibraryExW(L"xinput1_3.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

      if (module)
      {
        result = std::make_pair(version{
                                  .major = 1,
                                  .minor = 3 },
          L"xinput1_3.dll");

        ::FreeLibrary(module);
        return result;
      }

      module = ::LoadLibraryExW(L"xinput1_2.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

      if (module)
      {
        result = std::make_pair(version{
                                  .major = 1,
                                  .minor = 2 },
          L"xinput1_2.dll");
        ::FreeLibrary(module);
        return result;
      }

      module = ::LoadLibraryExW(L"xinput1_1.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

      if (module)
      {
        result = std::make_pair(version{
                                  .major = 1,
                                  .minor = 1 },
          L"xinput1_1.dll");
        ::FreeLibrary(module);
        return result;
      }

      module = ::LoadLibraryExW(L"xinput9_1_0.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

      if (module)
      {
        result = std::make_pair(version{
                                  .major = 1,
                                  .minor = 0 },
          L"xinput9_1_0.dll");
        ::FreeLibrary(module);
        return result;
      }

      return result;
    }();

    return result;
  }

  export std::optional<version> get_gdi_plus_version()
  {
    static std::optional<version> result = [] {
      std::optional<version> result = std::nullopt;
      auto module = ::LoadLibraryExW(L"gdiplus.dll", nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

      if (module)
      {
        result = {
          .major = 1
        };

        if (::GetProcAddress(module, "GdipConvertToEmfPlusToFile") != nullptr)
        {
          result->minor = 1;
        }

        ::FreeLibrary(module);
        return result;
      }

      return result;
    }();
    return result;
  }

  export std::optional<version> get_wic_version()
  {
    static std::optional<version> result = [] {
      win32::com::com_ptr<IWICImagingFactory2> temp_2{};

      if (::CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory2), temp_2.put_void()) == S_OK)
      {
        return std::make_optional(version{ .major = 2, .minor = 0 });
      }

      win32::com::com_ptr<IWICImagingFactory> temp_1{};

      if (::CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), temp_1.put_void()) == S_OK)
      {
        return std::make_optional(version{ .major = 1, .minor = 0 });
      }

      return std::optional<version>(std::nullopt);
    }();

    return result;
  }

  export bool has_font(std::wstring_view text)
  {
    static std::map<std::wstring, HFONT, std::less<>> fonts;

    auto font_iter = fonts.find(text);

    if (font_iter == fonts.end())
    {
      auto result = ::CreateFontW(0,
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        text.data());

      font_iter = fonts.emplace(std::wstring(text), result).first;
    }

    return font_iter->second != nullptr;
  }

  export bool has_segoe_emoji()
  {
    return false;
  }

  export std::optional<std::wstring_view> get_best_system_icon_font()
  {
    std::wstring_view temp = L"Segoe MDL2 Assets";

    if (has_font(temp))
    {
      return temp;
    }

    temp = L"Segoe UI Symbol";

    if (has_font(temp))
    {
      return temp;
    }

    return std::nullopt;
  }

  export std::optional<std::wstring_view> get_best_system_font()
  {
    std::wstring_view temp = L"Segoe UI";

    if (has_font(temp))
    {
      return temp;
    }

    temp = L"Tahoma";

    if (has_font(temp))
    {
      return temp;
    }

    temp = L"Ubuntu";

    if (has_font(temp))
    {
      return temp;
    }

    temp = L"MS Sans Serif";

    if (has_font(temp))
    {
      return temp;
    }

    temp = L"Courier New";

    if (has_font(temp))
    {
      return temp;
    }

    temp = L"Arial";

    if (has_font(temp))
    {
      return temp;
    }

    return std::nullopt;
  }

  export std::optional<std::wstring_view> get_segoe_symbol_name()
  {
    return std::nullopt;
  }
}// namespace win32
