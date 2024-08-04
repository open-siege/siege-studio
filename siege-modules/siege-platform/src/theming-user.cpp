#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/stream.hpp>
#include <VersionHelpers.h>

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
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      control.RemovePropW(win32::properties::window::bk_color);
      ::RemoveWindowSubclass(control, sub_class::HandleMessage, (UINT_PTR)control.get());
      ::RedrawWindow(control, nullptr, nullptr, RDW_ERASENOW);
    }
  }

  void apply_theme(const win32::window_ref& colors, win32::edit& control)
  {
    static HFONT font = ::CreateFontW(0,
      0,
      0,
      0,
      FW_DONTCARE,
      FALSE,
      FALSE,
      FALSE,
      DEFAULT_CHARSET,
      OUT_OUTLINE_PRECIS,
      CLIP_DEFAULT_PRECIS,
      CLEARTYPE_QUALITY,
      VARIABLE_PITCH,
      L"Segoe UI");

    struct sub_class final : win32::edit::notifications
    {
      std::pair<int, int> size;
      HRGN region = nullptr;

      std::optional<HBRUSH> wm_control_color(win32::edit control, win32::gdi_drawing_context_ref dc) override
      {
        SetTextColor(dc, RGB(0, 0, 255));

        auto rect = control.GetClientRect();

        auto new_pair = std::make_pair<int, int>(rect->right - rect->left, rect->bottom - rect->top);

        if (new_pair != size)
        {
          size.first = rect->right - rect->left;
          size.second = rect->bottom - rect->top;

          if (region)
          {
            DeleteObject(region);
            SetWindowRgn(control, nullptr, FALSE);
          }

          region = CreateRoundRectRgn(rect->left, rect->top, rect->right, rect->bottom, 10, 10);
          SetWindowRgn(control, region, FALSE);
        }

        //        SetDCPenColor(dc, RGB(255, 255, 0));
        //      SetDCBrushColor(dc, RGB(255, 255, 50));
        SelectObject(dc, get_solid_brush(RGB(250, 25, 80)));
        RoundRect(dc, rect->left, rect->top, rect->right, rect->bottom, 25, 25);

        SetBkMode(dc, TRANSPARENT);
        return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto result = win32::edit::notifications::dispatch_message((sub_class*)dwRefData, message, wParam, lParam);

        if (result)
        {
          return *result;
        }

        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      SetWindowLongPtrW(control,
        GWL_STYLE,
        GetWindowLongPtrW(control, GWL_STYLE) & ~WS_CLIPSIBLINGS);

      SendMessageW(control, WM_SETFONT, (WPARAM)font, FALSE);
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR) new sub_class());
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      sub_class* object = nullptr;
      if (::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR*)&object))
      {
        ::RemoveWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get());
        delete object;
        ::RedrawWindow(control, nullptr, nullptr, RDW_ERASENOW);
      }
    }
  }

  void apply_theme(const win32::window_ref& colors, win32::button& control)
  {
    struct sub_class final : win32::button::notifications
    {
      HFONT font = ::CreateFontW(0,
        0,
        0,
        0,
        FW_DONTCARE,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        VARIABLE_PITCH,
        L"Segoe UI");

      SIZE size{};
      HRGN region = nullptr;
      HBITMAP mask_bitmap = nullptr;

      std::wstring test = std::wstring(255, '\0');

      win32::lresult_t wm_notify(win32::button button, NMCUSTOMDRAW& custom_draw) override
      {
        if (custom_draw.dwDrawStage == CDDS_PREPAINT)
        {
          auto rect = custom_draw.rc;
          auto new_size = SIZE(rect.right - rect.left, rect.bottom - rect.top);
          if (new_size.cx != size.cx || new_size.cy != size.cy)
          {
            size = new_size;

            if (IsWindows8OrGreater())
            {
              auto scale = 16;

              mask_bitmap = win32::create_layer_mask(size, scale, [size = size](auto dc, auto scale) {
                RoundRect(dc, 0, 0, size.cx * scale, size.cy * scale, (size.cy * scale) / 2, size.cy * scale);
              });
            }
            else
            {
              if (region)
              {
                SetWindowRgn(button, nullptr, FALSE);
                DeleteObject(region);
              }
              region = CreateRoundRectRgn(0, 0, size.cx, size.cy, 25, 25);
              SetWindowRgn(button, region, FALSE);
            }
          }

          auto text_color = button.FindPropertyExW<COLORREF>(properties::button::text_color).value_or(RGB(128, 0, 0));
          auto bk_color = button.FindPropertyExW<COLORREF>(properties::button::bk_color).value_or(RGB(0, 128, 0));
          auto state = Button_GetState(button);

          SelectFont(custom_draw.hdc, font);
          if (state & BST_HOT)
          {
            ::SetTextColor(custom_draw.hdc, text_color);
            ::SetBkColor(custom_draw.hdc, RGB(0, 0, 255));
            ::FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(RGB(0, 0, 255)));
          }
          else if (state & BST_FOCUS)
          {
            ::SetTextColor(custom_draw.hdc, text_color);
            ::SetBkColor(custom_draw.hdc, RGB(0, 255, 255));
            ::FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(RGB(0, 255, 255)));
          }
          else if (state & BST_PUSHED)
          {
            ::SetTextColor(custom_draw.hdc, text_color);
            ::SetBkColor(custom_draw.hdc, RGB(255, 0, 255));
            ::FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(RGB(255, 0, 255)));
          }
          else
          {
            ::SetTextColor(custom_draw.hdc, text_color);
            ::SetBkColor(custom_draw.hdc, bk_color);
            ::FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(bk_color));
          }

          return CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
        }

        if (custom_draw.dwDrawStage == CDDS_POSTPAINT)
        {
          Button_GetText(button, test.data(), test.size());
          ::DrawTextExW(custom_draw.hdc, test.data(), -1, &custom_draw.rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP, nullptr);

          if (IsWindows8OrGreater())
          {
            auto width = custom_draw.rc.right - custom_draw.rc.left;
            auto height = custom_draw.rc.bottom - custom_draw.rc.top;

            auto new_dc = win32::apply_layer_mask(custom_draw.hdc, mask_bitmap);

            BLENDFUNCTION function{
              .BlendOp = AC_SRC_OVER,
              .SourceConstantAlpha = 255,
              .AlphaFormat = AC_SRC_ALPHA
            };

            auto screen_rect = std::make_optional(button.MapWindowPoints(*button.GetParent(), *button.GetClientRect())->second);
            POINT pos{ .x = screen_rect->left, .y = screen_rect->top };
            POINT dc_pos{};

            UpdateLayeredWindow(button, nullptr, &pos, &size, new_dc, &dc_pos, 0, &function, ULW_ALPHA);
          }
          else
          {
            if (region)
            {
              SetWindowRgn(button, region, TRUE);
            }
          }
        }

        return CDRF_DODEFAULT;
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(win32::button, DRAWITEMSTRUCT&) override
      {
        return std::nullopt;
      }

      virtual std::optional<HBRUSH> wm_control_color(win32::button button, win32::gdi_drawing_context_ref context) override
      {
        auto text_color = button.FindPropertyExW<COLORREF>(properties::button::text_color);

        if (text_color)
        {
          ::SetTextColor(context, *text_color);
        }

        auto bk_color = button.FindPropertyExW<COLORREF>(properties::button::bk_color);

        if (bk_color)
        {
          ::SetBkColor(context, *bk_color);
          return get_solid_brush(*bk_color);
        }

        return std::nullopt;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto result = win32::button::notifications::dispatch_message((sub_class*)dwRefData, message, wParam, lParam);

        if (result)
        {
          return *result;
        }

        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        auto lresult = DefSubclassProc(hWnd, message, wParam, lParam);

        if (message == WM_SIZE || message == WM_MOVE)
        {
          for (HWND button = FindWindowExW(hWnd, nullptr, win32::button::class_name, nullptr);
               button != nullptr;
               button = FindWindowExW(hWnd, button, win32::button::class_name, nullptr))
          {
            if (button && GetWindowLongPtrW(button, GWL_EXSTYLE) & WS_EX_LAYERED)
            {
              InvalidateRect(button, nullptr, TRUE);
            }
          }
        }

        return lresult;
      }
    };


    if (IsWindows8OrGreater())
    {
      SetWindowLongPtrW(control,
        GWL_EXSTYLE,
        GetWindowLongPtrW(control, GWL_EXSTYLE) | WS_EX_LAYERED);
    }
    else
    {
      SetWindowLongPtrW(control,
        GWL_STYLE,
        GetWindowLongPtrW(control, GWL_STYLE) | WS_CLIPSIBLINGS);
    }

    auto use_dark_theme = colors.GetPropW<bool>(L"AppsUseDarkTheme");
    auto bk_color = colors.FindPropertyExW<COLORREF>(properties::button::bk_color).value_or(use_dark_theme ? 0 : 0xffffffff);
    auto text_color = colors.FindPropertyExW<COLORREF>(properties::button::text_color).value_or(use_dark_theme ? 0xffffffff : 0);
    auto line_color = colors.FindPropertyExW<COLORREF>(properties::button::line_color).value_or(0x11111111);
    control.SetPropW(win32::properties::button::bk_color, bk_color);
    control.SetPropW(win32::properties::button::text_color, text_color);
    control.SetPropW(win32::properties::button::line_color, line_color);

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::button::class_name, &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::button::class_name, (DWORD_PTR) new sub_class());
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
  }

  void apply_theme(const win32::window_ref& colors, win32::static_control& control)
  {
    struct sub_class
    {
      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (uMsg == WM_CTLCOLORSTATIC && lParam == uIdSubclass)
        {
          auto text_color = win32::static_control((HWND)uIdSubclass).FindPropertyExW<COLORREF>(properties::static_control::text_color);
          if (text_color)
          {
            ::SetTextColor((HDC)wParam, *text_color);
          }

          auto bk_color = win32::static_control((HWND)uIdSubclass).FindPropertyExW<COLORREF>(properties::static_control::bk_color);
          if (bk_color)
          {
            ::SetBkColor((HDC)wParam, *bk_color);
            return (LRESULT)get_solid_brush(*bk_color);
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

      colors.ForEachPropertyExW([&](auto, auto key, auto value) {
        if (key.find(win32::static_control::class_name) != std::wstring_view::npos)
        {
          control.SetPropW(key, value);
        }
      });

      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      control.RemovePropW(win32::properties::static_control::bk_color);
      control.RemovePropW(win32::properties::static_control::text_color);
      ::RemoveWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get());
      ::RedrawWindow(control, nullptr, nullptr, RDW_ERASENOW);
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

            buffer.resize(list.GetTextLength(item.itemID), '\0');

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

    ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
  }
}// namespace win32