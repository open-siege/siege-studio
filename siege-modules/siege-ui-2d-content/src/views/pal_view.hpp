#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/win32_common_controls.hpp>
#include <siege/platform/win/desktop/win32_drawing.hpp>
#include "pal_controller.hpp"

namespace siege::views
{
	struct pal_view : win32::window
	{
		constexpr static auto formats = std::array<std::wstring_view, 4>{{ L".pal", L".ipl", L".ppl", L".dpl"}};

		pal_controller controller;
		//win32::paint_context paint_data;

		std::array<win32::gdi_brush, 16> brushes;

		win32::static_control render_view;
		win32::list_box selection;

		pal_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window(self)
		{

		}

		auto on_create(const win32::create_message&)
		{
			auto control_factory = win32::window_factory(win32::window_ref(*this));

			render_view = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | SS_OWNERDRAW });

			selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
				.style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS | LBS_OWNERDRAWFIXED, 
				.lpszClass = L"MFC::CCheckListBox"
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
			
			if (controller.is_pal(stream))
			{
				auto size = controller.load_palettes(stream);

				if (size > 0)
				{
							
					for (auto i = 0u; i < brushes.size(); ++i)
					{
						brushes[i].reset(::CreateSolidBrush(RGB(i, i * 8, i * 4)));
					}
				
					return TRUE;
				}

				return FALSE;
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

		

		//auto on_paint(win32::paint_message)
		//{
		//	auto context = paint_data.BeginPaint(self);

		//	win32::rect pos{};
		//	pos.right = 100;
		//	pos.bottom = 100;

		//	for (auto i = 0; i < brushes.size(); ++i)
		//	{
		//		context.FillRect(pos, brushes[i]);
		//		pos.Offset(100, 0);
		//	}

		//	return 0;
		//}
	};
}

#endif