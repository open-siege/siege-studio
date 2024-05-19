#ifndef WIN32_SHAPES_HPP
#define WIN32_SHAPES_HPP

#include <siege/platform/win/desktop/messages.hpp>
#include <siege/platform/win/auto_handle.hpp>

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

	struct gdi_drawing_context_ref : win32::auto_handle<HDC, gdi_no_deleter>
	{
		using base = win32::auto_handle<HDC, gdi_no_deleter>;
		using base::base;

		auto FillRect(const ::RECT& pos, HBRUSH brush)
		{
			return ::FillRect(*this, &pos, brush);
		}
	};


	struct paint_context : ::PAINTSTRUCT
	{
		paint_context() : ::PAINTSTRUCT{}
		{
		}

		struct drawing_context
		{
			paint_context& context;
			hwnd_t target;
			HDC dc;

			operator HDC()
			{
				return dc;
			}

			auto FillRect(const ::RECT& pos, HBRUSH brush)
			{
				return ::FillRect(dc, &pos, brush);
			}

			~drawing_context()
			{
				::EndPaint(target, &context);
			}
		};

		[[no_discard]] inline drawing_context BeginPaint(hwnd_t target)
		{
			return drawing_context{*this, target, ::BeginPaint(target, this)};
		}
	
	};

#endif

	struct rect : ::RECT
	{
		auto Offset(int dx, int dy)
		{
			return ::OffsetRect(this, dx, dy) == TRUE;
		}
	};

}

#endif // !WIN32_SHAPES_HPP
