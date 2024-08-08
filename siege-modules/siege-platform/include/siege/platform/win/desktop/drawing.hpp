#ifndef WIN32_DRAWING_HPP
#define WIN32_DRAWING_HPP

#include <siege/platform/win/desktop/window.hpp>
#include <siege/platform/win/auto_handle.hpp>
#include <cassert>

namespace win32
{
  struct rect : ::RECT
  {
    auto Offset(int dx, int dy)
    {
      return ::OffsetRect(this, dx, dy) == TRUE;
    }
  };
}// namespace win32

namespace win32::gdi
{
  struct gdi_deleter
  {
    void operator()(HGDIOBJ gdi_obj)
    {
      assert(::DeleteObject(gdi_obj) == TRUE);
    }
  };

  struct hdc_releaser
  {
    HWND window = nullptr;
    std::optional<PAINTSTRUCT> paint_info = std::nullopt;

    hdc_releaser() = default;

    hdc_releaser(HWND window) : window(window)
    {
    }

    hdc_releaser(HWND window, PAINTSTRUCT info) : window(window), paint_info(std::move(paint_info))
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

  struct hdc_deleter
  {
    int state_index = 0;

    hdc_deleter() = default;

    hdc_deleter(int state) : state_index(state)
    {
    }


    void operator()(HDC dc)
    {
      if (state_index)
      {
        assert(RestoreDC(dc, state_index));
      }
      assert(DeleteDC(dc));
    }
  };

  struct gdi_no_deleter
  {
    void operator()(HGDIOBJ window)
    {
    }
  };

  using bitmap = win32::auto_handle<HBITMAP, gdi_deleter>;
  using brush = win32::auto_handle<HBRUSH, gdi_deleter>;
  using palette = win32::auto_handle<HPALETTE, gdi_deleter>;
  using pen = win32::auto_handle<HPEN, gdi_deleter>;
  using font = win32::auto_handle<HFONT, gdi_deleter>;

  using bitmap_ref = win32::auto_handle<HBITMAP, gdi_no_deleter>;
  using icon_ref = win32::auto_handle<HICON, gdi_no_deleter>;
  using brush_ref = win32::auto_handle<HBRUSH, gdi_no_deleter>;
  using pen_ref = win32::auto_handle<HPEN, gdi_no_deleter>;
  using palette_ref = win32::auto_handle<HPALETTE, gdi_no_deleter>;
  using font_ref = win32::auto_handle<HFONT, gdi_no_deleter>;

  struct drawing_context_ref : win32::auto_handle<HDC, gdi_no_deleter>
  {
    using base = win32::auto_handle<HDC, gdi_no_deleter>;
    using base::base;


    auto FillRect(const ::RECT& pos, HBRUSH brush)
    {
      return ::FillRect(*this, &pos, brush);
    }
  };

  struct memory_drawing_context : win32::auto_handle<HDC, hdc_deleter>
  {
    using base = win32::auto_handle<HDC, hdc_deleter>;
    
    memory_drawing_context(drawing_context_ref other, bool auto_restore = true) : base(::CreateCompatibleDC(other), hdc_deleter(auto_restore ? 1 : 0))
    {
      if (auto_restore)
      {
        this->get_deleter().state_index = ::SaveDC(*this);
      }
    }
  };

  struct drawing_context : win32::auto_handle<HDC, hdc_releaser>
  {
    using base = win32::auto_handle<HDC, hdc_releaser>;
    using base::base;

    drawing_context(window_ref window) : base(::GetDC(window), hdc_releaser(window))
    {
    }

    drawing_context(window_ref window, ::PAINTSTRUCT paint_info) : base(::BeginPaint(window, &paint_info), hdc_releaser(window, std::move(paint_info)))
    {
    }

    static drawing_context from_screen()
    {
      return drawing_context(window_ref(nullptr));
    }

    auto FillRect(const ::RECT& pos, HBRUSH brush)
    {
      return ::FillRect(*this, &pos, brush);
    }
  };

  drawing_context GetDC(std::optional<window_ref> window = std::nullopt);


}// namespace win32::gdi

#endif// !WIN32_SHAPES_HPP
