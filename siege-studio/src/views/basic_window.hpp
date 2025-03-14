#ifndef BASIC_WINDOW_HPP
#define BASIC_WINDOW_HPP
#include <siege/platform/win/window.hpp>

namespace siege::views
{
  struct default_window : win32::window_ref
  {
    default_window(HWND handle, CREATESTRUCTW& params) : win32::window_ref(handle)
    {
    }

    virtual LRESULT default_proc(UINT message, WPARAM wparam, LPARAM lparam)
    {
      return DefWindowProcW(*this, message, wparam, lparam);
    }

    virtual std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam)
    {
      return std::nullopt;
    }

    virtual ~default_window() = default;
  };

  struct default_dialog : default_window
  {
    using default_window::default_window;

    LRESULT default_proc(UINT message, WPARAM wparam, LPARAM lparam) override
    {
      return DefDlgProcW(*this, message, wparam, lparam);
    }
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

          return temp->default_proc(message, wparam, lparam);
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
        auto result = window->default_proc(message, wparam, lparam);
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

      return window->default_proc(message, wparam, lparam);
    }
  };

}// namespace siege::views

#endif// !BASIC_WINDOW_HPP
