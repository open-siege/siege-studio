#ifndef WIN32_DRAWING_HPP
#define WIN32_DRAWING_HPP

#include <siege/platform/win/desktop/window.hpp>
#include <siege/platform/win/auto_handle.hpp>
#undef NDEBUG
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

  struct icon_deleter
  {
    void operator()(HICON gdi_obj)
    {
      assert(::DestroyIcon(gdi_obj) == TRUE);
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


  template<typename TDeleter>
  struct bitmap_base : win32::auto_handle<HBITMAP, TDeleter>
  {
    using base = win32::auto_handle<HBITMAP, TDeleter>;
    using base::base;

    SIZE get_size() const
    {
      BITMAP bitmap;
      if (this->get() && ::GetObjectW(*this, sizeof(BITMAP), &bitmap) > 0)
      {
        return SIZE{ .cx = bitmap.bmWidth, .cy = bitmap.bmHeight };
      }

      return {};
    }

    std::span<RGBQUAD> get_pixels() const
    {
      BITMAP bitmap;
      if (this->get() && ::GetObjectW(*this, sizeof(BITMAP), &bitmap) > 0 && bitmap.bmBits && bitmap.bmBitsPixel == 32)
      {
        return std::span<RGBQUAD>((RGBQUAD*)bitmap.bmBits, bitmap.bmWidth * bitmap.bmHeight);
      }

      return {};
    }

    std::span<std::byte> get_pixels_as_bytes() const
    {
      BITMAP bitmap;
      if (this->get() && ::GetObjectW(*this, sizeof(BITMAP), &bitmap) > 0 && bitmap.bmBits && bitmap.bmBitsPixel == 32)
      {
        return std::span<std::byte>((std::byte*)bitmap.bmBits, bitmap.bmWidthBytes * bitmap.bmHeight);
      }

      return {};
    }
  };


  struct bitmap_ref : bitmap_base<gdi_no_deleter>
  {
    using base = bitmap_base<gdi_no_deleter>;
    using base::base;
  };

  struct bitmap : bitmap_base<gdi_deleter>
  {
    using base = bitmap_base<gdi_deleter>;
    using base::base;

    enum shared_handle : bool
    {
      skip_shared_handle,
      create_shared_handle
    };

    bitmap_ref ref()
    {
      return bitmap_ref(get());
    }

    bitmap(SIZE size, shared_handle should_create = create_shared_handle) : base([&] {
                                                                              HANDLE shared_memory = nullptr;

                                                                              if (should_create)
                                                                              {
                                                                                shared_memory = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_EXECUTE_READWRITE, 0, size.cx * size.cy * sizeof(std::uint32_t), nullptr);
                                                                              }

                                                                              BITMAPINFO info{
                                                                                .bmiHeader{
                                                                                  .biSize = sizeof(BITMAPINFOHEADER),
                                                                                  .biWidth = LONG(size.cx),
                                                                                  .biHeight = LONG(size.cy),
                                                                                  .biPlanes = 1,
                                                                                  .biBitCount = 32,
                                                                                  .biCompression = BI_RGB }
                                                                              };
                                                                              void* pixels = nullptr;
                                                                              return ::CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &pixels, shared_memory, 0);
                                                                            }())
    {
    }
  };

  

  using icon = win32::auto_handle<HICON, icon_deleter>;
  using brush = win32::auto_handle<HBRUSH, gdi_deleter>;
  using cursor = win32::auto_handle<HCURSOR, gdi_deleter>;
  using palette = win32::auto_handle<HPALETTE, gdi_deleter>;
  using pen = win32::auto_handle<HPEN, gdi_deleter>;
  using font = win32::auto_handle<HFONT, gdi_deleter>;

  using icon_ref = win32::auto_handle<HICON, gdi_no_deleter>;
  using cursor_ref = win32::auto_handle<HCURSOR, gdi_no_deleter>;
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

    memory_drawing_context(bool auto_restore = true) : base(nullptr, hdc_deleter(auto_restore ? 1 : 0))
    {
      auto screen_dc = ::GetDC(nullptr);
      auto dc = ::CreateCompatibleDC(screen_dc);

      ReleaseDC(nullptr, screen_dc);
      this->reset(dc);

      if (auto_restore)
      {
        this->get_deleter().state_index = ::SaveDC(*this);
      }
    }

    memory_drawing_context(drawing_context_ref other,
      bool auto_restore = true) : base(::CreateCompatibleDC(other), hdc_deleter(auto_restore ? 1 : 0))
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
