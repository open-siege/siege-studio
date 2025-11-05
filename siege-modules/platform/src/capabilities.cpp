#include <siege/platform/win/capabilities.hpp>
#include <siege/platform/win/com.hpp>
#include <d2d1.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <wincodec.h>

namespace win32
{
  std::optional<std::pair<version, std::wstring_view>> get_xinput_version()
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
          L"xinput1_4.dll");
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

  std::optional<version> get_gdi_plus_version()
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

  std::optional<version> get_wic_version()
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

  bool has_font(std::wstring_view text)
  {
    static std::map<std::wstring_view, HFONT> fonts;

    auto font_iter = fonts.find(text);

    if (font_iter == fonts.end())
    {
      LOGFONTW info{
        .lfFaceName = L"Segoe UI"
      };

      auto result = ::CreateFontW(info.lfHeight,
        info.lfWidth,
        info.lfEscapement,
        info.lfOrientation,
        info.lfWeight,
        info.lfItalic,
        info.lfUnderline,
        info.lfStrikeOut,
        info.lfCharSet,
        info.lfOutPrecision,
        info.lfClipPrecision,
        info.lfQuality,
        info.lfPitchAndFamily,
        info.lfFaceName);

      font_iter = fonts.emplace(text, result).first;
    }

    return font_iter->second != nullptr;
  }

  bool has_segoe_emoji()
  {
    return false;
  }

  std::optional<std::wstring_view> get_best_system_icon_font()
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

  std::optional<std::wstring_view> get_best_system_font()
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

  std::optional<std::wstring_view> get_segoe_symbol_name()
  {
    return std::nullopt;
  }
}