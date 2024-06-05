#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include "dts_controller.hpp"

namespace siege::views
{
	struct dts_view : win32::window_ref
	{
		dts_controller controller;

		win32::static_control render_view;
		win32::list_box selection;

		dts_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
		{

		}

		auto on_create(const win32::create_message&)
		{
			auto control_factory = win32::window_factory(ref());

			render_view = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD });

			selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
				.style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS,
				});

			selection.InsertString(-1, L"Palette 1");
			selection.InsertString(-1, L"Palette 2");
			selection.InsertString(-1, L"Palette 3");

			return 0;
		}

		auto on_size(win32::size_message sized)
		{
			auto left_size = SIZE{ .cx = (sized.client_size.cx / 3) * 2, .cy = sized.client_size.cy };
            auto right_size = SIZE{ .cx = sized.client_size.cx - left_size.cx, .cy = sized.client_size.cy };

			render_view.SetWindowPos(left_size);
			render_view.SetWindowPos(POINT{});

			selection.SetWindowPos(right_size);
			selection.SetWindowPos(POINT{.x = left_size.cx});

			return 0;
		}


		auto on_copy_data(win32::copy_data_message<char> message)
		{
			std::spanstream stream(message.data);
			
			if (controller.is_dts(stream))
			{
				auto size = controller.load_shape(stream);

				if (size > 0)
				{
					return TRUE;
				}

				return FALSE;
			}

			return FALSE;
		}

	};
}

#endif