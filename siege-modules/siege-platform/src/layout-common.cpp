#include <siege/platform/win/layout.hpp>

namespace win32
{
  //  TODO Add close button to the tab control
  // And trigger NM_CLICK when the button is clicked
  void enable_closeable_tabs(win32::tab_control& tab_control)
  {
    struct handler
    {
      static LRESULT __stdcall on_message(HWND window, UINT message, WPARAM wparam, LPARAM lparam, UINT_PTR id, DWORD_PTR data)
      {
        auto get_window_from_item = [window](auto item) {
          TCITEMW info{
            .mask = TCIF_PARAM
          };
          HWND child = nullptr;

          if (TabCtrl_GetItem(window, item, &info) && info.lParam && ::IsWindow((HWND)info.lParam))
          {
            child = (HWND)info.lParam;
          }

          return child;
        };

        if (message == TCM_INSERTITEM)
        {
          auto result = def_subclass_proc(window, message, wparam, lparam);

          if (result == -1)
          {
            return result;
          }

          auto button = ::FindWindowExW(window, nullptr, L"Button", nullptr);

          RECT size{};
          TabCtrl_GetItemRect(window, result, &size);
          
          if (button == nullptr)
          {
            auto height = size.bottom - size.top;
            auto width = win32::get_system_metrics(SM_CXSIZE);
            std::wstring close_text;
            close_text.append(1, 0x2715); // ✕
            button = ::CreateWindowExW(0, L"Button", close_text.c_str(), WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, size.right - width, size.top, width, height, window, (HMENU)result, nullptr, nullptr);

            HWND child = get_window_from_item(result);

            bool indestructible = child && ::GetPropW(child, L"TabIndestructible");

            ::EnableWindow(button, indestructible ? FALSE : TRUE);
          }

          return result;
        }

        if (message == TCM_SETPADDING)
        {
          auto button = ::FindWindowExW(window, nullptr, L"Button", nullptr);

          if (button)
          {
            ::SetPropW(button, L"TabItemXPadding", (HANDLE)LOWORD(lparam));
          }
        }

        if (message == WM_SIZE || message == TCM_SETITEMSIZE || (message == WM_NOTIFY && ((NMHDR*)lparam)->code == UDN_DELTAPOS))
        {
          auto result = def_subclass_proc(window, message, wparam, lparam);

          auto button = ::FindWindowExW(window, nullptr, L"Button", nullptr);

          if (button)
          {
            auto button_id = ::GetWindowLongPtrW(button, GWLP_ID);

            RECT size{};
            TabCtrl_GetItemRect(window, button_id, &size);
            auto height = size.bottom - size.top;
            auto width = win32::get_system_metrics(SM_CXSIZE);
            auto padding = (int)::GetPropW(button, L"TabItemXPadding") * 2;
            ::ShowWindow(button, SW_SHOW);
            ::MoveWindow(button, size.right - width, size.top, width, height, TRUE);
          }

          return result;
        }

        if (message == TCM_SETCURSEL || message == WM_LBUTTONDOWN || message == WM_KEYDOWN)
        {
          auto result = def_subclass_proc(window, message, wparam, lparam);

          auto current = TabCtrl_GetCurSel(window);
          auto button = ::FindWindowExW(window, nullptr, L"Button", nullptr);

          auto button_id = ::GetWindowLongPtrW(button, GWLP_ID);

          if (button && button_id != current)
          {
            ::SetWindowLongPtrW(button, GWLP_ID, (LONG_PTR)current);

            RECT size{};
            def_subclass_proc(window, TCM_GETITEMRECT, current, (LPARAM)&size);
            auto height = size.bottom - size.top;
            auto width = win32::get_system_metrics(SM_CXSIZE);

            auto padding = (int)::GetPropW(button, L"TabItemXPadding") * 2;

            HWND child = get_window_from_item(current);

            bool indestructible = child && ::GetPropW(child, L"TabIndestructible");

            ::EnableWindow(button, indestructible ? FALSE : TRUE);
            ::ShowWindow(button, SW_SHOW);
            ::MoveWindow(button, size.right - width, size.top, width, height, TRUE);
          }

          return result;
        }

        if (message == TCM_DELETEITEM)
        {
          HWND child = get_window_from_item(wparam);

          bool indestructible = child && ::GetPropW(child, L"TabIndestructible");

          if (indestructible)
          {
            return FALSE;
          }

          auto deleted = def_subclass_proc(window, message, wparam, lparam);

          if (deleted)
          {
            if (child)
            {
              ::DestroyWindow(child);
            }

            auto count = TabCtrl_GetItemCount(window);
            if (!count)
            {
              auto button = ::FindWindowExW(window, nullptr, L"Button", nullptr);

              if (button)
              {
                ShowWindow(button, SW_HIDE);
              }

              return deleted;
            }

            auto index = std::clamp<int>(wparam, 0, count - 1);
            TabCtrl_SetCurSel(window, index);

            auto parent = ::GetAncestor(window, GA_PARENT);

            if (parent)
            {
              auto window_id = (UINT)::GetWindowLongPtrW(window, GWLP_ID);
              NMHDR notice{
                .hwndFrom = window,
                .idFrom = window_id,
                .code = TCN_SELCHANGE
              };
              ::SendMessageW(parent, WM_NOTIFY, (WPARAM)window_id, (LPARAM)&notice);
            }
          }

          return deleted;
        }

        if (message == WM_COMMAND && HIWORD(wparam) == BN_CLICKED)
        {
          TabCtrl_DeleteItem(window, LOWORD(wparam));
        }


        if (message == WM_NCDESTROY)
        {
          remove_window_subclass(window, handler::on_message, 0);

          auto button = ::FindWindowExW(window, nullptr, L"Button", nullptr);

          if (button)
          {
            ::RemovePropW(button, L"TabItemXPadding");
          }
        }

        return def_subclass_proc(window, message, wparam, lparam);
      }
    };

    set_window_subclass(tab_control, handler::on_message, 0, 0);
  }
  // Add check for property to prevent tab from being closed
  //
}// namespace win32