#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/resource_storage.hpp>

namespace win32
{
  HBRUSH get_solid_brush(COLORREF color);

  void apply_theme(const win32::window_ref& colors, win32::window_ref& control)
  {
    struct sub_class
    {
      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (uMsg == WM_ERASEBKGND)
        {
          auto context = win32::gdi_drawing_context_ref((HDC)wParam);

          auto self = win32::window_ref(hWnd);
          auto bk_color = self.FindPropertyExW<COLORREF>(win32::properties::window::bk_color);
          
          if (bk_color)
          {
            auto rect = self.GetClientRect();
            context.FillRect(*rect, get_solid_brush(*bk_color));
            return TRUE;
          }
        }

        if (uMsg == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
      }
    };

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      auto bk_color = colors.FindPropertyExW(win32::properties::window::bk_color);

      if (bk_color)
      {
        control.SetPropW(win32::properties::window::bk_color, bk_color);
      }

      ::SetWindowSubclass(control, sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
    }
    else
    {
      control.RemovePropW(win32::properties::window::bk_color);
      ::RemoveWindowSubclass(control, sub_class::HandleMessage, (UINT_PTR)control.get());
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
  }

  
  void apply_theme(const win32::window_ref& colors, win32::list_box& control)
  {
    struct sub_class
    {
      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (uMsg == WM_CTLCOLORLISTBOX && lParam == uIdSubclass)
        {
          auto bk_color = win32::list_box((HWND)uIdSubclass).FindPropertyExW<COLORREF>(properties::list_box::bk_color);
          if (bk_color)
          {
            return (LRESULT)get_solid_brush(*bk_color);
          }
        }

        if (uMsg == WM_MEASUREITEM && lParam)
        {
          MEASUREITEMSTRUCT& item = *(MEASUREITEMSTRUCT*)lParam;

          if (item.CtlType == ODT_LISTBOX)
          {
            auto themed_selection = win32::list_box((HWND)uIdSubclass);

            item.itemHeight = themed_selection.GetItemHeight(item.itemID);
            return TRUE;
          }
        }

        if (uMsg == WM_DRAWITEM && lParam)
        {
          thread_local std::wstring buffer;
          DRAWITEMSTRUCT& item = *(DRAWITEMSTRUCT*)lParam;
          if (item.hwndItem == (HWND)uIdSubclass && (item.itemAction == ODA_DRAWENTIRE || item.itemAction == ODA_SELECT))
          {
            auto list = win32::list_box(item.hwndItem);
            auto context = win32::gdi_drawing_context_ref(item.hDC);

            auto text_bk_color = list.FindPropertyExW<COLORREF>(properties::list_box::text_bk_color);

            if (text_bk_color)
            {
              auto text_highlight_color = list.FindPropertyExW<COLORREF>(properties::list_box::text_highlight_color).value_or(*text_bk_color);

              auto normal_brush = get_solid_brush(*text_bk_color);
              auto selected_brush = get_solid_brush(text_highlight_color);

              context.FillRect(item.rcItem, item.itemState & ODS_SELECTED ? selected_brush : normal_brush);
            }

            auto text_color = list.FindPropertyExW<COLORREF>(properties::list_box::text_color);

            if (text_color)
            {
              ::SetTextColor(context, *text_color);
            }

            buffer.resize(list.GetTextLength(item.itemID));

            list.GetText(item.itemID, buffer.data());

            ::TextOut(context, item.rcItem.left, item.rcItem.top, buffer.c_str(), buffer.size());

            return TRUE;
          }
        }

        if (uMsg == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
      }
    };

    auto copy_control = [&](LONG style) {
      auto size = control.GetClientRect();
      auto parent = control.GetParent();
      if (auto real_size = control.MapWindowPoints(*parent, *size); real_size)
      {
        size = real_size->second;
      }

      auto copy = win32::window_module_ref::current_module().CreateWindowExW<win32::list_box>(CREATESTRUCTW{
        .hwndParent = *parent,
        .cy = size->bottom - size->top,
        .cx = size->right - size->left,
        .y = size->top,
        .x = size->left,
        .style = style,
      });

      std::wstring temp(256, '\0');

      auto count = control.GetCount();

      for (auto i = 0; i < count; ++i)
      {
        control.GetText(i, temp.data());
        copy->AddString(temp.data());
      }

      ::DestroyWindow(control);
      control.reset(copy->release());
    };

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      auto style = control.GetWindowStyle();
      copy_control(style | LBS_OWNERDRAWFIXED);

      colors.ForEachPropertyExW([&](auto, auto key, auto value) {
        if (key.find(win32::list_box::class_name) != std::wstring_view::npos)
        {
          control.SetPropW(key, value);
        }
      });

      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
    }
    else
    {
      auto style = control.GetWindowStyle();
      copy_control(style & ~LBS_OWNERDRAWFIXED);
      ::RemoveWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get());
    }
  }
}// namespace win32