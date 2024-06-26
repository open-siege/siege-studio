#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include "pal_controller.hpp"

namespace siege::views
{
	struct pal_view : win32::window_ref
	{
		pal_controller controller;

		std::vector<win32::gdi_brush> brushes;

		win32::static_control render_view;
		win32::list_box selection;

		pal_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
		{

		}

		auto on_create(const win32::create_message&)
		{
			auto control_factory = win32::window_factory(ref());

			render_view = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | SS_OWNERDRAW });

			selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
				.style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS
				});

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
					auto& colours = controller.get_palette(0);

					brushes.reserve(colours.size());

					::COLORREF temp;
					
					for (auto i = 0u; i < colours.size(); ++i)
					{
						auto temp_colour = colours[i].colour;
						temp_colour.flags = std::byte{};
						std::memcpy(&temp, &temp_colour, sizeof(temp));
						brushes.emplace_back(::CreateSolidBrush(temp));
					}

					for(auto i = 1u; i <= size; ++i)
					{
						selection.InsertString(-1, L"Palette " + std::to_wstring(i));
					
					}
				
					return TRUE;
				}

				return FALSE;
			}

			return FALSE;
		}

		auto on_draw_item(win32::draw_item_message message)
		{
			if (message.item.itemAction == ODA_DRAWENTIRE)
			{
				auto context = win32::gdi_drawing_context_ref(message.item.hDC);

				auto total_width = message.item.rcItem.right - message.item.rcItem.left;
				auto total_height = message.item.rcItem.bottom - message.item.rcItem.top;
				auto total_area = total_width * total_height;
				auto best_size = (int)std::sqrt(double(total_area) / brushes.size());

				win32::rect pos{};
				pos.right = best_size;
				pos.bottom = best_size;

				for (auto i = 0; i < brushes.size(); ++i)
				{
					context.FillRect(pos, brushes[i]);
					pos.Offset(best_size, 0);

					if (pos.right >= total_width)
					{
						pos.left = 0;
						pos.right = best_size;
						pos.Offset(0, best_size);
					}
				}
			}

			return TRUE;
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
	};
}

#endif