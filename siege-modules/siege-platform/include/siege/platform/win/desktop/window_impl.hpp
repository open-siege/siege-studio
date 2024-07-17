#ifndef WIN32_CLASS_HPP
#define WIN32_CLASS_HPP

#include <siege/platform/win/desktop/notifications.hpp>
#include "window.hpp"
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

  template<typename TWindow>
  std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
  {
    DO_DISPATCH(create_message, wm_create);
    DO_DISPATCH(init_dialog_message, wm_init_dialog);
    DO_DISPATCH(destroy_message, wm_destroy);
    DO_DISPATCH(get_object_message, wm_get_object);
    DO_DISPATCH(size_message, wm_size);
    DO_DISPATCH(pos_changed_message, wm_pos_changed);
    DO_DISPATCH(paint_message, wm_paint);
    DO_DISPATCH(erase_background_message, wm_erase_background);
    DO_DISPATCH(draw_item_message, wm_draw_item);
    DO_DISPATCH(measure_item_message, wm_measure_item);
    DO_DISPATCH(setting_change_message, wm_setting_change);

    DO_DISPATCH(button_control_color_message, wm_control_color);
    DO_DISPATCH(list_box_control_color_message, wm_control_color);
    DO_DISPATCH(scroll_bar_control_color_message, wm_control_color);
    DO_DISPATCH(edit_control_color_message, wm_control_color);
    DO_DISPATCH(static_control_color_message, wm_control_color);

    DO_DISPATCH(keyboard_key_up_message, wm_keyboard_key_up);
    DO_DISPATCH(keyboard_key_down_message, wm_keyboard_key_down);
    DO_DISPATCH(keyboard_char_message, wm_keyboard_char);
    DO_DISPATCH(input_message, wm_input);
    DO_DISPATCH(input_device_change_message, wm_input_device_change);

    if constexpr (requires(TWindow t) { t.wm_copy_data(copy_data_message<char>{ wParam, lParam }); })
    {
      if (message == copy_data_message<char>::id)
      {
        return self->wm_copy_data(copy_data_message<char>{ wParam, lParam });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_copy_data(copy_data_message<std::uint8_t>{ wParam, lParam }); })
    {
      if (message == copy_data_message<std::uint8_t>::id)
      {
        return self->wm_copy_data(copy_data_message<std::uint8_t>{ wParam, lParam });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_copy_data(copy_data_message<std::byte>{ wParam, lParam }); })
    {
      if (message == copy_data_message<std::byte>::id)
      {
        return self->wm_copy_data(copy_data_message<std::byte>{ wParam, lParam });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_command(menu_command{ wParam, lParam }); })
    {
      if (message == command_message::id && menu_command::is_menu_command(wParam, lParam))
      {
        return self->wm_command(menu_command{ wParam, lParam });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_command(accelerator_command{ wParam, lParam }); })
    {
      if (message == command_message::id && accelerator_command::is_accelerator_command(wParam, lParam))
      {
        return self->wm_command(accelerator_command{ wParam, lParam });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_command(command_message{ wParam, lParam }); })
    {
      if (message == command_message::id)
      {
        return self->wm_command(command_message{ wParam, lParam });
      }
    }

    if constexpr (requires(TWindow t) { t.wm_notify(notify_message{ wParam, lParam }); })
    {
      if (message == command_message::id && !(accelerator_command::is_accelerator_command(wParam, lParam) || menu_command::is_menu_command(wParam, lParam)))
      {
        return self->wm_notify(notify_message{ command_message{ wParam, lParam } });
      }
    }

    if (message == notify_message::id)
    {
      notify_message header(wParam, lParam);
      thread_local std::array<wchar_t, 256> name{};

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

      if constexpr (requires(TWindow t) { t.wm_notify(tree_view_notification{ wParam, lParam }); })
      {
        if (header.code == TVN_ITEMEXPANDINGW || header.code == TVN_ITEMEXPANDEDW || header.code == TVN_SELCHANGINGW || header.code == TVN_SELCHANGEDW || header.code == TVN_SINGLEEXPAND || header.code == TVN_BEGINDRAGW || header.code == TVN_DELETEITEMW)
        {
          return self->wm_notify(tree_view_notification{ wParam, lParam });
        }
      }

      if constexpr (requires(TWindow t) { t.wm_notify(list_view_custom_draw_notification{ wParam, lParam }); })
      {
        if (header.code == NM_CUSTOMDRAW
            && ::RealGetWindowClassW(header.hwndFrom, name.data(), name.size()) > 0 && std::wstring_view(name.data()) == WC_LISTVIEWW)
        {
          return self->wm_notify(list_view_custom_draw_notification{ wParam, lParam });
        }
      }

      if constexpr (requires(TWindow t) { t.wm_notify(tree_view_custom_draw_notification{ wParam, lParam }); })
      {
        if (header.code == NM_CUSTOMDRAW
            && ::RealGetWindowClassW(header.hwndFrom, name.data(), name.size()) > 0 && std::wstring_view(name.data()) == WC_TREEVIEWW)
        {
          return self->wm_notify(tree_view_custom_draw_notification{ wParam, lParam });
        }
      }

      if constexpr (requires(TWindow t) { t.wm_notify(tool_bar_custom_draw_notification{ wParam, lParam }); })
      {
        if (header.code == NM_CUSTOMDRAW
            && ::RealGetWindowClassW(header.hwndFrom, name.data(), name.size()) > 0 && std::wstring_view(name.data()) == TOOLBARCLASSNAMEW)
        {
          return self->wm_notify(tool_bar_custom_draw_notification{ wParam, lParam });
        }
      }

      if constexpr (requires(TWindow t) { t.wm_notify(custom_draw_notification{ wParam, lParam }); })
      {
        if (header.code == NM_CUSTOMDRAW)
        {
          return self->wm_notify(custom_draw_notification{ wParam, lParam });
        }
      }

      if constexpr (requires(TWindow t) { t.wm_notify(header_filter_button_notification{ wParam, lParam }); })
      {
        if (header.code == HDN_FILTERBTNCLICK)
        {
          return self->wm_notify(header_filter_button_notification{ wParam, lParam });
        }
      }

      if constexpr (requires(TWindow t) { t.wm_notify(header_notification{ wParam, lParam }); })
      {
        if (header.code == HDN_FILTERCHANGE || header.code == HDN_BEGINFILTEREDIT || header.code == HDN_ENDFILTEREDIT || header.code == HDN_ITEMCLICK || header.code == HDN_ITEMDBLCLICK || header.code == HDN_ITEMKEYDOWN || header.code == HDN_ITEMSTATEICONCLICK || header.code == HDN_OVERFLOWCLICK || header.code == HDN_DIVIDERDBLCLICK)
        {
          return self->wm_notify(header_notification{ wParam, lParam });
        }
      }

      if constexpr (requires(TWindow t) { t.wm_notify(list_view_item_activation{ wParam, lParam }); })
      {
        if ((header.code == NM_CLICK || header.code == NM_DBLCLK || header.code == NM_RCLICK || header.code == NM_RDBLCLK)
            && ::RealGetWindowClassW(header.hwndFrom, name.data(), name.size()) > 0 && std::wstring_view(name.data()) == WC_LISTVIEWW)
        {
          return self->wm_notify(list_view_item_activation{ wParam, lParam });
        }
      }

      if constexpr (requires(TWindow t) { t.wm_notify(mouse_notification{ wParam, lParam }); })
      {
        if ((header.code == NM_CLICK || header.code == NM_DBLCLK || header.code == NM_RCLICK || header.code == NM_RDBLCLK)
            && ::RealGetWindowClassW(header.hwndFrom, name.data(), name.size()) > 0 && std::wstring_view(name.data()) == TOOLBARCLASSNAMEW)
        {
          return self->wm_notify(mouse_notification{ wParam, lParam });
        }
      }
#endif

      if constexpr (requires(TWindow t) { t.wm_notify(notify_message{ wParam, lParam }); })
      {
        if (message == notify_message::id)
        {
          return self->wm_notify(std::move(header));
        }
      }
    }

    return std::nullopt;
  }


  template<typename TWindow, int StaticSize = 0>
  struct window_meta_class : WNDCLASSEXW
  {
    window_meta_class() : WNDCLASSEXW{}
    {
      constexpr static auto data_size = sizeof(TWindow) / sizeof(LONG_PTR);
      constexpr static auto extra_size = sizeof(TWindow) % sizeof(LONG_PTR) == 0 ? 0 : 1;
      constexpr static auto total_size = (data_size + extra_size) * sizeof(LONG_PTR);
      constexpr static auto can_fit = std::is_trivially_copyable_v<TWindow> && total_size <= 40;

      struct handler
      {
        static lresult_t CALLBACK WindowHandler(hwnd_t hWnd, std::uint32_t message, wparam_t wParam, lparam_t lParam)
        {
          TWindow* self = nullptr;

          auto do_dispatch = [&]() -> lresult_t {
            std::optional<lresult_t> result = std::nullopt;

            if (self)
            {
              result = dispatch_message(self, message, wParam, lParam);

              if (message == WM_NCDESTROY)
              {
                self->~TWindow();

                if constexpr (!can_fit)
                {
                  auto heap = std::bit_cast<HANDLE>(GetWindowLongPtrW(hWnd, 0));
                  // auto size = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
                  auto data = std::bit_cast<TWindow*>(GetWindowLongPtrW(hWnd, 2 * sizeof(LONG_PTR)));

                  ::HeapFree(heap, 0, data);
                }
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
                win32::window_ref(hWnd).ForEachPropertyExW([](auto wnd, std::wstring_view name, HANDLE handle) {
                  ::RemovePropW(wnd, name.data());
                });
#endif
                return 0;
              }
            }

            return result.or_else([&] { return std::make_optional(DefWindowProc(hWnd, message, wParam, lParam)); }).value();
          };

          if constexpr (can_fit)
          {
            std::array<LONG_PTR, data_size + extra_size> raw_data{};

            if (message == non_client_create_message::id)
            {
              auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

              self = new (raw_data.data()) TWindow(hWnd, *pCreate);

              for (auto i = 0; i < raw_data.size(); i++)
              {
                SetWindowLongPtrW(hWnd, i * sizeof(LONG_PTR), raw_data[i]);
              }
            }
            else
            {
              for (auto i = 0; i < raw_data.size(); i++)
              {
                raw_data[i] = GetWindowLongPtrW(hWnd, i * sizeof(LONG_PTR));
              }

              self = std::bit_cast<TWindow*>(raw_data.data());
            }

            return do_dispatch();
          }
          else
          {
            if (message == non_client_create_message::id)
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

            return do_dispatch();
          }
        }
      };

      this->cbSize = sizeof(WNDCLASSEXW);
      this->lpfnWndProc = handler::WindowHandler;
      static auto window_type_name = type_name<TWindow>();
      this->lpszClassName = this->lpszClassName ? this->lpszClassName : window_type_name.c_str();

      static_assert(StaticSize <= 40, "StaticSize is too big for cbClsExtra");
      this->cbClsExtra = StaticSize;

      if constexpr (can_fit)
      {
        static_assert(total_size <= 40, "TWindow is too big for cbWndExtra");
        static_assert(std::is_trivially_copyable_v<TWindow>, "TWindow must be trivially copyable");
        this->cbWndExtra = int(total_size);
      }
      else
      {
        this->cbWndExtra = sizeof(std::size_t) * 3;
      }
    }
  };

  // TODO incorporate this logic into the main dispatch method
  inline std::any make_mouse_message(std::uint32_t message, wparam_t wParam, lparam_t lParam) noexcept
  {
    if (message == mouse_move_message::id || message == mouse_hover_message::id || message == mouse_wheel_message::id || mouse_button_double_click_message::matches_message(message) || mouse_button_down_message::matches_message(message) || mouse_button_up_message::matches_message(message))
    {
      mouse_button_message mouse_data{};
      mouse_data.x = LOWORD(lParam);
      mouse_data.y = HIWORD(lParam);

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
        return mouse_move_message{ std::move(mouse_data) };
      }
      else if (message == mouse_hover_message::id)
      {
        return mouse_hover_message{ std::move(mouse_data) };
      }
      else if (message == mouse_wheel_message::id)
      {
        auto delta = GET_WHEEL_DELTA_WPARAM(wParam);
        return mouse_wheel_message{ std::move(mouse_data), delta };
      }
      else if (mouse_button_down_message::matches_message(message))
      {
        return mouse_button_down_message{ std::move(mouse_data) };
      }
      else if (mouse_button_up_message::matches_message(message))
      {
        return mouse_button_up_message{ std::move(mouse_data) };
      }
      else if (mouse_button_double_click_message::matches_message(message))
      {
        return mouse_button_double_click_message{ std::move(mouse_data) };
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

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
  template<typename TWindow>
  struct static_window_meta_class : ::WNDCLASSEXW
  {
    constexpr static auto data_size = sizeof(TWindow) / sizeof(LONG_PTR);
    constexpr static auto extra_size = sizeof(TWindow) % sizeof(LONG_PTR) == 0 ? 0 : 1;
    constexpr static auto total_size = (data_size + extra_size) * sizeof(LONG_PTR);
    constexpr static auto can_fit = std::is_trivially_copyable_v<TWindow> && total_size <= 40 - sizeof(LONG_PTR);

    static_window_meta_class() : ::WNDCLASSEXW{}
    {
      struct handler
      {
        static lresult_t CALLBACK WindowHandler(hwnd_t hWnd, std::uint32_t message, wparam_t wParam, lparam_t lParam)
        {
          TWindow* self = nullptr;

          auto do_dispatch = [&]() -> lresult_t {
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

                  if constexpr (!can_fit)
                  {
                    auto heap = std::bit_cast<HANDLE>(GetClassLongPtrW(hWnd, sizeof(LONG_PTR)));
                    // auto size = GetWindowLongPtrW(hWnd, sizeof(LONG_PTR));
                    auto data = std::bit_cast<TWindow*>(GetClassLongPtrW(hWnd, 3 * sizeof(LONG_PTR)));

                    ::HeapFree(heap, 0, data);
                  }
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

                  win32::window_ref(hWnd).ForEachPropertyExW([](auto wnd, std::wstring_view name, HANDLE handle) {
                    ::RemovePropW(wnd, name.data());
                  });

#endif
                }

                SetClassLongPtrW(hWnd, 0, ref_count);

                return 0;
              }
            }

            return result.or_else([&] { return std::make_optional(DefWindowProc(hWnd, message, wParam, lParam)); }).value();
          };

          if constexpr (can_fit)
          {
            std::array<LONG_PTR, data_size + extra_size> raw_data{};

            if (message == non_client_create_message::id)
            {
              auto ref_count = GetClassLongPtrW(hWnd, 0);
              auto* pCreate = std::bit_cast<CREATESTRUCTW*>(lParam);

              if (ref_count == 0)
              {
                self = new (raw_data.data()) TWindow(hWnd, *pCreate);

                for (auto i = 0; i < raw_data.size(); i++)
                {
                  SetClassLongPtrW(hWnd, (i + 1) * sizeof(LONG_PTR), raw_data[i]);
                }
              }
              ref_count++;

              SetClassLongPtrW(hWnd, 0, ref_count);
            }
            else
            {
              for (auto i = 0; i < raw_data.size(); i++)
              {
                raw_data[i] = GetClassLongPtrW(hWnd, i * sizeof(LONG_PTR));
              }

              self = std::bit_cast<TWindow*>(raw_data.data());
            }

            return do_dispatch();
          }
          else
          {
            if (message == non_client_create_message::id)
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

            return do_dispatch();
          }
        }
      };

      this->cbSize = sizeof(WNDCLASSEXW);
      this->lpfnWndProc = handler::WindowHandler;
      static auto window_type_name = type_name<TWindow>();
      this->lpszClassName = this->lpszClassName ? this->lpszClassName : window_type_name.c_str();
      this->cbWndExtra = 0;

      if constexpr (can_fit)
      {
        static_assert(total_size <= 40 - sizeof(LONG_PTR), "TWindow is too big for cbClsExtra");
        static_assert(std::is_trivially_copyable_v<TWindow>, "TWindow must be trivially copyable");
        this->cbClsExtra = int(total_size + sizeof(LONG_PTR));
      }
      else
      {
        this->cbClsExtra = sizeof(std::size_t) * 4;
      }
    }
  };
#endif
}// namespace win32

#endif