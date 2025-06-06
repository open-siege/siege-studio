#ifndef WIN32_SINGULAR_CONTROLS_HPP
#define WIN32_SINGULAR_CONTROLS_HPP

#include <expected>
#include <functional>
#include <siege/platform/win/window.hpp>
#include <siege/platform/win/drawing.hpp>
#include <CommCtrl.h>

namespace win32
{
  inline BOOL set_window_subclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
  inline BOOL remove_window_subclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass);
  inline LRESULT def_subclass_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  inline LRESULT get_window_subclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass, DWORD_PTR* pdwRefData);

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
          remove_window_subclass(hWnd, dispatcher::handle_message, uIdSubclass);
        }

        return def_subclass_proc(hWnd, uMsg, wParam, lParam);
      }
    };

    auto* context = new function_context{ source.get(), code, std::move(callback) };

    set_window_subclass(target.get(), dispatcher::handle_message, (UINT_PTR)context, 0);

    return [context, target = target.get()] {
      remove_window_subclass(target, dispatcher::handle_message, (UINT_PTR)context);
      delete context;
    };
  }

  struct button : window
  {
    using window::window;
    constexpr static auto class_name = WC_BUTTONW;
    constexpr static std::uint16_t dialog_id = 0x0080;

    [[maybe_unused]] inline std::function<void()> bind_bn_clicked(std::move_only_function<void(button, const NMHDR&)> callback)
    {
      return bind_notification<button, NMHDR>(this->GetParent()->ref(), this->ref(), BN_CLICKED, std::move(callback));
    }

    struct custom_draw_callbacks
    {
      std::move_only_function<win32::lresult_t(win32::button, NMCUSTOMDRAW&)> nm_custom_draw;
      std::move_only_function<win32::lresult_t(win32::button, DRAWITEMSTRUCT&)> wm_draw_item;
      std::move_only_function<HBRUSH(win32::button button, win32::gdi::drawing_context_ref)> wm_control_color;
    };

    [[maybe_unused]] std::function<void()> bind_custom_draw(custom_draw_callbacks callbacks)
    {
      struct function_context
      {
        HWND source;
        custom_draw_callbacks callbacks;
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

          if (uMsg == WM_CTLCOLORBTN && lParam && uIdSubclass)
          {
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.wm_control_color && (HWND)lParam == context.source)
            {
              return (LRESULT)context.callbacks.wm_control_color(win32::button((HWND)lParam), win32::gdi::drawing_context_ref((HDC)wParam));
            }
          }

          if (uMsg == WM_DRAWITEM && lParam && uIdSubclass)
          {
            auto* header = (DRAWITEMSTRUCT*)lParam;
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.wm_draw_item && header->hwndItem == context.source)
            {
              return context.callbacks.wm_draw_item(win32::button(header->hwndItem), *header);
            }
          }

          if (uMsg == WM_NOTIFY && lParam && uIdSubclass)
          {
            auto* header = (NMHDR*)lParam;
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.nm_custom_draw && header->hwndFrom == context.source && header->code == NM_CUSTOMDRAW)
            {
              return context.callbacks.nm_custom_draw(win32::button(header->hwndFrom), *(NMCUSTOMDRAW*)header);
            }
          }

          if (uMsg == WM_NCDESTROY)
          {
            auto* context = (function_context*)uIdSubclass;
            delete context;
            remove_window_subclass(hWnd, dispatcher::handle_message, uIdSubclass);
          }

          return def_subclass_proc(hWnd, uMsg, wParam, lParam);
        }
      };

      auto* context = new function_context{ this->get(), std::move(callbacks) };

      set_window_subclass(this->GetParent()->get(), dispatcher::handle_message, (UINT_PTR)context, 0);

      return [context, target = this->GetParent()->get()] {
        remove_window_subclass(target, dispatcher::handle_message, (UINT_PTR)context);
        delete context;
      };
    }
  };

  struct combo_box : window
  {
    using window::window;
    constexpr static auto class_name = WC_COMBOBOXW;
    constexpr static std::uint16_t dialog_id = 0x0085;

    [[maybe_unused]] inline std::function<void()> bind_cbn_sel_change(std::move_only_function<void(combo_box, const NMHDR&)> callback)
    {
      return bind_notification<combo_box, NMHDR>(this->GetParent()->ref(), this->ref(), CBN_SELCHANGE, std::move(callback));
    }
  };

  struct edit : window
  {
    using window::window;
    constexpr static auto class_name = WC_EDITW;
    constexpr static std::uint16_t dialog_id = 0x0081;

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


    HBITMAP SetImage(HBITMAP image)
    {
      return HBITMAP(SendMessageW(*this, STM_SETIMAGE, IMAGE_BITMAP, lparam_t(image)));
    }

    HBITMAP GetBitmap()
    {
      return HBITMAP(SendMessageW(*this, STM_GETIMAGE, IMAGE_BITMAP, 0));
    }

    struct custom_draw_callbacks
    {
      std::move_only_function<HBRUSH(static_control, win32::gdi::drawing_context_ref)> wm_control_color;
      std::move_only_function<win32::lresult_t(static_control, DRAWITEMSTRUCT&)> wm_draw_item;
    };

    [[maybe_unused]] std::function<void()> bind_custom_draw(custom_draw_callbacks callbacks)
    {
      struct function_context
      {
        HWND source;
        custom_draw_callbacks callbacks;
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

          if (uMsg == WM_CTLCOLORSTATIC && lParam && uIdSubclass)
          {
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.wm_control_color && (HWND)lParam == context.source)
            {
              return (LRESULT)context.callbacks.wm_control_color(win32::static_control((HWND)lParam), win32::gdi::drawing_context_ref((HDC)wParam));
            }
          }

          if (uMsg == WM_DRAWITEM && lParam && uIdSubclass)
          {
            auto* header = (DRAWITEMSTRUCT*)lParam;
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.wm_draw_item && header->hwndItem == context.source)
            {
              return context.callbacks.wm_draw_item(win32::static_control(header->hwndItem), *header);
            }
          }

          if (uMsg == WM_NCDESTROY)
          {
            auto* context = (function_context*)uIdSubclass;
            delete context;
            remove_window_subclass(hWnd, dispatcher::handle_message, uIdSubclass);
          }

          return def_subclass_proc(hWnd, uMsg, wParam, lParam);
        }
      };

      auto* context = new function_context{ this->get(), std::move(callbacks) };

      set_window_subclass(this->GetParent()->get(), dispatcher::handle_message, (UINT_PTR)context, 0);

      return [context, target = this->GetParent()->get()] {
        remove_window_subclass(target, dispatcher::handle_message, (UINT_PTR)context);
        delete context;
      };
    }
  };

  struct list_box : window
  {
    using window::window;
    constexpr static auto class_name = WC_LISTBOXW;
    constexpr static std::uint16_t dialog_id = 0x0083;

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

    /*
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
    
    */

    struct custom_draw_callbacks
    {
      std::move_only_function<HBRUSH(list_box, win32::gdi::drawing_context_ref)> wm_control_color;
      std::move_only_function<SIZE(list_box, const MEASUREITEMSTRUCT&)> wm_measure_item;
      std::move_only_function<win32::lresult_t(list_box, DRAWITEMSTRUCT&)> wm_draw_item;
    };

    [[maybe_unused]] std::function<void()> bind_custom_draw(custom_draw_callbacks callbacks)
    {
      struct function_context
      {
        HWND source;
        custom_draw_callbacks callbacks;
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
          if (uMsg == WM_CTLCOLORLISTBOX && lParam && uIdSubclass)
          {
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.wm_control_color && (HWND)lParam == context.source)
            {
              return (LRESULT)context.callbacks.wm_control_color(win32::list_box((HWND)lParam), win32::gdi::drawing_context_ref((HDC)wParam));
            }
          }

          if (uMsg == WM_MEASUREITEM && lParam && uIdSubclass)
          {
            auto& measure_context = *(MEASUREITEMSTRUCT*)lParam;

            if (measure_context.CtlType == ODT_LISTBOX)
            {
              auto& context = *(function_context*)uIdSubclass;

              SIZE size{};

              if (context.callbacks.wm_measure_item && context.source == ::GetDlgItem(hWnd, wParam))
              {
                size = context.callbacks.wm_measure_item(list_box(context.source), measure_context);
              }
              else if (context.callbacks.wm_measure_item && context.source == ::FindWindowExW(hWnd, nullptr, list_box::class_name, nullptr))
              {
                size = context.callbacks.wm_measure_item(list_box(context.source), measure_context);
              }
              else
              {
                ::DebugBreak();
                size.cx = ::GetSystemMetrics(SM_CXSIZE);
                size.cy = ::GetSystemMetrics(SM_CYSIZE);
              }

              measure_context.itemWidth = size.cx;
              measure_context.itemHeight = size.cy;
              return 0;
            }
          }

          if (uMsg == WM_DRAWITEM && lParam && uIdSubclass)
          {
            auto* header = (DRAWITEMSTRUCT*)lParam;
            auto& context = *(function_context*)uIdSubclass;

            if (context.callbacks.wm_draw_item && header->hwndItem == context.source)
            {
              return context.callbacks.wm_draw_item(list_box(header->hwndItem), *header);
            }
          }

          if (uMsg == WM_NCDESTROY)
          {
            auto* context = (function_context*)uIdSubclass;
            delete context;
            remove_window_subclass(hWnd, dispatcher::handle_message, uIdSubclass);
          }

          return def_subclass_proc(hWnd, uMsg, wParam, lParam);
        }
      };

      auto* context = new function_context{ this->get(), std::move(callbacks) };

      set_window_subclass(this->GetParent()->get(), dispatcher::handle_message, (UINT_PTR)context, 0);

      return [context, target = this->GetParent()->get()] {
        remove_window_subclass(target, dispatcher::handle_message, (UINT_PTR)context);
        delete context;
      };
    }
  };

  struct scroll_bar : window
  {
    using window::window;
    constexpr static auto class_name = WC_SCROLLBARW;
    constexpr static std::uint16_t dialog_id = 0x0084;
  };

}// namespace win32


#endif