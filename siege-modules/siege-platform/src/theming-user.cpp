#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/stream.hpp>
#include <VersionHelpers.h>
#include <dwmapi.h>

namespace win32
{
  gdi::brush_ref get_solid_brush(COLORREF color);

  void apply_theme(win32::window_ref& control)
  {
    struct sub_class
    {
      std::map<std::wstring_view, COLORREF> colors;

      sub_class(std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (uMsg == WM_ERASEBKGND)
        {
          auto context = win32::gdi::drawing_context_ref((HDC)wParam);

          auto self = win32::window_ref(hWnd);
          auto bk_color = ((sub_class*)dwRefData)->colors[win32::properties::window::bk_color];

          auto rect = self.GetClientRect();
          context.FillRect(*rect, get_solid_brush(bk_color));
          return TRUE;
        }

        if (uMsg == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
      }
    };

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::window::bk_color, win32::get_color_for_window(control.ref(),win32::properties::window::bk_color) }
    };

    DWORD_PTR existing_object{};

    if (!::GetWindowSubclass(control, sub_class::HandleMessage, (UINT_PTR)L"Window", &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(control, sub_class::HandleMessage, (UINT_PTR)L"Window", (DWORD_PTR) new sub_class(std::move(color_map)));
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }

    if (control.GetWindowStyle() & ~WS_CHILD)
    {
      BOOL value = win32::is_dark_theme() ? TRUE : FALSE;
      if (::DwmSetWindowAttribute(control, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value)) == S_OK)
      {
        ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
      }
    }
  }

  void apply_theme(win32::edit& control)
  {
    struct sub_class final : win32::edit::notifications
    {
      std::optional<HBRUSH> wm_control_color(win32::edit control, win32::gdi::drawing_context_ref dc) override
      {
        return std::nullopt;
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

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    DWORD_PTR existing_object{};

    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::button::class_name, &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::button::class_name, (DWORD_PTR) new sub_class());
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
  }

  void apply_theme(win32::button& control)
  {
    struct sub_class final : win32::button::notifications
    {
      HFONT font = win32::load_font(LOGFONTW{
        .lfPitchAndFamily = VARIABLE_PITCH,
        .lfFaceName = L"Segoe UI" });

      SIZE size{};
      HRGN region = nullptr;
      HBITMAP mask_bitmap = nullptr;

      std::wstring test = std::wstring(255, '\0');

      std::map<std::wstring_view, COLORREF> colors;

      sub_class(std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
      }

      std::optional<win32::lresult_t> wm_notify(win32::button button, NMCUSTOMDRAW& custom_draw) override
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

              // auto font_icon = win32::load_font(LOGFONTW{
              //   .lfHeight = -1024,
              //   .lfClipPrecision = CLIP_DEFAULT_PRECIS,
              //   .lfQuality = NONANTIALIASED_QUALITY,
              //   .lfFaceName = L"Segoe MDL2 Assets"
              //     });
              // std::wstring icon_text;
              // icon_text.push_back(0xE701);
              // mask_bitmap = win32::create_layer_mask(size, std::move(font_icon), icon_text);

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

          auto text_color = colors[properties::button::text_color];
          auto bk_color = colors[properties::button::bk_color];
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

            auto new_dc = win32::apply_layer_mask(win32::gdi::drawing_context_ref(custom_draw.hdc), win32::gdi::bitmap_ref(mask_bitmap));

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

      virtual std::optional<HBRUSH> wm_control_color(win32::button button, win32::gdi::drawing_context_ref context) override
      {
        auto text_color = colors[properties::button::text_color];

        if (text_color)
        {
          ::SetTextColor(context, text_color);
        }

        auto bk_color = colors[properties::button::bk_color];

        if (bk_color)
        {
          ::SetBkColor(context, bk_color);
          return get_solid_brush(bk_color);
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

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::button::bk_color, win32::get_color_for_window(control.ref(),win32::properties::button::bk_color) },
      { win32::properties::button::text_color, win32::get_color_for_window(control.ref(),win32::properties::button::text_color) },
      { win32::properties::button::line_color, win32::get_color_for_window(control.ref(),win32::properties::button::line_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::button::class_name, &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::button::class_name, (DWORD_PTR) new sub_class(std::move(color_map)));
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }

  void apply_theme(win32::static_control& control)
  {
    struct sub_class final : win32::static_control::notifications
    {
      std::map<std::wstring_view, COLORREF> colors;

      sub_class(std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
      }

      std::optional<HBRUSH> wm_control_color(win32::static_control static_control, win32::gdi::drawing_context_ref context) override
      {
        auto text_color = colors[properties::static_control::text_color];
       
         ::SetTextColor(context, text_color);

        auto bk_color = colors[properties::static_control::bk_color];
        ::SetBkColor(context, bk_color);
        return get_solid_brush(bk_color).get();
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto result = win32::static_control::notifications::dispatch_message((sub_class*)dwRefData, message, wParam, lParam);

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

    HFONT font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font, FALSE);

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::static_control::bk_color, win32::get_color_for_window(control.ref(),win32::properties::static_control::bk_color) },
      { win32::properties::static_control::text_color, win32::get_color_for_window(control.ref(),win32::properties::static_control::text_color) }
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::static_control::class_name, &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::static_control::class_name, (DWORD_PTR) new sub_class(std::move(color_map)));
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }


  void apply_theme(win32::list_box& control)
  {
    struct sub_class final : win32::list_box::notifications
    {
      std::map<std::wstring_view, COLORREF> colors;

      sub_class(std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
      }

      std::optional<HBRUSH> wm_control_color(win32::list_box list_box, win32::gdi::drawing_context_ref context) override
      {
        auto bk_color = colors[properties::list_box::bk_color];
        return get_solid_brush(bk_color).get();
      }

      SIZE wm_measure_item(win32::list_box themed_selection, const MEASUREITEMSTRUCT& item) override
      {
        return SIZE{ .cy = themed_selection.GetItemHeight(item.itemID) };
      }

      std::optional<win32::lresult_t> wm_draw_item(win32::list_box list, DRAWITEMSTRUCT& item) override
      {
        thread_local std::wstring buffer;
        auto context = win32::gdi::drawing_context_ref(item.hDC);

        auto item_height = list.GetItemHeight(0);
        HFONT font = win32::load_font(LOGFONTW{
          .lfPitchAndFamily = VARIABLE_PITCH,
          .lfFaceName = L"Segoe UI" });

        SelectFont(context, font);

        auto text_bk_color = colors[properties::list_box::text_bk_color];

        auto text_highlight_color = colors[properties::list_box::text_highlight_color];

        auto normal_brush = get_solid_brush(text_bk_color);
        auto selected_brush = get_solid_brush(text_highlight_color);

        context.FillRect(item.rcItem, item.itemState & ODS_SELECTED ? selected_brush : normal_brush);

        auto text_color = colors[properties::list_box::text_color];

        ::SetTextColor(context, text_color);
        ::SetBkMode(context, TRANSPARENT);

        buffer.resize(list.GetTextLength(item.itemID), '\0');

        list.GetText(item.itemID, buffer.data());

        ::TextOut(context, item.rcItem.left, item.rcItem.top, buffer.c_str(), buffer.size());

        return TRUE;
      }

      HWND current = nullptr;
      operator HWND()
      {
        return current;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto* self = (sub_class*)dwRefData;
        self->current = hWnd;
        auto result = win32::list_box::notifications::dispatch_message(self, message, wParam, lParam);

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

      gdi::font_ref font = win32::load_font(LOGFONTW{
        .lfPitchAndFamily = VARIABLE_PITCH,
        .lfFaceName = L"Segoe UI" });

      SendMessageW(*copy, WM_SETFONT, (WPARAM)font.get(), FALSE);

      std::wstring temp(256, '\0');

      auto count = control.GetCount();

      for (auto i = 0; i < count; ++i)
      {
        control.GetText(i, temp.data());
        copy->AddString(temp.data());

        auto item_height = control.GetItemHeight(i);
        ListBox_SetItemHeight(*copy, i, item_height);
      }

      ::DestroyWindow(control);
      control.reset(copy->release());
    };

    gdi::font_ref font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    auto size = get_font_size_for_string(std::move(font), L"A");

    if (size)
    {
      ListBox_SetItemHeight(control, 0, size->cy);
    }

    auto style = control.GetWindowStyle();
    copy_control(style | LBS_OWNERDRAWFIXED);

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::list_box::bk_color, win32::get_color_for_window(control.ref(),win32::properties::list_box::bk_color) },
      { win32::properties::list_box::text_color, win32::get_color_for_window(control.ref(),win32::properties::list_box::text_color) },
      { win32::properties::list_box::text_bk_color, win32::get_color_for_window(control.ref(),win32::properties::list_box::text_bk_color) },
      { win32::properties::list_box::text_highlight_color, win32::get_color_for_window(control.ref(),win32::properties::list_box::text_highlight_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::list_box::class_name, &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)win32::list_box::class_name, (DWORD_PTR) new sub_class(std::move(color_map)));
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }
}// namespace win32