#ifndef PLATFORM_DIALOG_HPP
#define PLATFORM_DIALOG_HPP
#include <siege/platform/win/window.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <functional>

namespace win32
{
  struct default_dialog : ::DLGTEMPLATE
  {
    default_dialog(::DLGTEMPLATE data) : ::DLGTEMPLATE(std::move(data))
    {
    }

    alignas(WORD) WORD menu_id = 0;
    alignas(WORD) WORD class_id = 0;
    alignas(WORD) WORD title_id = 0;
  };

  template<typename TWindow>
  struct dialog_message_handler
  {
    static INT_PTR __stdcall on_message(HWND self, UINT message, WPARAM wparam, LPARAM lparam)
    {
      TWindow* instance = nullptr;

      if (message == WM_INITDIALOG)
      {
        RECT client_rect{};

        ::GetClientRect(self, &client_rect);

        CREATESTRUCTW params{
          .lpCreateParams = (void*)lparam,
          .hInstance = (HINSTANCE)::GetWindowLongPtrW(self, GWLP_HINSTANCE),
          .hMenu = ::GetMenu(self),
          .hwndParent = ::GetParent(self),
          .cy = (int)(client_rect.bottom - client_rect.top),
          .cx = (int)(client_rect.right - client_rect.left),
          .y = (int)client_rect.top,
          .x = (int)client_rect.left,
          .style = (LONG)::GetWindowLongPtrW(self, GWL_STYLE),
          .dwExStyle = (DWORD)::GetWindowLongPtr(self, GWL_EXSTYLE)
        };
        auto* window = new TWindow(self, params);
        window->wm_create();

        ::SetWindowLongPtrW(self, DWLP_USER, (LONG_PTR)window);

        return FALSE;
      }
      else
      {
        instance = (TWindow*)GetWindowLongPtrW(self, DWLP_USER);
      }

      if (instance)
      {
        auto result = dispatch_message(instance, message, wparam, lparam);

        if (message == WM_NCDESTROY)
        {
          delete instance;
        }

        if (result)
        {
          if (message == WM_CTLCOLORBTN || message == WM_CTLCOLORDLG || message == WM_CTLCOLOREDIT || message == WM_CTLCOLORLISTBOX || message == WM_CTLCOLORSCROLLBAR || message == WM_CTLCOLORSTATIC)
          {
            return (INT_PTR)*result;
          }

          SetWindowLongPtrW(self, DWLP_MSGRESULT, *result);
          return TRUE;
        }
      }

      return FALSE;
    }
  };

  template<typename TWindow>
  auto DialogBoxIndirectParamW(HINSTANCE instance, default_dialog dialog, win32::window_ref parent, LPARAM extra_data = 0)
  {
    return ::DialogBoxIndirectParamW(instance, &dialog, parent, dialog_message_handler<TWindow>::on_message, extra_data);
  }

  template<typename TWindow>
  auto CreateDialogBoxIndirectParamW(HINSTANCE instance, default_dialog dialog, win32::window_ref parent, LPARAM extra_data = 0)
  {
    return ::CreateDialogIndirectParamW(instance, &dialog, parent, dialog_message_handler<TWindow>::on_message, extra_data);
  }

  inline auto DialogBoxIndirectParamW(HINSTANCE instance, default_dialog dialog, win32::window_ref parent, std::move_only_function<std::optional<LRESULT>(win32::window_ref, UINT, WPARAM, LPARAM)> dialog_proc)
  {
    struct handler
    {
      static INT_PTR __stdcall on_message(HWND self, UINT message, WPARAM wparam, LPARAM lparam)
      {
        if (message == WM_INITDIALOG)
        {
          auto proc = (std::add_pointer_t<decltype(dialog_proc)>)lparam;
          auto result = (*proc)(win32::window_ref(self), message, wparam, lparam);
          ::SetWindowLongPtrW(self, DWLP_USER, (LONG_PTR)proc);

          if (result)
          {
            return (INT_PTR)*result;
          }

          return TRUE;
        }

        auto proc = (std::add_pointer_t<decltype(dialog_proc)>)::GetWindowLongPtrW(self, DWLP_USER);

        if (!proc)
        {
          return FALSE;
        }

        auto result = (*proc)(win32::window_ref(self), message, wparam, lparam);

        if (result)
        {
          SetWindowLongPtrW(self, DWLP_MSGRESULT, *result);
        }
        
        if (message == WM_NCDESTROY)
        {
          delete proc;
          ::SetWindowLongPtrW(self, DWLP_USER, 0);
        }

        return result ? TRUE : FALSE;
      }
    };

    auto* temp = new decltype(dialog_proc)(std::move(dialog_proc));
    return ::DialogBoxIndirectParamW(instance, &dialog, parent, handler::on_message, (LPARAM)temp);
  }

}// namespace win32
#endif