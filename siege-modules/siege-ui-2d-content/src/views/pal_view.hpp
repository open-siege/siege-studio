#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/win32_controls.hpp>
#include <siege/platform/win/desktop/win32_shapes.hpp>
#include "pal_controller.hpp"

namespace siege::views
{
	struct pal_view
	{
		constexpr static auto formats = std::array<std::wstring_view, 4>{{ L".pal", L".ipl", L".ppl", L".dpl"}};

		win32::hwnd_t self;
		pal_controller controller;
		win32::paint_context paint_data;

		std::array<HBRUSH, 16> brushes;

		pal_view(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
		{
			for (auto i = 0u; i < brushes.size(); ++i)
			{
				brushes[i] = CreateSolidBrush(RGB(i, i * 8, i * 4));
			}
		}

		~pal_view()
		{
			for (auto brush : brushes)
			{
				DeleteObject(brush);
			}
		}

		auto on_create(const win32::create_message&)
		{
			/*win32::CreateWindowExW(CREATESTRUCTW{
				.hwndParent = self,
				.cy = 200,
				.cx = 100,
				.y = 100,
				.style = WS_CHILD | WS_VISIBLE,
				.lpszName = L"Hello world",
				.lpszClass = win32::button::class_name,			
			});*/


			return 0;
		}

		auto on_paint(win32::paint_message)
		{
			auto context = paint_data.BeginPaint(self);

			win32::rect pos{};
			pos.right = 100;
			pos.bottom = 100;

			for (auto i = 0; i < brushes.size(); ++i)
			{
				context.FillRect(pos, brushes[i]);
				pos.Offset(100, 0);
			}

			return 0;
		}

		auto on_copy_data(win32::copy_data_message<char> message)
		{
			std::spanstream stream(message.data);
			
			if (controller.is_pal(stream))
			{
				return controller.load_palettes(stream) > 0 ? TRUE : FALSE;
			}

			return FALSE;
		}

		std::optional<LRESULT> on_get_object(win32::get_object_message message)
		{
			if (message.object_id == OBJID_NATIVEOM)
			{
                auto collection = std::make_unique<win32::com::OwningCollection<std::unique_ptr<IStream, void(*)(IStream*)>>>();

				return LresultFromObject(__uuidof(IDispatch), message.flags, static_cast<IDispatch*>(collection.release()));   
			}

			return std::nullopt;
		}

		static bool is_pal(std::istream& raw_data)
		{
			return false;
		}
	};
}

#endif