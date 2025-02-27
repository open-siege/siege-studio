#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/wic.hpp>
#include <siege/platform/win/direct_2d.hpp>
#include <siege/platform/win/animation.hpp>
#include <siege/platform/stream.hpp>

namespace win32
{
  std::map<COLORREF, gdi::brush>& get_color_map()
  {
    thread_local std::map<COLORREF, gdi::brush> cache;
    return cache;
  }

  gdi::brush_ref get_solid_brush(COLORREF color)
  {
    auto& colors = get_color_map();

    auto brush = colors.find(color);

    if (brush != colors.end() && !brush->second)
    {
      colors.erase(brush);
      brush = colors.end();
    }

    if (brush != colors.end())
    {
      return gdi::brush_ref(brush->second);
    }

    auto result = colors.emplace(color, gdi::brush(::CreateSolidBrush(color)));

    return gdi::brush_ref(result.first->second.get());
  }

  bool& get_thread_theme()
  {
    static win32::file_mapping mapping = [] {
      auto existing = ::OpenFileMappingW(FILE_MAP_WRITE, FALSE, L"SiegeAppUseDarkMode");

      if (existing == nullptr)
      {
        return win32::file_mapping(::CreateFileMappingW(
          INVALID_HANDLE_VALUE,// use paging file
          nullptr,// default security attributes
          PAGE_READWRITE,// read/write access
          0,// size: high 32-bits
          sizeof(bool),// size: low 32-bits
          L"SiegeAppUseDarkMode"));
      }

      return win32::file_mapping(existing);
    }();

    static win32::file_view variable_view = mapping.MapViewOfFile(FILE_MAP_WRITE, sizeof(bool));

    return *(bool*)variable_view.get();
  }

  bool is_dark_theme()
  {
    return get_thread_theme();
  }

  void set_is_dark_theme(bool new_value)
  {
    get_thread_theme() = new_value;
  }

  HWND get_theme_cache()
  {
    HWND temp = ::FindWindowExW(HWND_MESSAGE, nullptr, L"SiegeAppThemeColorCache", nullptr);

    if (temp == nullptr)
    {
      WNDCLASSEXW info{
        .cbSize = sizeof(WNDCLASSEXW),
        .lpfnWndProc = DefWindowProcW,
        .hInstance = ::GetModuleHandleW(nullptr),
        .lpszClassName = L"SiegeAppThemeColorCache"
      };
    
      if (!GetClassInfoExW(info.hInstance, info.lpszClassName, &info))
      {
        ::RegisterClassExW(&info);
      }

      temp = ::CreateWindowExW(0, info.lpszClassName, L"ThemeColorCache", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    }

    return temp;
  }

  std::optional<COLORREF> get_color_from_handle(HANDLE handle)
  {
    if (handle)
    {
      auto type = GetObjectType(handle);

      if (type == OBJ_BRUSH)
      {
        static LOGBRUSH brush;

        if (::GetObjectW(handle, sizeof(brush), &brush))
        {
          return brush.lbColor;
        }
      }
      else if (type == OBJ_PEN)
      {
        static LOGPEN pen;

        if (::GetObjectW(handle, sizeof(pen), &pen))
        {
          return pen.lopnColor;
        }
      }
      else if (type == OBJ_EXTPEN)
      {
        static EXTLOGPEN pen;

        if (::GetObjectW(handle, sizeof(pen), &pen))
        {
          return pen.elpColor;
        }
      }
    }

    return std::nullopt;
  }

  COLORREF get_color_for_window(win32::window_ref window, std::wstring_view prop)
  {
    auto cache = get_theme_cache();

    static std::array<wchar_t, 255> temp;

    ::RealGetWindowClassW(window, temp.data(), temp.size());

    auto child = ::FindWindowExW(cache, nullptr, nullptr, temp.data());

    if (child)
    {
      cache = child;
    }

    auto handle = ::GetPropW(cache, prop.data());

    auto color = get_color_from_handle(handle);

    if (color)
    {
      return *color;
    }

    if (is_dark_theme())
    {
      return prop.find(L"BkColor") == std::wstring_view::npos ? RGB(0, 0, 0) : RGB(255, 255, 255);
    }

    return prop.find(L"BkColor") == std::wstring_view::npos ? RGB(255, 255, 255) : RGB(0, 0, 0);
  }

  std::optional<COLORREF> set_color_for_window(std::wstring_view prop, std::optional<COLORREF> color)
  {
    auto cache = get_theme_cache();

    auto existing = ::GetPropW(cache, prop.data());

    if (!color)
    {
      auto handle = ::RemovePropW(cache, prop.data());
      return get_color_from_handle(handle);
    }

    SetPropW(cache, prop.data(), get_solid_brush(*color).get());

    return get_color_from_handle(existing);
  }
}// namespace win32