#ifndef WIN32_SINGULAR_CONTROLS_HPP
#define WIN32_SINGULAR_CONTROLS_HPP

#include <expected>
#include <functional>
#include <siege/platform/win/desktop/window.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <CommCtrl.h>

namespace win32
{
  template<typename TControl, typename TNotification, typename TReturn = void>
  [[maybe_unused]] std::function<void()> bind_notification(win32::window_ref target, win32::window_ref source, UINT code, std::move_only_function<TReturn(TControl, const TNotification&)> callback)
  {
    struct function_context
    {
      HWND source;
      UINT code;
      std::move_only_function<TReturn(TControl, const TNotification&)> callback;
    };

    struct dispatcher
    {
      static LRESULT __stdcall handle_message(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        UINT_PTR uIdSubclass,
        DWORD_PTR dwRefData)
      {
        if (uMsg == WM_COMMAND && lParam && uIdSubclass)
        {
          auto& context = *(function_context*)uIdSubclass;
          auto id = LOWORD(wParam);
          auto code = HIWORD(wParam);
          auto child = (HWND)lParam;

          if (context.source == child && context.code == code)
          {
            NMHDR info{ .hwndFrom = child, .idFrom = id, .code = code };
            TNotification destInfo{};
            std::memcpy(&destInfo, &info, sizeof(info));

            if constexpr (std::is_void_v<TReturn>)
            {
              context.callback(TControl(child), destInfo);
            }
            else
            {
              return context.callback(TControl(child), destInfo);
            }
          }
        }

        if (uMsg == WM_NOTIFY && lParam && uIdSubclass)
        {
          auto* header = (NMHDR*)lParam;
          auto& context = *(function_context*)uIdSubclass;

          if (header->hwndFrom == context.source && header->code == context.code)
          {
            if constexpr (std::is_void_v<TReturn>)
            {
              context.callback(TControl(header->hwndFrom), *(TNotification*)lParam);
            }
            else
            {
              return context.callback(TControl(header->hwndFrom), *(TNotification*)lParam);
            }
          }
        }

        if (uMsg == WM_NCDESTROY)
        {
          auto* context = (function_context*)uIdSubclass;
          delete context;
          ::RemoveWindowSubclass(hWnd, dispatcher::handle_message, uIdSubclass);
        }

        return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
      }
    };

    auto* context = new function_context{ source.get(), code, std::move(callback) };

    ::SetWindowSubclass(target.get(), dispatcher::handle_message, (UINT_PTR)context, 0);

    return [context, target = target.get()] {
      ::RemoveWindowSubclass(target, dispatcher::handle_message, (UINT_PTR)context);
      delete context;
    };
  }

  struct button : window
  {
    using window::window;
    constexpr static auto class_name = WC_BUTTONW;
    constexpr static std::uint16_t dialog_id = 0x0080;

    struct notifications
    {
      virtual std::optional<win32::lresult_t> wm_command(win32::button, int)
      {
        return std::nullopt;
      }

      virtual std::optional<win32::lresult_t> wm_notify(win32::button, NMCUSTOMDRAW&)
      {
        return std::nullopt;
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(win32::button, DRAWITEMSTRUCT&)
      {
        return std::nullopt;
      }

      virtual std::optional<HBRUSH> wm_control_color(win32::button, win32::gdi::drawing_context_ref)
      {
        return std::nullopt;
      }

      template<typename TWindow>
      static std::optional<lresult_t> dispatch_message(TWindow* window, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
          auto* self = static_cast<notifications*>(window);

          if (message == WM_NOTIFY)
          {
            auto& header = *(NMHDR*)lParam;

            if (header.code == NM_CUSTOMDRAW
                && win32::window_ref(header.hwndFrom).RealGetWindowClassW() == button::class_name)
            {
              return self->wm_notify(button(header.hwndFrom), *(NMCUSTOMDRAW*)lParam);
            }
          }

          if (message == WM_DRAWITEM)
          {
            auto& context = *(DRAWITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_BUTTON)
            {
              return self->wm_draw_item(win32::button(context.hwndItem), context);
            }
          }

          if (message == WM_COMMAND && win32::window_ref((HWND)lParam).RealGetWindowClassW() == button::class_name)
          {
            return self->wm_command(win32::button((HWND)lParam), HIWORD(wParam));
          }
        }

        return std::nullopt;
      }
    };
  };

  struct combo_box : window
  {
    using window::window;
    constexpr static auto class_name = WC_COMBOBOXW;
    constexpr static std::uint16_t dialog_id = 0x0085;

    struct notifications
    {
      virtual std::optional<win32::lresult_t> wm_command(win32::combo_box, int)
      {
        return std::nullopt;
      }

