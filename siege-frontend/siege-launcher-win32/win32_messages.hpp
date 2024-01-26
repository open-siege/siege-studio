#ifndef WIN32_MESSAGES_HPP
#define WIN32_MESSAGES_HPP

#include <array>
#include <functional>
#include <variant>
#include <optional>
#include <bit>
#include <cstdint>
#include <wtypes.h>
#include <WinDef.h>
#include <WinUser.h>

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

	struct command_message
	{
		int notification_code;
		int identifier;
		hwnd_t handle;

		constexpr static UINT id = WM_COMMAND;

		inline WPARAM wparam() const noexcept
		{
			return MAKEWPARAM(identifier, notification_code);
		}

		inline LPARAM lparam() const noexcept
		{
			return reinterpret_cast<LPARAM>(handle);
		}
	};

	struct create_message
	{
		constexpr static std::uint32_t id = WM_CREATE;
		CREATESTRUCTW& data;
	};

	struct pre_create_message
	{
		constexpr static std::uint32_t id = WM_NCCREATE;
		CREATESTRUCTW& data;
	};

	struct init_dialog_message
	{
		constexpr static std::uint32_t id = WM_INITDIALOG;
		HWND default_focus_control;
	};

	struct destroy_message
	{
		constexpr static std::uint32_t id = WM_DESTROY;
	};

	using window_message = std::variant<message,
		command_message,
		pre_create_message,
		create_message,
		init_dialog_message,
		destroy_message>;

	static_assert(sizeof(window_message) < sizeof(MSG));

	constexpr window_message make_window_message(std::uint32_t message, wparam_t wParam, lparam_t lParam) noexcept
	{
		if (message == command_message::id)
		{
			return command_message{
				.notification_code = HIWORD(wParam),
				.identifier = LOWORD(wParam),
				.handle = std::bit_cast<hwnd_t>(lParam)
			};
		}

		if (message == pre_create_message::id)
		{
			return pre_create_message{
				.data = *std::bit_cast<CREATESTRUCTW*>(lParam)
			};
		}

		if (message == create_message::id)
		{
			return create_message{
				.data = *std::bit_cast<CREATESTRUCTW*>(lParam)
			};
		}

		if (message == init_dialog_message::id)
		{
			return init_dialog_message{
				.default_focus_control = std::bit_cast<hwnd_t>(wParam)
			};
		}

		if (message == destroy_message::id)
		{
			return destroy_message{};
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
