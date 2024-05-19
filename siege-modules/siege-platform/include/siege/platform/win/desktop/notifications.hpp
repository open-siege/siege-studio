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
#include <wtypes.h>
#include <WinDef.h>
#include <WinUser.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <siege/platform/win/desktop/messages.hpp>

namespace win32
{
	#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		struct NMHDR {
			  HWND     hwndFrom;
			  UINT_PTR idFrom;
			  UINT     code;
		};
	#endif

	template<typename TNotification = NMHDR>
	struct notify_message_base : TNotification
	{
		constexpr static std::uint32_t id = WM_NOTIFY;

		notify_message_base(NMHDR base) : TNotification{}
		{
			std::memcpy(this, &base, sizeof(NMHDR));
		}

		notify_message_base(wparam_t, lparam_t lParam) : TNotification{}
		{
			std::memcpy(this, (void*)lParam, sizeof(TNotification));
		}
	};

	using notify_message = notify_message_base<>;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	using tree_view_notification = notify_message_base<NMTREEVIEWW>;
	using header_notification = notify_message_base<NMHEADERW>;
	using list_view_item_activation = notify_message_base<NMITEMACTIVATE>;
#endif

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

		inline wparam_t wparam() const noexcept
		{
			return MAKEWPARAM(identifier, notification_code);
		}

		inline lparam_t lparam() const noexcept
		{
			return std::bit_cast<lparam_t>(sender);
		}

		operator NMHDR()
		{
			return NMHDR{.hwndFrom = sender, .idFrom = identifier, .code = notification_code };
		}
	};
}

#endif // !WINDOWSHPP
