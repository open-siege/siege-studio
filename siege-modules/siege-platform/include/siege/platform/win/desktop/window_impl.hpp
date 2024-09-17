#ifndef WIN32_CLASS_HPP
#define WIN32_CLASS_HPP

#include <siege/platform/win/desktop/user_controls.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include "window.hpp"
#include <windowsx.h>
#include <any>

namespace win32
{
#define DO_DISPATCH(message_name, event_name)                                          \
  if constexpr (requires(TWindow t) { t.event_name(message_name{ wParam, lParam }); }) \
  {                                                                                    \
    if (message == message_name::id)                                                   \
    {                                                                                  \
      return self->event_name(message_name{ wParam, lParam });                         \
    }                                                                                  \
  }

#define DO_DISPATCH_NOPARAM(message_id, event_name)      \
  if constexpr (requires(TWindow t) { t.event_name(); }) \
  {                                                      \
    if (message == message_id)                           \
    {                                                    \
      return self->event_name();                         \
    }                                                    \
  }

#define DO_DISPATCH_LPARAM(message_id, lparam_type, event_name)       \
  if constexpr (requires(TWindow t) { t.event_name(lparam_type{}); }) \
  {                                                                   \
    if (message == message_id)                                        \
    {                                                                 \
      return self->event_name(*(lparam_type*)lParam);                 \
    }                                                                 \
  }

#define DO_DISPATCH_WPARAM_AND_LPARAM(message_id, wparam_type, lparam_type, event_name)       \
  if constexpr (requires(TWindow t, lparam_type temp) { t.event_name(wparam_type{}, temp); }) \
  {                                                                                           \
    if (message == message_id)                                                                \
    {                                                                                         \
      return self->event_name((wparam_type)wParam, *(lparam_type)lParam);                    \
    }                                                                                         \
  }


  template<typename TWindow>
  std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
  {
    DO_DISPATCH_NOPARAM(WM_CREATE, wm_create);
    DO_DISPATCH_NOPARAM(WM_CLOSE, wm_close);
    DO_DISPATCH_LPARAM(WM_CREATE, CREATESTRUCTW, wm_create);
    DO_DISPATCH_NOPARAM(WM_DESTROY, wm_destroy);
    DO_DISPATCH(get_object_message, wm_get_object);
    DO_DISPATCH_WPARAM_AND_LPARAM(WM_TIMER, std::size_t, TIMERPROC, wm_timer);
    DO_DISPATCH_WPARAM_AND_LPARAM(WM_MEASUREITEM, std::size_t, MEASUREITEMSTRUCT*, wm_measure_item);
    DO_DISPATCH_WPARAM_AND_LPARAM(WM_DRAWITEM, std::size_t, DRAWITEMSTRUCT*, wm_draw_item);

    DO_DISPATCH(setting_change_message, wm_setting_change);

    DO_DISPATCH(input_message, wm_input);
    DO_DISPATCH(input_device_change_message, wm_input_device_change);

    if constexpr (requires(TWindow t) { t.wm_size(std::size_t{}, SIZE{}); })
    {
      if (message == WM_SIZE)
      {
        return self->wm_size(std::size_t(wParam), SIZE{ .cx = SHORT(LOWORD(lParam)), .cy = SHORT(HIWORD(lParam)) });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_set_focus(); })
    {
      if (message == WM_SETFOCUS)
      {
        return self->wm_set_focus();
      }
    }

    if constexpr (requires(TWindow t) { t.wm_size(std::size_t{}, SIZE{}); })
    {
      if (message == WM_SIZE)
      {
        return self->wm_size(std::size_t(wParam), SIZE{ .cx = SHORT(LOWORD(lParam)), .cy = SHORT(HIWORD(lParam)) });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_erase_background(win32::gdi::drawing_context_ref{}); })
    {
      if (message == WM_ERASEBKGND)
      {
        return self->wm_erase_background(win32::gdi::drawing_context_ref((HDC)wParam));
      }
    }

    if constexpr (requires(TWindow t) { t.wm_mouse_move(std::size_t{}, POINTS{}); })
    {
      if (message == WM_MOUSEMOVE)
      {
        return self->wm_mouse_move(std::size_t(wParam), POINTS{ .x = SHORT(GET_X_LPARAM(lParam)), .y = SHORT(GET_Y_LPARAM(lParam)) });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_mouse_leave(); })
    {
      if (message == WM_MOUSELEAVE)
      {
        return self->wm_mouse_leave();
      }
    }

    if constexpr (requires(TWindow t) { t.wm_mouse_button_down(std::size_t{}, POINTS{}); })
    {
      if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_XBUTTONDOWN)
      {
        return self->wm_mouse_button_down(std::size_t(wParam), POINTS{ .x = SHORT(GET_X_LPARAM(lParam)), .y = SHORT(GET_Y_LPARAM(lParam)) });
      }
    }

