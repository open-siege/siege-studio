#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/wic.hpp>
#include <siege/platform/win/desktop/direct_2d.hpp>
#include <siege/platform/win/desktop/animation.hpp>
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

  std::map<std::pair<win32::window_ref, std::wstring_view>, COLORREF>& get_theme_map()
  {
    static std::pair<win32::file_mapping, win32::file_view> mapping = [] {
      auto existing = ::OpenFileMappingW(FILE_MAP_WRITE, FALSE, L"SiegeAppThemeColorCache");

      if (existing == nullptr)
      {
        auto mapping = win32::file_mapping(::CreateFileMappingW(
          INVALID_HANDLE_VALUE,// use paging file
          nullptr,// default security attributes
          PAGE_READWRITE,// read/write access
          0,// size: high 32-bits
          sizeof(std::map<std::pair<win32::window_ref, std::wstring_view>, COLORREF>),// size: low 32-bits
          L"SiegeAppThemeColorCache"));

        auto result = mapping.MapViewOfFile(FILE_MAP_WRITE, sizeof(sizeof(std::map<std::pair<win32::window_ref, std::wstring_view>, COLORREF>)));
        new (result.get()) std::map<std::pair<win32::window_ref, std::wstring_view>, COLORREF>();
        return std::make_pair(std::move(mapping), std::move(result));
      }

      auto mapping = win32::file_mapping(existing);
      auto result = mapping.MapViewOfFile(FILE_MAP_WRITE, sizeof(sizeof(std::map<std::pair<win32::window_ref, std::wstring_view>, COLORREF>)));
      return std::make_pair(std::move(mapping), std::move(result));
    }();

    return *(std::map<std::pair<win32::window_ref, std::wstring_view>, COLORREF>*)mapping.second.get();
  }

  COLORREF get_color_for_window(win32::window_ref window, std::wstring_view prop)
  {
    auto key = std::make_pair(window.ref(), prop);
    auto& cache = get_theme_map();

    auto existing = cache.find(key);

    if (existing != cache.end())
    {
      return existing->second;
    }

    for (auto& other : cache)
    {
      if (other.first.second == prop)
      {
        return other.second;
      }
    }

    if (is_dark_theme())
    {
      return prop.find(L"BkColor") == std::wstring_view::npos ? RGB(0, 0, 0) : RGB(255, 255, 255);
    }

    return prop.find(L"BkColor") == std::wstring_view::npos ? RGB(255, 255, 255) : RGB(0, 0, 0);
  }

  std::optional<COLORREF> set_color_for_window(win32::window_ref window, std::wstring_view prop, std::optional<COLORREF> color)
  {
    auto& cache = get_theme_map();

    auto key = std::make_pair(window.ref(), prop);
    auto existing = cache.find(key);
    std::optional<COLORREF> old = std::nullopt;

    if (existing != cache.end())
    {
      old = existing->second;
    }

    if (color == std::nullopt && existing != cache.end())
    {
      cache.erase(existing);
    }
    else if (color != std::nullopt && existing != cache.end())
    {
      existing->second = *color;
    }
    else if (color != std::nullopt && existing == cache.end())
    {
      cache.emplace(std::move(key), *color);
    }

    return old;
  }
}// namespace win32