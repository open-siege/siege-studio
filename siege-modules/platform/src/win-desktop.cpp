#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/wic.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <siege/platform/win/shell.hpp>

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

  win32::window_ref get_theme_cache()
  {
    HWND temp = ::FindWindowExW(HWND_MESSAGE, nullptr, L"SiegeAppThemeColorCache", L"ThemeColorCache");

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

    return win32::window_ref(temp);
  }

  std::uint32_t to_storage(COLORREF color)
  {
    std::uint32_t raw_color{};
    RGBQUAD quad{ .rgbBlue = GetBValue(color), .rgbGreen = GetGValue(color), .rgbRed = GetRValue(color), .rgbReserved = 0xff };
    std::memcpy(&raw_color, &quad, sizeof(raw_color));
    return raw_color;
  }

  std::optional<COLORREF> from_storage(std::uint32_t raw_color)
  {
    RGBQUAD quad{};
    std::memcpy(&quad, &raw_color, sizeof(quad));

    if (!quad.rgbReserved)
    {
      return std::nullopt;
    }

    return RGB(quad.rgbRed, quad.rgbGreen, quad.rgbBlue);
  }


  COLORREF get_color_for_class(std::wstring_view class_name, std::wstring_view prop)
  {
    auto cache = get_theme_cache();

    auto child = ::FindWindowExW(HWND_MESSAGE, nullptr, L"SiegeAppThemeColorCache", class_name.data());

    if (child)
    {
      cache = win32::window_ref(child);
    }

    auto color = from_storage(cache.GetPropW<std::uint32_t>(prop));

    if (color)
    {
      return *color;
    }

    if (is_dark_theme())
    {
      return prop.find(L"BkColor") == std::wstring_view::npos ? RGB(255, 255, 255) : RGB(0, 0, 0);
    }

    return prop.find(L"BkColor") == std::wstring_view::npos ? RGB(0, 0, 0) : RGB(255, 255, 255);
  }

  COLORREF get_color_for_window(win32::window_ref window, std::wstring_view prop)
  {
    auto color = from_storage(window.GetPropW<std::uint32_t>(prop));

    if (color)
    {
      return *color;
    }

    auto cache = get_theme_cache();

    static std::array<wchar_t, 255> temp;

    ::RealGetWindowClassW(window, temp.data(), temp.size());

    return get_color_for_class(temp.data(), prop);
  }

  std::optional<COLORREF> set_color_for_class(std::wstring_view class_name, std::wstring_view prop, std::optional<COLORREF> color)
  {
    auto cache = get_theme_cache();

    auto child = class_name.empty() ? cache.ref() : win32::window_ref(::FindWindowExW(HWND_MESSAGE, nullptr, L"SiegeAppThemeColorCache", class_name.data()));

    if (!child)
    {
      child = win32::window_ref(::CreateWindowExW(0, L"SiegeAppThemeColorCache", class_name.data(), 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr));
    }

    if (!color)
    {
      return from_storage(child.RemovePropW<std::uint32_t>(prop));
    }

    auto existing = child.GetPropW<std::uint32_t>(prop);

    child.SetPropW(prop, to_storage(*color));

    return from_storage(existing);
  }

  std::expected<std::filesystem::path, HRESULT> get_path_via_file_dialog(OPENFILENAMEW info)
  {
    auto dialog = win32::com::CreateFileOpenDialog();

    if (!dialog)
    {
      return std::unexpected(dialog.error());
    }

    auto open_dialog = *dialog;
    open_dialog->SetOptions(info.Flags);

    if (info.lpstrInitialDir)
    {
      open_dialog.SetFolder(info.lpstrInitialDir);
    }

    auto result = open_dialog->Show(nullptr);

    if (result != S_OK)
    {
      return std::unexpected(result);
    }

    auto selection = open_dialog.GetResult();

    if (!selection)
    {
      return std::unexpected(result);
    }

    return selection.value().GetFileSysPath();
  }
}// namespace win32