    auto create_input_struct = [](auto wParam, auto lParam) -> KEYBDINPUT {
      KEYBDINPUT input{};
      input.wVk = wParam;

      WORD keyFlags = HIWORD(lParam);
      input.wScan = LOBYTE(keyFlags);

      if (input.wScan)
      {
        input.dwFlags |= KEYEVENTF_SCANCODE;
      }

      if ((keyFlags & KF_EXTENDED) == KF_EXTENDED)
      {
        input.dwFlags |= KEYEVENTF_EXTENDEDKEY;
      }

      input.time = ::GetMessageTime();
      input.dwExtraInfo = lParam;

      return input;
    };

    if constexpr (requires(TWindow t) { t.wm_sys_key_down(::KEYBDINPUT{}); })
    {
      if (message == WM_SYSKEYDOWN)
      {
        return self->wm_sys_key_down(create_input_struct(wParam, lParam));
      }
    }

    if constexpr (requires(TWindow t) { t.wm_key_down(::KEYBDINPUT{}); })
    {
      if (message == WM_KEYDOWN)
      {
        return self->wm_key_down(create_input_struct(wParam, lParam));
      }
    }

    if constexpr (requires(TWindow t) { t.wm_copy_data(copy_data_message<char>{ wParam, lParam }); })
    {
      if (message == copy_data_message<char>::id)
      {
        return self->wm_copy_data(copy_data_message<char>{ wParam, lParam });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_copy_data(copy_data_message<std::byte>{ wParam, lParam }); })
    {
      if (message == copy_data_message<std::byte>::id)
      {
        return self->wm_copy_data(copy_data_message<std::byte>{ wParam, lParam });
      }
    }

