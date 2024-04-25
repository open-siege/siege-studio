#ifndef WIN32_SHAPES_HPP
#define WIN32_SHAPES_HPP

#include <siege/platform/win/gaming/win32_user32.hpp>

namespace win32
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
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
