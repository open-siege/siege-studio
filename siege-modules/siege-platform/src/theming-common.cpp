#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/stream.hpp>

namespace win32
{
  gdi::brush_ref get_solid_brush(COLORREF color);

  void apply_theme(win32::header& control)
  {
    struct sub_class
    {
      std::map<std::wstring_view, COLORREF> colors;
      std::function<void()> bind_remover;

      sub_class(win32::header& control, std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
        bind_remover = control.bind_custom_draw({ .nm_custom_draw = std::bind_front(&sub_class::nm_custom_draw, this) });
      }

      ~sub_class()
      {
        bind_remover();
      }

      win32::lresult_t nm_custom_draw(win32::header header, NMCUSTOMDRAW& custom_draw)
      {
        if (custom_draw.dwDrawStage == CDDS_PREPAINT)
        {
          auto font = win32::load_font(LOGFONTW{
            .lfPitchAndFamily = VARIABLE_PITCH,
            .lfFaceName = L"Segoe UI" });

          auto text_bk_color = colors[properties::header::text_bk_color];
          FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(text_bk_color));

          return CDRF_NOTIFYITEMDRAW | CDRF_NEWFONT;
        }


        if (custom_draw.dwDrawStage == CDDS_ITEMPREPAINT)
        {
          auto focused_item = Header_GetFocusedItem(header);

          auto text_highlight_color = colors[properties::header::text_highlight_color];
          auto text_bk_color = colors[properties::header::text_bk_color];

          if (custom_draw.dwItemSpec == focused_item)
          {
            ::SetBkColor(custom_draw.hdc, text_highlight_color);
            ::SelectObject(custom_draw.hdc, get_solid_brush(text_highlight_color));
          }
          else
          {
            ::SetBkColor(custom_draw.hdc, text_bk_color);
            ::SelectObject(custom_draw.hdc, get_solid_brush(text_bk_color));
          }

          auto text_color = colors[properties::header::text_color];
          ::SetTextColor(custom_draw.hdc, text_color);

          return CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
        }

        if (custom_draw.dwDrawStage == CDDS_ITEMPOSTPAINT)
        {
          auto rect = custom_draw.rc;

          if (header.GetWindowStyle() & HDS_FILTERBAR)
          {
            thread_local std::wstring filter_value;
            filter_value.clear();
            filter_value.resize(255, L'\0');
            HD_TEXTFILTERW string_filter{
              .pszText = filter_value.data(),
              .cchTextMax = (int)filter_value.size(),
            };

            win32::gdi::drawing_context_ref context(custom_draw.hdc);
            auto header_item = header.GetItem(custom_draw.dwItemSpec, { .mask = HDI_FILTER, .type = HDFT_ISSTRING, .pvFilter = &string_filter });

            filter_value.erase(std::wcslen(filter_value.data()));


            auto bottom = custom_draw.rc;

            rect.bottom = rect.bottom / 2;
            bottom.top = rect.bottom;
            FillRect(custom_draw.hdc, &bottom, get_solid_brush(colors[properties::header::text_bk_color]));

            bottom.left += 10;
            bottom.right -= 10;
            if (filter_value.empty())
            {
              ::DrawTextW(context, L"Enter text here", -1, &bottom, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
            }
            else
            {
              ::DrawTextW(context, filter_value.c_str(), -1, &bottom, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
            }
          }

          return CDRF_DODEFAULT;
        }

        return CDRF_DODEFAULT;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_DESTROY)
        {
          auto* context = (sub_class*)dwRefData;
          delete context;
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::header::bk_color, win32::get_color_for_window(control.ref(), win32::properties::header::bk_color) },
      { win32::properties::header::text_color, win32::get_color_for_window(control.ref(), win32::properties::header::text_color) },
      { win32::properties::header::text_bk_color, win32::get_color_for_window(control.ref(), win32::properties::header::text_bk_color) },
      { win32::properties::header::text_highlight_color, win32::get_color_for_window(control.ref(), win32::properties::header::text_highlight_color) },
    };

    win32::theme_module().SetWindowTheme(control, L"", L"");
    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), &existing_object) && existing_object == 0)
    {
      ::SendMessageW(control, CCM_DPISCALE, TRUE, 0);
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR) new sub_class(control, std::move(color_map)));
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }

  void apply_theme(win32::tab_control& control)
  {
    struct sub_class
    {
      std::map<std::wstring_view, COLORREF> colors;
      std::optional<SIZE> padding;
      std::function<void()> bind_remover;

      sub_class(win32::tab_control& control, std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
        bind_remover = control.bind_custom_draw({ .wm_draw_item = std::bind_front(&sub_class::wm_draw_item, this) });
      }

      ~sub_class()
      {
        bind_remover();
      }

      win32::lresult_t wm_draw_item(win32::tab_control tabs, DRAWITEMSTRUCT& item)
      {
        thread_local std::wstring buffer(256, '\0');

        if (item.itemAction == ODA_DRAWENTIRE || item.itemAction == ODA_SELECT)
        {
          auto context = win32::gdi::drawing_context_ref(item.hDC);

          SetBkColor(context, colors[win32::properties::tab_control::bk_color]);

          win32::tab_control control(item.hwndItem);

          auto item_rect = item.rcItem;

          auto text_highlight_color = colors[win32::properties::tab_control::text_highlight_color];
          auto text_bk_color = colors[win32::properties::tab_control::text_bk_color];

          if (item.itemState & ODS_HOTLIGHT)
          {
            context.FillRect(item_rect, get_solid_brush(text_highlight_color));
          }
          else if (item.itemState & ODS_SELECTED)
          {
            context.FillRect(item_rect, get_solid_brush(text_highlight_color));
          }
          else
          {
            context.FillRect(item_rect, get_solid_brush(text_bk_color));
          }

          auto text_color = colors[win32::properties::tab_control::text_color];
          ::SetTextColor(context, text_color);
          ::SetBkMode(context, TRANSPARENT);
          ::SelectObject(context, get_solid_brush(text_bk_color));

          auto item_info = control.GetItem(item.itemID, TCITEMW{ .mask = TCIF_TEXT, .pszText = buffer.data(), .cchTextMax = int(buffer.size()) });

          if (item_info)
          {
            auto real_rect = item.rcItem;

            if (this->padding)
            {
              real_rect.left += this->padding->cx;
              real_rect.right -= this->padding->cx;
              real_rect.top += this->padding->cy;
              real_rect.bottom -= this->padding->cy;
              ::DrawTextW(context, (LPCWSTR)item_info->pszText, -1, &real_rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
            }
            else
            {
              ::DrawTextW(context, (LPCWSTR)item_info->pszText, -1, &real_rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
            }
          }

          return TRUE;
        }

        return FALSE;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_DESTROY)
        {
          auto* context = (sub_class*)dwRefData;
          delete context;
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }

      static LRESULT __stdcall HandleChildMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        auto* self = (sub_class*)dwRefData;
        if (message == TCM_SETPADDING)
        {
          self->padding = SIZE{ .cx = LOWORD(lParam), .cy = HIWORD(lParam) };
        }

        if (message == WM_PAINT)
        {
          PAINTSTRUCT ps{};
          HDC hdc = wParam != 0 ? (HDC)wParam : BeginPaint(hWnd, &ps);
          auto tabs = win32::tab_control(hWnd);
          auto parent_context = win32::gdi::drawing_context_ref(hdc);
          auto rect = tabs.GetClientRect();
          auto bk_color = self->colors[win32::properties::tab_control::bk_color];

          if (ps.fErase)
          {
            parent_context.FillRect(*rect, get_solid_brush(bk_color));
          }

          auto result = DefSubclassProc(hWnd, message, (WPARAM)hdc, lParam);

          auto text_bk_color = self->colors[win32::properties::tab_control::text_bk_color];
          auto pen = CreatePen(PS_SOLID, 3, text_bk_color);

          auto old_pen = SelectObject(hdc, pen);
          SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

          SetDCPenColor(hdc, RGB(0, 0, 0));


          auto x_border = GetSystemMetrics(SM_CXEDGE);
          auto y_border = GetSystemMetrics(SM_CYEDGE);


          auto client_area = tabs.GetClientRect();

          client_area = tabs.AdjustRect(false, *client_area);

          client_area->left = std::clamp<LONG>(client_area->left - x_border, 0, client_area->left);
          client_area->right += x_border;
          client_area->top = std::clamp<LONG>(client_area->top - y_border, 0, client_area->top);
          client_area->bottom += y_border;

          auto count = tabs.GetItemCount();

          if (count > 0)
          {
            auto tab_rect = tabs.GetItemRect(count - 1);
            rect->left = tab_rect->right;
            rect->bottom = tab_rect->bottom;
          }

          if (ps.fErase)
          {
            parent_context.FillRect(*rect, get_solid_brush(bk_color));
            parent_context.FillRect(*client_area, get_solid_brush(bk_color));
          }

          RECT item_rect;
          for (auto i = 0; i < TabCtrl_GetItemCount(hWnd); ++i)
          {
            TabCtrl_GetItemRect(hWnd, i, &item_rect);
            Rectangle(hdc, item_rect.left, item_rect.top, item_rect.right, item_rect.bottom);
          }

          SelectObject(hdc, old_pen);
          DeleteObject(pen);

          if (wParam == 0)
          {
            EndPaint(hWnd, &ps);

          }

          auto up_down = win32::window_ref(::FindWindowExW(hWnd, nullptr, UPDOWN_CLASSW, nullptr));

          if (up_down)
          {
            if (win32::is_dark_theme())
            {
              win32::theme_module().SetWindowTheme(up_down, L"DarkMode_Explorer", nullptr);
            }
            else
            {
              win32::theme_module().SetWindowTheme(up_down, nullptr, nullptr);
            }
          }

          return result;
        }

        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleChildMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    auto style = control.GetWindowStyle();
    control.SetWindowStyle(style | TCS_OWNERDRAWFIXED);


    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::tab_control::bk_color, win32::get_color_for_window(control.ref(), win32::properties::tab_control::bk_color) },
      { win32::properties::tab_control::text_color, win32::get_color_for_window(control.ref(), win32::properties::tab_control::text_color) },
      { win32::properties::tab_control::text_bk_color, win32::get_color_for_window(control.ref(), win32::properties::tab_control::text_bk_color) },
      { win32::properties::tab_control::text_highlight_color, win32::get_color_for_window(control.ref(), win32::properties::tab_control::text_highlight_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), &existing_object) && existing_object == 0)
    {
      auto data = (DWORD_PTR) new sub_class(control, std::move(color_map));
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), data);
      ::SetWindowSubclass(control, sub_class::HandleChildMessage, (UINT_PTR)win32::tab_control::class_name, data);
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }

  void apply_theme(win32::list_view& control)
  {
    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::list_view::bk_color, win32::get_color_for_window(control.ref(), properties::list_view::bk_color) },
      { win32::properties::list_view::text_color, win32::get_color_for_window(control.ref(), win32::properties::list_view::text_color) },
      { win32::properties::list_view::text_bk_color, win32::get_color_for_window(control.ref(), win32::properties::list_view::text_bk_color) },
      { win32::properties::list_view::outline_color, win32::get_color_for_window(control.ref(), win32::properties::list_view::outline_color) },
    };

    ListView_SetBkColor(control, color_map[properties::list_view::bk_color]);
    ListView_SetTextColor(control, color_map[properties::list_view::text_color]);
    ListView_SetTextBkColor(control, color_map[properties::list_view::text_bk_color]);
    ListView_SetOutlineColor(control, color_map[properties::list_view::outline_color]);

    auto header = control.GetHeader();

    if (header)
    {
      win32::apply_theme(header);
    }

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    if (win32::is_dark_theme())
    {
      win32::theme_module().SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
    }

    struct sub_class final
    {
      std::map<std::wstring_view, COLORREF> colors;
      std::function<void()> bind_remover;

      sub_class(win32::list_view& control, std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
        bind_remover = control.bind_custom_draw({ .nm_custom_draw = std::bind_front(&sub_class::nm_custom_draw, this) });
      }

      ~sub_class()
      {
        bind_remover();
      }

      win32::lresult_t nm_custom_draw(win32::list_view control, NMLVCUSTOMDRAW& custom_draw)
      {
        if (custom_draw.dwItemType == LVCDI_GROUP && custom_draw.nmcd.dwDrawStage == CDDS_PREPAINT)
        {
          const int nGroupId = int(custom_draw.nmcd.dwItemSpec);

          std::array<wchar_t, 255> group_name{};
          LVGROUP lvg = {
            .cbSize = sizeof(LVGROUP),
            .mask = LVGF_HEADER | LVGF_STATE,
            .pszHeader = group_name.data(),
            .cchHeader = 255,
            .stateMask = 0xff,
          };

          if (ListView_GetGroupInfo(control, nGroupId, &lvg))
          {
            RECT header_rect{};
            ListView_GetGroupRect(control, nGroupId, LVGGR_HEADER, &header_rect);

            SetTextColor(custom_draw.nmcd.hdc, colors[properties::list_view::text_color]);

            auto font = win32::load_font(LOGFONTW{
              .lfPitchAndFamily = VARIABLE_PITCH,
              .lfFaceName = L"Segoe MDL2 Assets" });

            std::wstring icon;

            if (lvg.state & LVGS_COLLAPSED)
            {
              icon.push_back(0xE76C);// ChevronRight
            }
            else if (lvg.state & ~LVGS_HIDDEN)
            {
              icon.push_back(0xE70D);// ChevronDown
            }

            if (!icon.empty())
            {
              SelectFont(custom_draw.nmcd.hdc, font);
              auto rect = header_rect;

              SIZE text_size{};
              GetTextExtentPoint32W(custom_draw.nmcd.hdc, icon.data(), 1, &text_size);

              rect.right = rect.right - text_size.cx;

              DrawTextExW(custom_draw.nmcd.hdc, icon.data(), -1, &rect, DT_SINGLELINE | DT_RIGHT | DT_VCENTER, nullptr);
            }

            font = win32::load_font(LOGFONTW{
              .lfPitchAndFamily = VARIABLE_PITCH,
              .lfFaceName = L"Segoe UI" });

            SelectFont(custom_draw.nmcd.hdc, font);


            DrawTextExW(custom_draw.nmcd.hdc, group_name.data(), -1, &header_rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER, nullptr);
          }

          return CDRF_SKIPDEFAULT;
        }

        return CDRF_DODEFAULT;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_DESTROY)
        {
          auto* context = (sub_class*)dwRefData;
          delete context;
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), &existing_object) && existing_object == 0)
    {
      ::SendMessageW(control, CCM_DPISCALE, TRUE, 0);
      auto data = (DWORD_PTR) new sub_class(control, std::move(color_map));
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), data);
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }

    ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
  }

  void apply_theme(win32::tree_view& control)
  {
    auto useDarkMode = win32::is_dark_theme();
    auto color = win32::get_color_for_window(control.ref(), properties::tree_view::bk_color);
    TreeView_SetBkColor(control, color);

    color = win32::get_color_for_window(control.ref(), properties::tree_view::text_color);
    TreeView_SetTextColor(control, color);

    color = win32::get_color_for_window(control.ref(), properties::tree_view::line_color);
    TreeView_SetLineColor(control, color);

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    if (useDarkMode)
    {
      win32::theme_module().SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
    }
    ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
  }

  void apply_theme(win32::tool_bar& control)
  {
    auto highlight_color = win32::get_color_for_window(control.ref(), properties::tool_bar::btn_highlight_color);
    auto shadow_color = win32::get_color_for_window(control.ref(), properties::tool_bar::btn_shadow_color);

    bool change_theme = false;

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    win32::theme_module().SetWindowTheme(control, L"", L"");
    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    COLORSCHEME scheme{ .dwSize = sizeof(COLORSCHEME), .clrBtnHighlight = CLR_DEFAULT, .clrBtnShadow = CLR_DEFAULT };

    scheme.clrBtnHighlight = highlight_color;
    scheme.clrBtnShadow = shadow_color;

    ::SendMessageW(control, TB_SETCOLORSCHEME, 0, (LPARAM)&scheme);

    struct sub_class
    {
      std::map<std::wstring_view, COLORREF> colors;
      std::function<void()> bind_remover;

      sub_class(win32::tool_bar& control, std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
        bind_remover = control.bind_custom_draw({ .nm_custom_draw = std::bind_front(&sub_class::nm_custom_draw, this) });
      }

      ~sub_class()
      {
        bind_remover();
      }

      win32::lresult_t nm_custom_draw(win32::tool_bar buttons, NMTBCUSTOMDRAW& custom_draw)
      {
        if (custom_draw.nmcd.dwDrawStage == CDDS_PREPAINT)
        {
          auto font = win32::load_font(LOGFONTW{
            .lfPitchAndFamily = VARIABLE_PITCH,
            .lfFaceName = L"Segoe UI" });

          SelectFont(custom_draw.nmcd.hdc, font);
          return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
        }

        if (custom_draw.nmcd.dwDrawStage == CDDS_ITEMPREPAINT || custom_draw.nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
        {
          custom_draw.clrBtnFace = colors[properties::tool_bar::btn_face_color];
          custom_draw.clrBtnHighlight = colors[properties::tool_bar::btn_highlight_color];
          custom_draw.clrText = colors[properties::tool_bar::text_color];

          return CDRF_NEWFONT | TBCDRF_USECDCOLORS;
        }

        if (custom_draw.nmcd.dwDrawStage == CDDS_POSTPAINT)
        {
          win32::gdi::drawing_context_ref context(custom_draw.nmcd.hdc);

          auto count = buttons.ButtonCount();
          auto rect = buttons.GetClientRect();
          if (count > 0)
          {
            auto button_rect = buttons.GetItemRect(count - 1);

            rect->left = button_rect->right;
            rect->bottom = button_rect->bottom;

            context.FillRect(*rect, get_solid_brush(colors[properties::tool_bar::bk_color]));
          }
        }

        return CDRF_DODEFAULT;
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_DESTROY)
        {
          auto* context = (sub_class*)dwRefData;
          delete context;
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::tool_bar::bk_color, win32::get_color_for_window(control.ref(), win32::properties::tool_bar::bk_color) },
      { win32::properties::tool_bar::text_color, win32::get_color_for_window(control.ref(), win32::properties::tool_bar::text_color) },
      { win32::properties::tool_bar::btn_face_color, win32::get_color_for_window(control.ref(), win32::properties::tool_bar::btn_face_color) },
      { win32::properties::tool_bar::btn_highlight_color, win32::get_color_for_window(control.ref(), win32::properties::tool_bar::btn_highlight_color) },
      { win32::properties::tool_bar::btn_shadow_color, win32::get_color_for_window(control.ref(), win32::properties::tool_bar::btn_shadow_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), &existing_object) && existing_object == 0)
    {
      ::SendMessageW(control, CCM_DPISCALE, TRUE, 0);
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR) new sub_class(control, std::move(color_map)));
      ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
  }


  void apply_theme(win32::track_bar& control)
  {
    struct sub_class
    {
      std::map<std::wstring_view, COLORREF> colors;
      HPEN pen = CreatePen(PS_SOLID, 10, RGB(255, 255, 0));

      gdi::font_ref ui_icons = win32::load_font(LOGFONTW{
        .lfPitchAndFamily = VARIABLE_PITCH,
        .lfFaceName = L"Segoe MDL2 Assets" });

      win32::lresult_t wm_notify(win32::track_bar track_bar, NMCUSTOMDRAW& custom_draw)
      {
        if (custom_draw.dwDrawStage == CDDS_PREPAINT)
        {
          return CDRF_NOTIFYITEMDRAW;
        }

        auto bk_color = colors[properties::track_bar::bk_color];
        auto text_color = colors[properties::track_bar::text_color];
        auto text_bk_color = colors[properties::track_bar::text_bk_color];

        if (custom_draw.dwDrawStage == CDDS_ITEMPREPAINT && custom_draw.dwItemSpec == TBCD_CHANNEL)
        {
          auto client_rect = track_bar.GetClientRect();
          FillRect(custom_draw.hdc, &*client_rect, get_solid_brush(bk_color));
          FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(text_bk_color));
          return CDRF_SKIPDEFAULT;
        }

        if (custom_draw.dwDrawStage == CDDS_ITEMPREPAINT && custom_draw.dwItemSpec == TBCD_THUMB)
        {
          SetBkColor(custom_draw.hdc, bk_color);
          SetBkMode(custom_draw.hdc, TRANSPARENT);

          if (custom_draw.uItemState & CDIS_SELECTED)
          {
            SetTextColor(custom_draw.hdc, is_dark_theme() ? RGB(127, 127, 200) : RGB(0, 0, 200));
          }
          else
          {
            POINT mouse{};
            if (::GetCursorPos(&mouse) && ::ScreenToClient(track_bar, &mouse) && ::PtInRect(&custom_draw.rc, mouse))
            {
              SetTextColor(custom_draw.hdc, RGB(127, 127, 127));
            }
            else
            {
              SetTextColor(custom_draw.hdc, text_color);
            }
          }

          SelectFont(custom_draw.hdc, ui_icons);
          std::wstring slider_thumb(1, (wchar_t)segoe_fluent_icons::slider_thumb);
          DrawTextExW(custom_draw.hdc, slider_thumb.data(), -1, &custom_draw.rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER, nullptr);

          return CDRF_SKIPDEFAULT;
        }

        if (custom_draw.dwDrawStage == CDDS_ITEMPREPAINT && custom_draw.dwItemSpec == TBCD_TICS)
        {
          FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(text_color));
          return CDRF_SKIPDEFAULT;
        }

        return CDRF_DODEFAULT;
      }

      sub_class(std::map<std::wstring_view, COLORREF> colors) : colors(std::move(colors))
      {
      }

      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
      }
    };

    std::map<std::wstring_view, COLORREF> color_map{
      { win32::properties::track_bar::bk_color, win32::get_color_for_window(control.ref(), win32::properties::track_bar::bk_color) },
      { win32::properties::track_bar::text_color, win32::get_color_for_window(control.ref(), win32::properties::track_bar::text_color) },
      { win32::properties::track_bar::text_bk_color, win32::get_color_for_window(control.ref(), win32::properties::track_bar::text_bk_color) },
    };

    DWORD_PTR existing_object{};
    if (!::GetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), &existing_object) && existing_object == 0)
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR) new sub_class(std::move(color_map)));
    }
    else
    {
      ((sub_class*)existing_object)->colors = std::move(color_map);
    }
    auto pos = SendMessageW(control, TBM_GETPOS, 0, 0);
    SendMessageW(control, TBM_SETPOS, FALSE, pos + 1);
    SendMessageW(control, TBM_SETPOS, TRUE, pos);
  }
}// namespace win32