    return win32::button::notifications::dispatch_message(self, message, wParam, lParam)
      .or_else([&] { return win32::menu::notifications::dispatch_message(self, *self, message, wParam, lParam); })
      .or_else([&] { return win32::edit::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::list_box::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::static_control::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::scroll_bar::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::combo_box::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::sys_link::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::track_bar::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::list_view::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::header::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::rebar::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::tab_control::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::tool_bar::notifications::dispatch_message(self, message, wParam, lParam); })
      .or_else([&] { return win32::tree_view::notifications::dispatch_message(self, message, wParam, lParam); });
  }


  template<typename TWindow, int StaticSize = 0>
  struct window_meta_class : WNDCLASSEXW
  {
    window_meta_class() : WNDCLASSEXW{}
    {
      constexpr static auto data_size = sizeof(TWindow) / sizeof(LONG_PTR);
      constexpr static auto extra_size = sizeof(TWindow) % sizeof(LONG_PTR) == 0 ? 0 : 1;
      constexpr static auto total_size = (data_size + extra_size) * sizeof(LONG_PTR);

      struct handler
      {
        static lresult_t CALLBACK WindowHandler(hwnd_t hWnd, std::uint32_t message, wparam_t wParam, lparam_t lParam)
        {
          TWindow* self = nullptr;

          if (message == WM_NCCREATE)
          {
            auto heap = ::GetProcessHeap();
            auto size = sizeof(TWindow);
            auto raw_data = ::HeapAlloc(heap, 0, size);

            auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

            self = new (raw_data) TWindow(hWnd, *pCreate);

            SetWindowLongPtrW(hWnd, 0, std::bit_cast<LONG_PTR>(heap));
            SetWindowLongPtrW(hWnd, sizeof(LONG_PTR), size);
            SetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR), std::bit_cast<LONG_PTR>(raw_data));
          }
          else
          {
            // auto heap = GetWindowLongPtrW(hWnd, 0);
            // auto size = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
            self = std::bit_cast<TWindow*>(GetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR)));
          }

          std::optional<lresult_t> result = std::nullopt;

          if (self)
          {
            result = dispatch_message(self, message, wParam, lParam);

            if (message == WM_NCDESTROY)
            {
              self->~TWindow();
              auto heap = std::bit_cast<HANDLE>(GetWindowLongPtrW(hWnd, 0));
              // auto size = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
              auto data = std::bit_cast<TWindow*>(GetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR)));

              ::HeapFree(heap, 0, data);
              win32::window_ref(hWnd).ForEachPropertyExW([](auto wnd, std::wstring_view name, HANDLE handle) {
                ::RemovePropW(wnd, name.data());
              });

              return 0;
            }
          }

          return result.or_else([&] { return std::make_optional(DefWindowProc(hWnd, message, wParam, lParam)); }).value();
        }
      };


      this->cbSize = sizeof(WNDCLASSEXW);
      this->lpfnWndProc = handler::WindowHandler;
      static auto window_type_name = type_name<TWindow>();
      this->lpszClassName = this->lpszClassName ? this->lpszClassName : window_type_name.c_str();

      this->style = CS_VREDRAW | CS_HREDRAW;

      static_assert(StaticSize <= 40, "StaticSize is too big for cbClsExtra");
      this->cbClsExtra = StaticSize;
      this->cbWndExtra = sizeof(std::size_t) * 3;
    }
  };

  template<typename TWindow>
  struct static_window_meta_class : ::WNDCLASSEXW
  {
    constexpr static auto data_size = sizeof(TWindow) / sizeof(LONG_PTR);
    constexpr static auto extra_size = sizeof(TWindow) % sizeof(LONG_PTR) == 0 ? 0 : 1;
    constexpr static auto total_size = (data_size + extra_size) * sizeof(LONG_PTR);

    static_window_meta_class() : ::WNDCLASSEXW{}
    {
      struct handler
      {
        static lresult_t CALLBACK WindowHandler(hwnd_t hWnd, std::uint32_t message, wparam_t wParam, lparam_t lParam)
        {
          TWindow* self = nullptr;
          if (message == WM_NCCREATE)
          {
            auto ref_count = GetClassLongPtrW(hWnd, 0);

            if (ref_count == 0)
            {
              auto heap = ::GetProcessHeap();
              auto size = sizeof(TWindow);
              auto raw_data = ::HeapAlloc(heap, 0, size);

              auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

              self = new (raw_data) TWindow(hWnd, *pCreate);

              SetClassLongPtrW(hWnd, sizeof(LONG_PTR), std::bit_cast<LONG_PTR>(heap));
              SetClassLongPtrW(hWnd, 2 * sizeof(LONG_PTR), size);
              SetClassLongPtrW(hWnd, 3 * sizeof(LONG_PTR), std::bit_cast<LONG_PTR>(raw_data));
            }

            ref_count++;
            SetClassLongPtrW(hWnd, 0, ref_count);
          }
          else
          {
            // auto heap = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR);
            // auto size = GetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR));
            self = std::bit_cast<TWindow*>(GetClassLongPtrW(hWnd, 3 * sizeof(LONG_PTR)));
          }

          std::optional<lresult_t> result = std::nullopt;

          if (self)
          {
            result = dispatch_message(self, message, wParam, lParam);

            if (message == WM_NCDESTROY)
            {
              auto ref_count = GetClassLongPtrW(hWnd, 0);
              ref_count--;

              if (ref_count == 0)
              {
                self->~TWindow();
                auto heap = std::bit_cast<HANDLE>(GetClassLongPtrW(hWnd, sizeof(LONG_PTR)));
                // auto size = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
                auto data = std::bit_cast<TWindow*>(GetClassLongPtrW(hWnd, 3 * sizeof(LONG_PTR)));

                ::HeapFree(heap, 0, data);

                win32::window_ref(hWnd).ForEachPropertyExW([](auto wnd, std::wstring_view name, HANDLE handle) {
                  ::RemovePropW(wnd, name.data());
                });
              }

              SetClassLongPtrW(hWnd, 0, ref_count);

              return 0;
            }
          }

          return result.or_else([&] { return std::make_optional(DefWindowProc(hWnd, message, wParam, lParam)); }).value();
        }
      };

      this->cbSize = sizeof(WNDCLASSEXW);
      this->lpfnWndProc = handler::WindowHandler;
      static auto window_type_name = type_name<TWindow>();
      this->lpszClassName = this->lpszClassName ? this->lpszClassName : window_type_name.c_str();
      this->cbWndExtra = 0;
      this->cbClsExtra = sizeof(std::size_t) * 4;
    }
  };
}// namespace win32

#endif