      virtual std::optional<HBRUSH> wm_control_color(win32::combo_box, win32::gdi::drawing_context_ref)
      {
        return std::nullopt;
      }

      virtual SIZE wm_measure_item(win32::combo_box, const MEASUREITEMSTRUCT&)
      {
        return {};
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(win32::combo_box, MEASUREITEMSTRUCT&)
      {
        return std::nullopt;
      }

      template<typename TWindow>
      static std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
          if (message == WM_DRAWITEM)
          {
            auto& context = *(DRAWITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_COMBOBOX)
            {
              return self->wm_draw_item(combo_box(context.hwndItem), context);
            }
          }

          if (message == WM_MEASUREITEM)
          {
            auto& context = *(MEASUREITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_COMBOBOX)
            {
              auto control = ::GetDlgItem(*self, wParam);

              if (!control)
              {
                control = ::FindWindowExW(*self, nullptr, combo_box::class_name, nullptr);
              }

              auto size = self->wm_measure_item(combo_box(control), context);
              context.itemWidth = size.cx;
              context.itemHeight = size.cy;
              return 0;
            }
          }
        }

        return std::nullopt;
      }
    };
  };

  struct edit : window
  {
    using window::window;
    constexpr static auto class_name = WC_EDITW;
    constexpr static std::uint16_t dialog_id = 0x0081;

    struct notifications
    {
      virtual std::optional<win32::lresult_t> wm_command(win32::edit, int)
      {
        return std::nullopt;
      }

      virtual std::optional<HBRUSH> wm_control_color(win32::edit, win32::gdi::drawing_context_ref)
      {
        return std::nullopt;
      }

      template<typename TWindow>
      static std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
          std::optional<window_ref> parent;
          if (message == WM_CTLCOLORSTATIC || message == WM_CTLCOLOREDIT)
          {
            parent = win32::window_ref((HWND)lParam).GetParent();
          }

          if (message == WM_CTLCOLORSTATIC && win32::window_ref((HWND)lParam).RealGetWindowClassW() == edit::class_name && parent && parent->RealGetWindowClassW() != combo_box::class_name)
          {
            auto result = self->wm_control_color(edit((hwnd_t)lParam), win32::gdi::drawing_context_ref((HDC)wParam));

            if (result)
            {
              return (LRESULT)*result;
            }
          }

          if (message == WM_CTLCOLOREDIT && win32::window_ref((HWND)lParam).RealGetWindowClassW() == edit::class_name && parent && parent->RealGetWindowClassW() != combo_box::class_name)
          {
            auto result = self->wm_control_color(edit((hwnd_t)lParam), win32::gdi::drawing_context_ref((HDC)wParam));

            if (result)
            {
              return (LRESULT)*result;
            }
          }
        }

        return std::nullopt;
      }
    };

    [[maybe_unused]] inline std::function<void()> bind_en_change(std::move_only_function<void(edit, const NMHDR&)> callback)
    {
      return bind_notification<edit, NMHDR>(this->GetParent()->ref(), this->ref(), EN_CHANGE, std::move(callback));
    }

    [[maybe_unused]] inline std::function<void()> bind_en_kill_focus(std::move_only_function<void(edit, const NMHDR&)> callback)
    {
      return bind_notification<edit, NMHDR>(this->GetParent()->ref(), this->ref(), EN_KILLFOCUS, std::move(callback));
    }
  };

  struct static_control : window
  {
    using window::window;
    constexpr static auto class_name = WC_STATICW;
    constexpr static std::uint16_t dialog_id = 0x0082;

    struct notifications
    {
      virtual std::optional<win32::lresult_t> wm_command(win32::static_control, int)
      {
        return std::nullopt;
      }

      virtual std::optional<HBRUSH> wm_control_color(win32::static_control, win32::gdi::drawing_context_ref)
      {
        return std::nullopt;
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(win32::static_control, DRAWITEMSTRUCT&)
      {
        return std::nullopt;
      }

      template<typename TWindow>
      static std::optional<lresult_t> dispatch_message(TWindow* window, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
          auto* self = static_cast<notifications*>(window);

          if (message == WM_DRAWITEM)
          {
            auto& context = *(DRAWITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_STATIC)
            {
              return self->wm_draw_item(win32::static_control(context.hwndItem), context);
            }
          }

          if (message == WM_CTLCOLORSTATIC && win32::window_ref((HWND)lParam).RealGetWindowClassW() == static_control::class_name)
          {
            auto result = self->wm_control_color(static_control((HWND)lParam), win32::gdi::drawing_context_ref((HDC)wParam));

            if (result)
            {
              return (LRESULT)*result;
            }
          }
        }

        return std::nullopt;
      }
    };

    HBITMAP SetImage(HBITMAP image)
    {
      return HBITMAP(SendMessageW(*this, STM_SETIMAGE, IMAGE_BITMAP, lparam_t(image)));
    }
  };

  struct list_box : window
  {
    using window::window;
    constexpr static auto class_name = WC_LISTBOXW;
    constexpr static std::uint16_t dialog_id = 0x0083;

    struct notifications
    {
      virtual std::optional<win32::lresult_t> wm_command(win32::list_box, int)
      {
        return std::nullopt;
      }

      virtual std::optional<HBRUSH> wm_control_color(win32::list_box, win32::gdi::drawing_context_ref)
      {
        return std::nullopt;
      }

      virtual SIZE wm_measure_item(win32::list_box, const MEASUREITEMSTRUCT&)
      {
        return {};
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(win32::list_box, DRAWITEMSTRUCT&)
      {
        return std::nullopt;
      }

      template<typename TWindow>
      static std::optional<lresult_t> dispatch_message(TWindow* window, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
          auto* self = static_cast<notifications*>(window);

          if (message == WM_CTLCOLORLISTBOX)
          {
            auto result = self->wm_control_color(list_box((HWND)lParam), win32::gdi::drawing_context_ref((HDC)wParam));

            if (result)
            {
              return (LRESULT)*result;
            }
          }

          if (message == WM_DRAWITEM)
          {
            auto& context = *(DRAWITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_LISTBOX)
            {
              return self->wm_draw_item(list_box(context.hwndItem), context);
            }
          }

          if (message == WM_MEASUREITEM)
          {
            auto& context = *(MEASUREITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_LISTBOX)
            {
              auto control = ::GetDlgItem(*window, wParam);

              if (!control)
              {
                control = ::FindWindowExW(*window, nullptr, list_box::class_name, nullptr);
              }

              auto size = self->wm_measure_item(list_box(control), context);
              context.itemWidth = size.cx;
              context.itemHeight = size.cy;
              return 0;
            }
          }

          if (message == WM_COMMAND && win32::window_ref((HWND)lParam).RealGetWindowClassW() == list_box::class_name)
          {
            return self->wm_command(list_box((HWND)lParam), HIWORD(wParam));
          }
        }

        return std::nullopt;
      }
    };

    [[nodiscard]] inline DWORD GetCount()
    {
      return ListBox_GetCount(*this);
    }

    [[nodiscard]] inline lresult_t GetCurrentSelection()
    {
      return SendMessageW(*this, LB_GETCURSEL, 0, 0);
    }

    [[nodiscard]] inline lresult_t GetTextLength(wparam_t index)
    {
      return SendMessageW(*this, LB_GETTEXTLEN, index, 0);
    }

    [[nodiscard]] inline lresult_t GetItemHeight(wparam_t index)
    {
      return SendMessageW(*this, LB_GETITEMHEIGHT, index, 0);
    }

    [[maybe_unused]] inline lresult_t GetText(wparam_t index, wchar_t* data)
    {
      return SendMessageW(*this, LB_GETTEXT, index, (LPARAM)data);
    }

    [[maybe_unused]] inline lresult_t SetCurrentSelection(wparam_t index)
    {
      return SendMessageW(*this, LB_SETCURSEL, index, 0);
    }

    [[maybe_unused]] inline wparam_t AddString(std::wstring_view text)
    {
      return SendMessageW(*this, LB_ADDSTRING, 0, std::bit_cast<LPARAM>(text.data()));
    }

    [[maybe_unused]] inline wparam_t InsertString(wparam_t index, std::wstring_view text)
    {
      return SendMessageW(*this, LB_INSERTSTRING, index, std::bit_cast<LPARAM>(text.data()));
    }

    [[maybe_unused]] inline std::function<void()> bind_lbn_sel_change(std::move_only_function<void(list_box, const NMHDR&)> callback)
    {
      return bind_notification<list_box, NMHDR>(this->GetParent()->ref(), this->ref(), LBN_SELCHANGE, std::move(callback));
    }
  };

  struct scroll_bar : window
  {
    using window::window;
    constexpr static auto class_name = WC_SCROLLBARW;
    constexpr static std::uint16_t dialog_id = 0x0084;

    struct notifications
    {
      virtual std::optional<HBRUSH> wm_control_color(win32::scroll_bar, win32::gdi::drawing_context_ref)
      {
        return std::nullopt;
      }

      template<typename TWindow>
      static std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
        }

        return std::nullopt;
      }
    };
  };

}// namespace win32


#endif