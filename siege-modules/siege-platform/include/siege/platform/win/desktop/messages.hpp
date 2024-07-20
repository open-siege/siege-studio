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

  struct size_message
  {
    constexpr static std::uint32_t id = WM_SIZE;

    int type;
    SIZE client_size;

    size_message(wparam_t wParam, lparam_t lParam) : type(wParam),
                                                     client_size(SIZE{ LOWORD(lParam), HIWORD(lParam) })
    {
    }
  };

  struct setting_change_message
  {
    constexpr static std::uint32_t id = WM_SETTINGCHANGE;

    wparam_t wparam;

    std::wstring setting;

    setting_change_message(wparam_t wParam, lparam_t lParam) : wparam(wParam),
                                                               setting(lParam != 0 ? (wchar_t*)lParam : L"")
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

  struct erase_background_message
  {
    constexpr static std::uint32_t id = WM_ERASEBKGND;

    HDC context;

    erase_background_message(wparam_t wParam, lparam_t) : context((HDC)wParam)
    {

    }
  };

  struct button_control_color_message
  {
    constexpr static std::uint32_t id = WM_CTLCOLORBTN;

    HDC context;
    HWND control;

    button_control_color_message(wparam_t wParam, lparam_t lParam) : context((HDC)wParam), control((HWND)lParam)
    {
    }
  };

  struct edit_control_color_message
  {
    constexpr static std::uint32_t id = WM_CTLCOLOREDIT;

    HDC context;
    HWND control;

    edit_control_color_message(wparam_t wParam, lparam_t lParam) : context((HDC)wParam), control((HWND)lParam)
    {
    }
  };

  struct static_control_color_message
  {
    constexpr static std::uint32_t id = WM_CTLCOLORSTATIC;

    HDC context;
    HWND control;

    static_control_color_message(wparam_t wParam, lparam_t lParam) : context((HDC)wParam), control((HWND)lParam)
    {
    }
  };

  struct list_box_control_color_message
  {
    constexpr static std::uint32_t id = WM_CTLCOLORLISTBOX;

    HDC context;
    HWND control;

    list_box_control_color_message(wparam_t wParam, lparam_t lParam) : context((HDC)wParam), control((HWND)lParam)
    {
    }
  };

  struct scroll_bar_control_color_message
  {
    constexpr static std::uint32_t id = WM_CTLCOLORSCROLLBAR;

    HDC context;
    HWND control;

    scroll_bar_control_color_message(wparam_t wParam, lparam_t lParam) : context((HDC)wParam), control((HWND)lParam)
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

  #if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  struct DRAWITEMSTRUCT
  {
    UINT CtlType;
    UINT CtlID;
    UINT itemID;
    UINT itemAction;
    UINT itemState;
    HWND hwndItem;
    HDC hDC;
    RECT rcItem;
    ULONG_PTR itemData;
  };
#endif

  
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  struct MEASUREITEMSTRUCT
  {
    UINT CtlType;
    UINT CtlID;
    UINT itemID;
    UINT itemWidth;
    UINT itemHeight;
    ULONG_PTR itemData;
  };
#endif

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
    struct COPYDATASTRUCT
    {
      ULONG_PTR dwData;
      DWORD cbData;
      PVOID lpData;
    };
#endif

    copy_data_message(wparam_t wParam, lparam_t lParam) : sender(hwnd_t(wParam)), data_type(0), data()
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
    struct WINDOWPOS
    {
      HWND hwnd;
      HWND hwndInsertAfter;
      int x;
      int y;
      int cx;
      int cy;
      UINT flags;
    };
#endif
    WINDOWPOS& data;
    pos_changed_message(wparam_t, lparam_t lParam) : data(*std::bit_cast<WINDOWPOS*>(lParam))
    {
    }
  };
}// namespace win32

#endif// !WINDOWSHPP
