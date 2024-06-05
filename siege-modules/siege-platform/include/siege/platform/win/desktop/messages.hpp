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
#include <span>
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

	using hinstance_t = HINSTANCE;
    using lresult_t = LRESULT;

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
		SIZE client_size;

		size_message(wparam_t wParam, lparam_t lParam) : 
			type(wParam),
			client_size(SIZE{LOWORD(lParam), HIWORD(lParam)})
		{
		}
	};

	struct paint_message
	{
		constexpr static std::uint32_t id = WM_PAINT;

		paint_message(wparam_t, lparam_t) 
		{
		}
	};

	struct input_message
	{
		constexpr static std::uint32_t id = WM_INPUT;
		wparam_t code;
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		HANDLE handle;

		input_message(wparam_t code, lparam_t item) : code(code & 0xff), handle(reinterpret_cast<HANDLE>(item))
        {
        }
#else
        HRAWINPUT handle;

		input_message(wparam_t code, lparam_t item) : code(GET_RAWINPUT_CODE_WPARAM(code)), handle(reinterpret_cast<HRAWINPUT>(item))
        {
        }
#endif
	};

	struct input_device_change_message
	{
		constexpr static std::uint32_t id = WM_INPUT_DEVICE_CHANGE;
        wparam_t code;
        HANDLE device_handle;

		input_device_change_message(wparam_t code, lparam_t handle) : code(code), device_handle(reinterpret_cast<HANDLE>(handle))
        {
        }
	};

	struct draw_item_message
	{
		constexpr static std::uint32_t id = WM_DRAWITEM;

		#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
			struct DRAWITEMSTRUCT {
		      UINT      CtlType;
			  UINT      CtlID;
			  UINT      itemID;
			  UINT      itemAction;
			  UINT      itemState;
			  HWND      hwndItem;
			  HDC       hDC;
			  RECT      rcItem;
			  ULONG_PTR itemData;
			};
		#endif

		std::size_t control_id;
		DRAWITEMSTRUCT& item;

		draw_item_message(wparam_t control_id, lparam_t item) : control_id(control_id), item(*reinterpret_cast<DRAWITEMSTRUCT*>(item))
		{
		}
	};

	struct get_object_message
	{
		constexpr static std::uint32_t id = WM_GETOBJECT;
		
		std::uint32_t flags;
		std::uint32_t object_id;


		get_object_message(wparam_t flags, lparam_t objectId) : flags(std::uint32_t(flags)), object_id(std::uint32_t(objectId))
		{

		}
	};

	template<typename TChar = std::byte>
	struct copy_data_message
	{
		constexpr static std::uint32_t id = WM_COPYDATA;
		hwnd_t sender;
		std::size_t data_type;
		std::span<TChar> data;

		#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
			struct COPYDATASTRUCT {
			  ULONG_PTR dwData;
			  DWORD     cbData;
			  PVOID     lpData;
			};
		#endif

		copy_data_message(wparam_t wParam, lparam_t lParam) : 
			sender(hwnd_t(wParam)), data_type(0), data()
		{
			COPYDATASTRUCT* raw_data = std::bit_cast<COPYDATASTRUCT*>(lParam);
			if (raw_data)
			{
				data_type = raw_data->dwData;
				if (raw_data->cbData == 0 || raw_data->lpData == nullptr)
				{
					return;				
				}

				data = std::span<TChar>(std::bit_cast<TChar*>(raw_data->lpData), raw_data->cbData);
			}
		}
	};

	struct pos_changed_message
	{
		constexpr static std::uint32_t id = WM_WINDOWPOSCHANGED;
		
		#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		struct WINDOWPOS {
				  HWND hwnd;
				  HWND hwndInsertAfter;
				  int  x;
				  int  y;
				  int  cx;
				  int  cy;
				  UINT flags;
				};
		#endif
		WINDOWPOS& data;
		pos_changed_message(wparam_t, lparam_t lParam) : data(*std::bit_cast<WINDOWPOS*>(lParam))
		{
		}
	};


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
}

#endif // !WINDOWSHPP
