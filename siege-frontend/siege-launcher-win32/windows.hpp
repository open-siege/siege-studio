#ifndef WINDOWSHPP
#define WINDOWSHPP

#include "framework.h"
#include "Resource.h"
#include <array>
#include <functional>
#include <variant>
#include <optional>
#include <bit>

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

static_assert(sizeof(window_message) < sizeof(MSG));

constexpr window_message make_window_message(UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
	if (message == command_message::id)
	{
		return command_message{
			.notification_code = HIWORD(wParam),
			.identifier = LOWORD(wParam),
			.handle = std::bit_cast<HWND>(lParam)
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
			.default_focus_control = std::bit_cast<HWND>(wParam)
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

constexpr window_message make_window_message(const MSG& message) noexcept
{
	return make_window_message(message.message, message.wParam, message.lParam);
}

struct window
{
	using callback_type = std::move_only_function<std::optional<LRESULT>(window&, window_message)>;
	
	HWND handle;
	callback_type HandleMessage;
	

	window(WNDCLASSEXW descriptor, CREATESTRUCTW params, callback_type handler) 
		: handle(0),
		HandleMessage(std::move(handler))
		{
			descriptor.cbSize = sizeof(WNDCLASSEXW);
			descriptor.lpfnWndProc = WindowHandler;
			
			if (GetClassInfoExW(descriptor.hInstance, descriptor.lpszClassName, &descriptor) == FALSE)
			{
				RegisterClassExW(&descriptor);
			}

			handle = CreateWindowExW(params.dwExStyle,
				descriptor.lpszClassName,
				params.lpszName,
				params.style,
				params.x,
				params.y,
				params.cx,
				params.cy,
				params.hwndParent,
				params.hMenu,
				descriptor.hInstance,
				std::bit_cast<LPVOID>(this));
		}

	static LRESULT CALLBACK WindowHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		window* self = nullptr;

		if (message == WM_NCCREATE)
		{
			auto* pCreate = std::bit_cast<CREATESTRUCT*>(lParam);
			self = std::bit_cast<window*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
			self->handle = hWnd;
		}
		else
		{
			self = std::bit_cast<window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}

		if (self && self->HandleMessage)
		{
			auto result = self->HandleMessage(*self, make_window_message(message, wParam, lParam));

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
	using callback_type = std::move_only_function<INT_PTR(dialog&, window_message)>;
	HWND handle;
	callback_type HandleMessage;
	
	dialog(callback_type handler): handle(0), HandleMessage(std::move(handler))
	{
	}

	[[maybe_unused]] static auto show_modal(HWND parent, LPWSTR templateName, callback_type handler)
	{
		dialog temp{std::move(handler)};
		auto hInstance = std::bit_cast<HINSTANCE>(GetWindowLongPtrW(parent, GWLP_HINSTANCE));
		return DialogBoxParamW(hInstance, templateName, parent, dialog::HandleAboutDialogMessage, std::bit_cast<LPARAM>(&temp));
	}
	

	static INT_PTR CALLBACK HandleAboutDialogMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		dialog* child = nullptr;

		if (message == WM_INITDIALOG)
		{
			child = std::bit_cast<dialog*>(lParam);
			
			if (child->handle == 0)
			{
	            child->handle = hDlg;
			}

			SetWindowLongPtr(hDlg, DWLP_USER, std::bit_cast<LONG_PTR>(child));
			return (INT_PTR)TRUE;
		}
		else
		{
			child = std::bit_cast<dialog*>(GetWindowLongPtr(hDlg, DWLP_USER));
		}

		if (child && child->HandleMessage)
		{
			auto result = child->HandleMessage(*child, make_window_message(message, wParam, lParam));

			if (message == WM_DESTROY)
			{
				SetWindowLongPtr(child->handle, DWLP_USER, std::bit_cast<LONG_PTR>(nullptr));
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

	using callback_type = std::move_only_function<std::optional<LRESULT>(Control&, UINT_PTR uIdSubclass, window_message)>;
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
		SetWindowSubclass(handle, control::CustomButtonHandler, 0, std::bit_cast<DWORD_PTR>(this));
	}

	control(CREATESTRUCTW params, callback_type handler)
		: control(CreateWindowExW(
			params.dwExStyle,
			params.lpszClass ? params.lpszClass : Control::class_name,
			params.lpszName,
			params.style,
			params.x,
			params.y,
			params.cx,
			params.cy,
			params.hwndParent,
			params.hMenu,
			params.hInstance != 0 ? params.hInstance : std::bit_cast<HINSTANCE>(GetWindowLongPtrW(params.hwndParent, GWLP_HINSTANCE)),
			params.lpCreateParams
		), std::move(handler)) 
	{
	}

	control(const control&) = delete;

	control(control&& other) noexcept
		: control(other.handle, std::move(other.HandleMessage))
	{
		if (IsWindow(handle))
		{
			SetWindowSubclass(handle, control::CustomButtonHandler, 0, std::bit_cast<DWORD_PTR>(this));
		}
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

	constexpr static auto class_name = WC_BUTTONW;
};

struct combo_box : control<combo_box>
{
	using control::control;

	constexpr static auto class_name = WC_COMBOBOXW;
};

struct edit : control<edit>
{
	using control::control;

	constexpr static auto class_name = WC_EDITW;
};

struct list_box : control<list_box>
{
	using control::control;

	constexpr static auto class_name = WC_LISTBOXW;
};

struct scroll_bar : control<scroll_bar>
{
	using control::control;
	
	constexpr static auto class_name = WC_SCROLLBARW;
};

struct static_text : control<static_text>
{
	using control::control;

	constexpr static auto class_name = WC_STATICW;
};

using client_control = std::variant<button, combo_box, edit, list_box, scroll_bar, static_text>;

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

#endif // !WINDOWSHPP
