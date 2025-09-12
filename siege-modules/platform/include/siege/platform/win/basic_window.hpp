#ifndef BASIC_WINDOW_HPP
#define BASIC_WINDOW_HPP
#include <siege/platform/win/window.hpp>

namespace win32
{
  struct default_window : win32::window_ref
  {
    default_window(HWND handle, CREATESTRUCTW& params) : win32::window_ref(handle)
    {
    }

    virtual std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam)
    {
      return std::nullopt;
    }

    virtual ~default_window() = default;
  };

  template<typename TWindow>
  struct basic_window : default_window
  {
    using default_window::default_window;

    static LRESULT __stdcall window_proc(HWND self, UINT message, WPARAM wparam, LPARAM lparam)
    {
      if (message == WM_NCCREATE)
      {
        try
        {
          default_window* temp = new TWindow(self, *(CREATESTRUCTW*)lparam);
          ::SetWindowLongPtrW(self, 0, (LONG_PTR)temp);

          return ::DefWindowProcW(self, message, wparam, lparam);
        }
        catch (...)
        {
          return FALSE;
        }
      }

      auto* window = (default_window*)::GetWindowLongPtrW(self, 0);

      if (!window)
      {
        return ::DefWindowProcW(self, message, wparam, lparam);
      }

      if (message == WM_NCDESTROY && *window == self)
      {
        auto result = ::DefWindowProcW(self, message, wparam, lparam);
        delete window;
        ::SetWindowLongPtrW(self, 0, 0);
        return result;
      }

      if (*window == self)
      {
        auto result = window->window_proc(message, wparam, lparam);

        if (result)
        {
          return *result;
        }
      }

      return ::DefWindowProcW(self, message, wparam, lparam);
    }
  };

}// namespace siege::views

#endif// !BASIC_WINDOW_HPP
