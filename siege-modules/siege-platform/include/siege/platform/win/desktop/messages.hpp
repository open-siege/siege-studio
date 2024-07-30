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

  struct input_message
  {
    constexpr static std::uint32_t id = WM_INPUT;
    wparam_t code;
    HRAWINPUT handle;

    input_message(wparam_t code, lparam_t item) : code(GET_RAWINPUT_CODE_WPARAM(code)), handle(reinterpret_cast<HRAWINPUT>(item))
    {
    }
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
}// namespace win32

#endif// !WINDOWSHPP
