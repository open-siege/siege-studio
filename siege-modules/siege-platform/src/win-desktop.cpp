#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/stream.hpp>

namespace win32
{
  std::map<COLORREF, win32::gdi_brush>& get_color_map()
  {
    thread_local std::map<COLORREF, win32::gdi_brush> cache;
    return cache;
  }

  HBRUSH get_solid_brush(COLORREF color)
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
      return brush->second;
    }

    auto result = colors.emplace(color, win32::gdi_brush(::CreateSolidBrush(color)));

    return result.first->second.get();
  }
}// namespace win32