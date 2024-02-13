#ifndef WIN32_MESSAGES_HPP
#define WIN32_MESSAGES_HPP

#include <array>
#include <functional>
#include <variant>
#include <optional>
#include <bit>
#include <cstdint>
#include <bitset>
#include <algorithm>
#include <wtypes.h>
#include <WinDef.h>
#include <WinUser.h>
#include <windowsx.h>

namespace win32
{
	static_assert(sizeof(void*) == sizeof(LONG_PTR));

	using wparam_t = WPARAM;
	using lparam_t = LPARAM;
	using wparam_array = std::array<std::byte, sizeof(WPARAM)>;
	using lparam_array = std::array<std::byte, sizeof(LPARAM)>;
	static_assert(sizeof(std::uint32_t) == sizeof(UINT));

	using hwnd_t = HWND;

	struct message
	{
		std::uint32_t message;
		wparam_t wParam;
		lparam_t lParam;
	};

	struct create_message
	{
		constexpr static std::uint32_t id = WM_CREATE;
		CREATESTRUCTW& data;

		create_message(wparam_t, lparam_t lParam) : data(*std::bit_cast<CREATESTRUCTW*>(lParam))
		{
		}
	};

	struct size_message
	{
		constexpr static std::uint32_t id = WM_SIZE;

		int type;
		int width;
		int height;

		size_message(wparam_t wParam, lparam_t lParam) : 
			type(wParam),
			width(LOWORD(lParam)),	
			height(HIWORD(lParam))
		{
		}
	}

	struct non_client_create_message
	{
		constexpr static std::uint32_t id = WM_NCCREATE;
		CREATESTRUCTW& data;

		non_client_create_message(wparam_t, lparam_t lParam) : data(*std::bit_cast<CREATESTRUCTW*>(lParam))
		{
		}
	};

	struct init_dialog_message
	{
		constexpr static std::uint32_t id = WM_INITDIALOG;
		hwnd_t default_focus_control;

		init_dialog_message(wparam_t wParam, lparam_t) : default_focus_control(std::bit_cast<hwnd_t>(wParam))
		{
		}
	};

	struct destroy_message
	{
		constexpr static std::uint32_t id = WM_DESTROY;

		destroy_message(wparam_t, lparam_t)
		{
		}
	};

	struct command_message
	{
		constexpr static std::uint32_t id = WM_COMMAND;
		
		hwnd_t sender;
		int identifier;
		int notification_code;

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
	};

	struct notify_message
	{
		constexpr static std::uint32_t id = WM_NOTIFY;

		hwnd_t sender;
		int identifier;
		int notification_code;

		notify_message(wparam_t, lparam_t lParam) : 
			sender(nullptr),
			identifier(0),
			notification_code(0)
		{
			NMHDR* header = std::bit_cast<NMHDR*>(lParam);
			sender = header->hwndFrom;
			identifier = header->idFrom;
			notification_code = header->code;
		}
	};

	enum struct mouse_button : std::uint16_t
	{
		lbutton = 0,
		rbutton,
		mbutton,
		xbutton_1,
		xbutton_2
	};

	struct keyboard_message
	{
		std::uint16_t virtual_key_code;
		keyboard_message(wparam_t wParam, lparam_t) : virtual_key_code(std::uint16_t(wParam))
		{
		}

	};

	struct keyboard_key_up_message : keyboard_message
	{
		constexpr static std::uint32_t id = WM_KEYUP;	
		using keyboard_message::keyboard_message;
	};

	struct keyboard_key_down_message : keyboard_message
	{
		constexpr static std::uint32_t id = WM_KEYDOWN;
		using keyboard_message::keyboard_message;
	};

	struct keyboard_char_message
	{
		constexpr static std::uint32_t id = WM_CHAR;
		wchar_t translated_char;

		keyboard_char_message(wparam_t wParam, lparam_t) : translated_char(wchar_t(wParam))
		{
		}
	};

	struct mouse_message
	{
		std::int16_t x;
		std::int16_t y;
		std::bitset<8> mouse_buttons_down;
		bool control_key_is_down;
		bool shift_key_is_down;

		static bool matches_message(const std::array<uint32_t, 4>& ids, std::uint32_t value)
		{
			return std::any_of(ids.begin(), ids.end(), [value](auto other) { return value == other; });
		}
	};

	struct mouse_button_message : mouse_message
	{
		mouse_button source;
	};

	struct mouse_move_message : mouse_message
	{
		constexpr static std::uint32_t id = WM_MOUSEMOVE;
	};

	struct mouse_hover_message : mouse_message
	{
		constexpr static std::uint32_t id = WM_MOUSEHOVER;
	};

	struct mouse_button_double_click_message : mouse_button_message
	{
		constexpr static auto ids = std::array<uint32_t, 4>{{WM_LBUTTONDBLCLK, WM_RBUTTONDBLCLK, WM_MBUTTONDBLCLK, WM_XBUTTONDBLCLK}};
		
		static bool matches_message(std::uint32_t value)
		{
			return mouse_message::matches_message(ids, value);
		}
	};

	struct mouse_button_down_message : mouse_button_message
	{
		constexpr static auto ids = std::array<uint32_t, 4>{{WM_LBUTTONDOWN, WM_RBUTTONDOWN, WM_MBUTTONDOWN, WM_XBUTTONDOWN}};

