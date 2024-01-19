#ifndef WINDOWSHPP
#define WINDOWSHPP

#include "framework.h"
#include "Resource.h"
#include <array>
#include <functional>
#include <variant>
#include <optional>

static_assert(sizeof(void*) == sizeof(LONG_PTR));

constexpr static auto WM_COUT = WM_APP + 1;

using wparam_array = std::array<std::byte, sizeof(WPARAM)>;
using lparam_array = std::array<std::byte, sizeof(LPARAM)>;

struct message
{
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};

struct command_message
{
	int notification_code;
	int identifier;
	HWND handle;

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
	constexpr static UINT id = WM_CREATE;
	CREATESTRUCTW& data;
};

struct pre_create_message
{
	constexpr static UINT id = WM_NCCREATE;
	CREATESTRUCTW& data;
};

struct init_dialog_message
{
	constexpr static UINT id = WM_INITDIALOG;
	HWND default_focus_control;
};

struct destroy_message
{
	constexpr static UINT id = WM_DESTROY;
};

using window_message = std::variant<message,
	command_message,
	pre_create_message,
	create_message,
	init_dialog_message,
	destroy_message>;

window_message make_window_message(UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
	if (message == command_message::id)
	{
		return command_message{
			.notification_code = HIWORD(wParam),
			.identifier = LOWORD(wParam),
			.handle = reinterpret_cast<HWND>(lParam)
		};
	}

	if (message == pre_create_message::id)
	{
		return pre_create_message{
			.data = *reinterpret_cast<CREATESTRUCTW*>(lParam)
		};
	}

	if (message == create_message::id)
	{
		return create_message{
			.data = *reinterpret_cast<CREATESTRUCTW*>(lParam)
		};
	}

	if (message == init_dialog_message::id)
	{
		return init_dialog_message{
			.default_focus_control = reinterpret_cast<HWND>(wParam)
		};
	}

	if (message == destroy_message::id)
	{
		return destroy_message{};
	}

	return ::message{
		.message = message,
		.wParam = wParam,
		.lParam = lParam
	};
}

window_message make_window_message(const MSG& message) noexcept
{
	return make_window_message(message.message, message.wParam, message.lParam);
}

struct main_window
{
	std::function<std::optional<LRESULT>(main_window&, window_message)> HandleMessage;
	HWND handle;

	static auto get_descriptor(HINSTANCE hInstance)
	{
		constexpr static auto descriptor = WNDCLASSEXW{
		.cbSize{sizeof(WNDCLASSEX)},
		.style{CS_HREDRAW | CS_VREDRAW},
		.lpfnWndProc{WindowHandler},
		.cbClsExtra{0},
		.cbWndExtra{0},
		.lpszClassName{L"SiegeLauncherMainWindow"}
		};
		auto temp = descriptor;
		temp.hInstance = hInstance;
		temp.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SIEGELAUNCHERWIN32));
		temp.hCursor = LoadCursorW(hInstance, IDC_ARROW);
		temp.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		temp.lpszMenuName = MAKEINTRESOURCEW(IDC_SIEGELAUNCHERWIN32);
		temp.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL));
		return temp;
	}

	static LRESULT CALLBACK WindowHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		main_window* window = nullptr;

		if (message == WM_NCCREATE)
		{
			auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			window = reinterpret_cast<main_window*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		}
		else
		{
			window = reinterpret_cast<main_window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}

		if (window && window->HandleMessage)
		{
			window->handle = hWnd;
			auto result = window->HandleMessage(*window, make_window_message(message, wParam, lParam));

			if (result.has_value())
			{
				return result.value();
			}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
};


struct dialog
{
	std::function<INT_PTR(dialog&, window_message)> HandleMessage;
	HWND handle;

	static INT_PTR CALLBACK HandleAboutDialogMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		dialog* child = nullptr;

		if (message == WM_INITDIALOG)
		{
			child = reinterpret_cast<dialog*>(lParam);
			SetWindowLongPtr(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(child));
			return (INT_PTR)TRUE;
		}
		else
		{
			child = reinterpret_cast<dialog*>(GetWindowLongPtr(hDlg, DWLP_USER));
		}

		if (child && child->HandleMessage)
		{
			child->handle = hDlg;
			auto result = child->HandleMessage(*child, make_window_message(message, wParam, lParam));

			if (message == WM_DESTROY)
			{
				SetWindowLongPtr(child->handle, DWLP_USER, reinterpret_cast<LONG_PTR>(nullptr));
				child->HandleMessage = nullptr;
				child->handle = 0;
				return (INT_PTR)TRUE;
			}

			return result;
		}

		return (INT_PTR)FALSE;
	}
};

template<typename Control>
struct control
{
	HWND handle;

	using callback_type = std::function<std::optional<LRESULT>(Control&, UINT_PTR uIdSubclass, window_message)>;
	callback_type HandleMessage;

	static LRESULT CALLBACK CustomButtonHandler(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		auto* child = reinterpret_cast<Control*>(dwRefData);

		if (child && child->HandleMessage)
		{
			auto result = child->HandleMessage(*child, uIdSubclass, make_window_message(uMsg, wParam, lParam));

			if (uMsg == WM_DESTROY)
			{
				RemoveWindowSubclass(child->handle, Control::CustomButtonHandler, uIdSubclass);
				child->HandleMessage = nullptr;
				child->handle = 0;
				return TRUE;
			}

			if (result.has_value())
			{
				return result.value();
			}
		}

		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	control(HWND handle, callback_type handler)
		: handle(handle), HandleMessage(std::move(handler))
	{
		SetWindowSubclass(handle, control::CustomButtonHandler, 0, reinterpret_cast<DWORD_PTR>(this));
	}

	control(const control&) = delete;

	control(control&& other) noexcept
		: control(other.handle, std::move(other.HandleMessage))
	{
		SetWindowSubclass(handle, control::CustomButtonHandler, 0, reinterpret_cast<DWORD_PTR>(this));
	}

	control& operator=(control&& other) noexcept
	{
		this->handle = std::move(other.handle);
		this->HandleMessage = std::move(other.HandleMessage);
		return *this;
	}
};

struct button : control<button>
{
	using control::control;
};

struct combo_box : control<combo_box>
{
	using control::control;
};

struct edit : control<edit>
{
	using control::control;
};

struct list_box : control<list_box>
{
	using control::control;
};

struct scroll_bar : control<scroll_bar>
{
	using control::control;
};

struct static_text : control<static_text>
{
	using control::control;
};

using client_control = std::variant<button, combo_box, edit, list_box, scroll_bar, static_text>;

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

#endif // !WINDOWSHPP
