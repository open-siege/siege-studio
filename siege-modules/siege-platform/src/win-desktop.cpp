#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
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
}// namespace win32