		static bool matches_message(std::uint32_t value)
		{
			return mouse_message::matches_message(ids, value);
		}
	};

	struct mouse_button_up_message : mouse_button_message
	{
		constexpr static auto ids = std::array<uint32_t, 4>{{WM_LBUTTONUP, WM_RBUTTONUP, WM_MBUTTONUP, WM_XBUTTONUP}};

		static bool matches_message(std::uint32_t value)
		{
			return mouse_message::matches_message(ids, value);
		}
	};

	struct mouse_wheel_message : mouse_message
	{
		constexpr static std::uint32_t id = WM_MOUSEWHEEL;
		std::int16_t delta;
	};

	struct mouse_leave_message
	{
		constexpr static std::uint32_t id = WM_MOUSELEAVE;
	};

	using window_message = std::variant<message,
		command_message,
		non_client_create_message,
		create_message,
		init_dialog_message,
		destroy_message,
		keyboard_key_down_message,
		keyboard_key_up_message,
		keyboard_char_message,
		mouse_move_message,
		mouse_hover_message,
		mouse_leave_message,
		mouse_button_double_click_message,
		mouse_button_down_message,
		mouse_button_up_message,
		mouse_wheel_message>;

	static_assert(sizeof(window_message) < sizeof(MSG));

	constexpr window_message make_window_message(std::uint32_t message, wparam_t wParam, lparam_t lParam) noexcept
	{
		if (message == mouse_move_message::id || 
			message == mouse_hover_message::id ||
			message == mouse_wheel_message::id ||
			mouse_button_double_click_message::matches_message(message) ||
			mouse_button_down_message::matches_message(message) ||
			mouse_button_up_message::matches_message(message))
		{
			mouse_button_message mouse_data{};
			mouse_data.x = GET_X_LPARAM(lParam);
			mouse_data.y = GET_Y_LPARAM(lParam);

			auto set_key_state = [&](auto keys) {
				mouse_data.control_key_is_down = keys & MK_CONTROL;
				mouse_data.shift_key_is_down = keys & MK_SHIFT;
				mouse_data.mouse_buttons_down[std::size_t(mouse_button::lbutton)] = keys & MK_LBUTTON;
				mouse_data.mouse_buttons_down[std::size_t(mouse_button::rbutton)] = keys & MK_RBUTTON;
				mouse_data.mouse_buttons_down[std::size_t(mouse_button::mbutton)] = keys & MK_MBUTTON;
				mouse_data.mouse_buttons_down[std::size_t(mouse_button::xbutton_1)] = keys & MK_XBUTTON1;
				mouse_data.mouse_buttons_down[std::size_t(mouse_button::xbutton_2)] = keys & MK_XBUTTON2;
			};

			if (message == mouse_button_up_message::ids[0] || message == mouse_button_down_message::ids[0] || message == mouse_button_double_click_message::ids[0])
			{
				mouse_data.source = mouse_button::lbutton;
			}
			else if (message == mouse_button_up_message::ids[1] || message == mouse_button_down_message::ids[1] || message == mouse_button_double_click_message::ids[1])
			{
				mouse_data.source = mouse_button::rbutton;
			}
			else if (message == mouse_button_up_message::ids[2] || message == mouse_button_down_message::ids[2] || message == mouse_button_double_click_message::ids[2])
			{
				mouse_data.source = mouse_button::mbutton;
			}
			else if (message == mouse_button_up_message::ids[3] || message == mouse_button_down_message::ids[3] || message == mouse_button_double_click_message::ids[3])
			{
				mouse_data.source = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? mouse_button::xbutton_1 : mouse_button::xbutton_2;

				auto keys = GET_KEYSTATE_WPARAM(wParam);
				set_key_state(keys);
			}
			else if (message == mouse_wheel_message::id)
			{
				auto keys = GET_KEYSTATE_WPARAM(wParam);
                set_key_state(keys);
			}
			else
			{
                set_key_state(wParam);
			}

			if (message == mouse_move_message::id)
			{
				return mouse_move_message{std::move(mouse_data)};
			}
			else if (message == mouse_hover_message::id)
			{
				return mouse_hover_message{std::move(mouse_data)};
			}
			else if (message == mouse_wheel_message::id)
			{
				auto delta = GET_WHEEL_DELTA_WPARAM(wParam);
				return mouse_wheel_message{std::move(mouse_data), delta};
			}
			else if (mouse_button_down_message::matches_message(message))
			{
				return mouse_button_down_message{std::move(mouse_data)};
			}
			else if (mouse_button_up_message::matches_message(message))
			{
				return mouse_button_up_message{std::move(mouse_data)};
			}
			else if (mouse_button_double_click_message::matches_message(message))
			{
				return mouse_button_double_click_message{std::move(mouse_data)};
			}
		}

		if (message == mouse_leave_message::id)
		{
			return mouse_leave_message{};
		}

		return win32::message{
			.message = message,
			.wParam = wParam,
			.lParam = lParam
		};
	}

	constexpr window_message make_window_message(const MSG& message) noexcept
	{
		return make_window_message(message.message, message.wParam, message.lParam);
	}

	template<class... Ts>
	struct overloaded : Ts... { using Ts::operator()...; };
}

#endif // !WINDOWSHPP
