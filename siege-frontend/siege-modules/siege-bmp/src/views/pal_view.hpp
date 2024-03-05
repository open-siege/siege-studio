#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include "win32_controls.hpp"
#include "pal_controller.hpp"

namespace siege::views
{
	struct pal_view
	{
		constexpr static std::u8string_view formats = u8".pal .ipl .ppl .dpl";

		win32::hwnd_t self;
		pal_controller controller;
		std::vector<std::vector<pal_controller::palette>> palettes;

		pal_view(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
		{
		}

		auto on_create(const win32::create_message&)
		{
			auto parent_size = win32::GetClientRect(self);
			assert(parent_size);

			auto button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
							.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
							.cx = short(parent_size->right),  
							.cy = short(parent_size->bottom)     
							}, self, win32::button::class_name, L"Pal window");

			return 0;
		}

		auto on_size(win32::size_message sized)
		{
			win32::ForEachDirectChildWindow(self, [&](auto child) {
				win32::SetWindowPos(child, sized.client_size);
			});

			return std::nullopt;
		}

		auto on_copy_data(win32::copy_data_message<char> message)
		{
			std::spanstream stream(message.data);
			
			if (controller.is_pal(stream))
			{
				palettes = controller.load_palettes(stream);

				return TRUE;
			}

			return FALSE;
		}

		static bool is_pal(std::istream& raw_data)
		{
			return false;
		}
	};
}

#endif