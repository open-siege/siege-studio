#ifndef WIN32_NOTIFICATIONS_HPP
#define WIN32_NOTIFICATIONS_HPP

#include <array>
#include <functional>
#include <variant>
#include <optional>
#include <bit>
#include <cstdint>
#include <bitset>
#include <algorithm>
#include <span>
#include <bit>
#include <wtypes.h>
#include <WinDef.h>
#include <WinUser.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <siege/platform/win/desktop/messages.hpp>

namespace win32
{
	struct menu_command
	{
		constexpr static std::uint32_t id = WM_COMMAND;
		int identifier;

		menu_command(wparam_t wParam, lparam_t) : 
			identifier(LOWORD(wParam))
		{
		}

		static bool is_menu_command(wparam_t wParam, lparam_t lParam)
		{
			return HIWORD(wParam) == 0 && lParam == 0;
		}
	};

	struct accelerator_command
	{
		constexpr static std::uint32_t id = WM_COMMAND;
		
		int notification_code;	

		accelerator_command(wparam_t wParam, lparam_t) : notification_code(HIWORD(wParam))
		{
		}

		static bool is_accelerator_command(wparam_t wParam, lparam_t lParam)
		{
			return HIWORD(wParam) == 1 && lParam == 0;
		}
	};

	struct command_message
	{
		constexpr static std::uint32_t id = WM_COMMAND;
		
		hwnd_t sender;
		std::size_t identifier;
		std::uint32_t notification_code;

		command_message(wparam_t wParam, lparam_t lParam) : 
			sender(std::bit_cast<hwnd_t>(lParam)),
			identifier(LOWORD(wParam)),
			notification_code(HIWORD(wParam))
		{
		}
	};
}

#endif // !WINDOWSHPP
