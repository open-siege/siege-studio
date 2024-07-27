#ifndef WIN32_SINGULAR_CONTROLS_HPP
#define WIN32_SINGULAR_CONTROLS_HPP

#include <expected>
#include <siege/platform/win/desktop/window.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <CommCtrl.h>

namespace win32
{
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

      virtual win32::lresult_t wm_notify(win32::button, NMCUSTOMDRAW&)
      {
        return CDRF_DODEFAULT;
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(win32::button, DRAWITEMSTRUCT&)
      {
        return std::nullopt;
      }

      virtual std::optional<HBRUSH> wm_control_color(win32::button, win32::gdi_drawing_context_ref)
      {
        return std::nullopt;
      }

      template<typename TWindow>
      static std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
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

      virtual std::optional<HBRUSH> wm_control_color(win32::edit, win32::gdi_drawing_context_ref)
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

      virtual std::optional<HBRUSH> wm_control_color(win32::static_control, win32::gdi_drawing_context_ref)
      {
          return std::nullopt;
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(win32::static_control, DRAWITEMSTRUCT&)
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

            if (context.CtlType == ODT_STATIC)
            {
              return self->wm_draw_item(win32::static_control(context.hwndItem), context);
            }
          }

          if (message == WM_CTLCOLORSTATIC && win32::window_ref((HWND)lParam).RealGetWindowClassW() == static_control::class_name)
          {
            return (LRESULT)self->wm_control_color(static_control(lParam), win32::gdi_drawing_context_ref((HDC)wParam));
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

      virtual std::optional<HBRUSH> wm_control_color(win32::list_box, win32::gdi_drawing_context_ref)
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
      static std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
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
              auto control = ::GetDlgItem(*self, wParam);

              if (!control)
              {
                control = ::FindWindowExW(*self, nullptr, list_box::class_name, nullptr);
              }

              auto size = self->wm_measure_item(list_box(control), context);
              context.itemWidth = size.cx;
              context.itemHeight = size.cy;
              return 0;
            }
          }

          if (message == WM_COMMAND && 
              !(HIWORD(wParam) == 0 && HIWORD(wParam) == 1) &&
              win32::window_ref((HWND)lParam).RealGetWindowClassW() == list_box::class_name)
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
  };

  struct scroll_bar : window
  {
    using window::window;
    constexpr static auto class_name = WC_SCROLLBARW;
    constexpr static std::uint16_t dialog_id = 0x0084;

    struct notifications
    {
      virtual std::optional<HBRUSH> wm_control_color(win32::scroll_bar, win32::gdi_drawing_context_ref)
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

      virtual std::optional<HBRUSH> wm_control_color(win32::combo_box, win32::gdi_drawing_context_ref)
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
}// namespace win32


#endif