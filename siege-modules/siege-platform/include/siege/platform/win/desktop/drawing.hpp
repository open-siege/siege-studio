#ifndef WIN32_SHAPES_HPP
#define WIN32_SHAPES_HPP

#include <siege/platform/win/desktop/messages.hpp>
#include <siege/platform/win/auto_handle.hpp>
#include <cassert>

namespace win32
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

  struct gdi_deleter
  {
    void operator()(HGDIOBJ gdi_obj)
    {
      assert(::DeleteObject(gdi_obj) == TRUE);
    }
  };

  struct hdc_deleter
  {
    HWND window;
    std::optional<PAINTSTRUCT> paint_info;
    hdc_deleter(HWND window) : window(window)
    {
    }

    hdc_deleter(HWND window, PAINTSTRUCT info) : window(window), paint_info(std::move(paint_info))
    {
    }

    void operator()(HDC dc)
    {
      if (paint_info)
      {
        ::EndPaint(window, &*paint_info);
      }
      else
      {
        ::ReleaseDC(window, dc);
      }
    }
  };

  struct gdi_no_deleter
  {
    void operator()(HGDIOBJ window)
    {
    }
  };

  using gdi_bitmap = win32::auto_handle<HBITMAP, gdi_deleter>;
  using gdi_brush = win32::auto_handle<HBRUSH, gdi_deleter>;
  using gdi_palette = win32::auto_handle<HPALETTE, gdi_deleter>;
  using gdi_pen = win32::auto_handle<HPEN, gdi_deleter>;
  using gdi_font = win32::auto_handle<HFONT, gdi_deleter>;

  struct gdi_drawing_context : win32::auto_handle<HDC, hdc_deleter>
  {
    using base = win32::auto_handle<HDC, hdc_deleter>;
    using base::base;

    gdi_drawing_context(HWND window) : base(::GetDC(window), hdc_deleter(window))
    {

    }

    gdi_drawing_context(HWND window, ::PAINTSTRUCT paint_info) : base(::BeginPaint(window, &paint_info), hdc_deleter(window, std::move(paint_info)))
    {
    }

    auto FillRect(const ::RECT& pos, HBRUSH brush)
    {
      return ::FillRect(*this, &pos, brush);
    }
  };

  struct gdi_drawing_context_ref : win32::auto_handle<HDC, gdi_no_deleter>
  {
    using base = win32::auto_handle<HDC, gdi_no_deleter>;
    using base::base;


    auto FillRect(const ::RECT& pos, HBRUSH brush)
    {
      return ::FillRect(*this, &pos, brush);
    }
  };


  struct rect : ::RECT
  {
    auto Offset(int dx, int dy)
    {
      return ::OffsetRect(this, dx, dy) == TRUE;
    }
  };

#endif

}// namespace win32

#endif// !WIN32_SHAPES_HPP
