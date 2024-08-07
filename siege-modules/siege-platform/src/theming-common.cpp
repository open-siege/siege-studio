#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/stream.hpp>

namespace win32
{
  gdi::brush_ref get_solid_brush(COLORREF color);

  void apply_theme(const win32::window_ref& colors, win32::header& control)
  {
    struct sub_class
    {
      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (uMsg == WM_DRAWITEM && lParam)
        {
          thread_local std::wstring buffer(256, '\0');
          DRAWITEMSTRUCT& item = *(DRAWITEMSTRUCT*)lParam;
          if (item.hwndItem == (HWND)uIdSubclass && (item.itemAction == ODA_DRAWENTIRE || item.itemAction == ODA_SELECT))
          {
            auto control = win32::header((HWND)uIdSubclass);


            auto context = win32::gdi::drawing_context_ref(item.hDC);
            auto header = win32::header(item.hwndItem);

            auto rect = item.rcItem;
            auto bottom = item.rcItem;

            if (item.itemAction == ODA_DRAWENTIRE)
            {
              auto bk_color = control.FindPropertyExW<COLORREF>(win32::properties::header::bk_color);

              context.FillRect(rect, get_solid_brush(*bk_color));
            }

            if (header.GetWindowStyle() & HDS_FILTERBAR)
            {
              rect.bottom = rect.bottom / 2;
              bottom.top = rect.bottom;
            }

            SetBkMode(context, TRANSPARENT);

            auto text_highlight_color = control.FindPropertyExW<COLORREF>(win32::properties::header::text_highlight_color);
            auto text_bk_color = control.FindPropertyExW<COLORREF>(win32::properties::header::text_bk_color);

            if (item.itemState & ODS_HOTLIGHT)
            {
              context.FillRect(rect, get_solid_brush(*text_highlight_color));
            }
            else if (item.itemState & ODS_SELECTED)
            {
              context.FillRect(rect, get_solid_brush(*text_highlight_color));
            }
            else
            {
              context.FillRect(rect, get_solid_brush(*text_bk_color));
            }

            auto text_color = control.FindPropertyExW<COLORREF>(win32::properties::header::text_color);
            ::SetTextColor(context, *text_color);

            auto item_info = header.GetItem(item.itemID, HDITEMW{ .mask = HDI_TEXT, .pszText = buffer.data(), .cchTextMax = int(buffer.size()) });

            if (item_info)
            {
              ::DrawTextW(context, (LPCWSTR)item_info->pszText, -1, &rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
            }

            if (header.GetWindowStyle() & HDS_FILTERBAR)
            {
              thread_local std::wstring filter_value;
              filter_value.clear();
              filter_value.resize(255, L'\0');
              HD_TEXTFILTERW string_filter{
                .pszText = filter_value.data(),
                .cchTextMax = (int)filter_value.size(),
              };

              auto header_item = header.GetItem(item.itemID, { .mask = HDI_FILTER, .type = HDFT_ISSTRING, .pvFilter = &string_filter });

              filter_value.erase(std::wcslen(filter_value.data()));

              if (filter_value.empty())
              {
                ::DrawTextW(context, L"Enter text here", -1, &bottom, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
              }
              else
              {
                ::DrawTextW(context, filter_value.c_str(), -1, &bottom, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
              }
            }

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

    auto count = control.GetItemCount();

    for (auto i = 0; i < count; ++i)
    {
      auto item = control.GetItem(
        i, HDITEMW{ .mask = HDI_FORMAT });

      if (item)
      {
        if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
        {
          item->fmt = item->fmt | HDF_OWNERDRAW;
        }
        else
        {
          item->fmt = item->fmt & ~HDF_OWNERDRAW;
        }

        control.SetItem(i, *item);
      }
    }

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      colors.ForEachPropertyExW([&](auto, auto key, auto value) {
        if (key.find(win32::header::class_name) != std::wstring_view::npos)
        {
          control.SetPropW(key, value);
        }
      });

      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
    }
    else
    {
      control.RemovePropW(win32::properties::header::bk_color);
      control.RemovePropW(win32::properties::header::text_color);
      control.RemovePropW(win32::properties::header::text_bk_color);
      control.RemovePropW(win32::properties::header::text_highlight_color);
      ::RemoveWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get());
    }
  }

  void apply_theme(const win32::window_ref& colors, win32::tab_control& control)
  {
    struct sub_class
    {
      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (uMsg == WM_MEASUREITEM && lParam)
        {
          MEASUREITEMSTRUCT& item = *(MEASUREITEMSTRUCT*)lParam;

          if (item.CtlType == ODT_TAB)
          {
            auto control = win32::tab_control((HWND)uIdSubclass);

            auto size = control.SetItemSize(SIZE{ .cx = 90, .cy = 30 });
            control.SetItemSize(size);

            item.itemWidth = size.cx;
            item.itemHeight = size.cy;
            return TRUE;
          }
        }

        if (uMsg == WM_DRAWITEM && lParam)
        {
          thread_local std::wstring buffer(256, '\0');
          DRAWITEMSTRUCT& item = *(DRAWITEMSTRUCT*)lParam;
          if (item.hwndItem == (HWND)uIdSubclass && (item.itemAction == ODA_DRAWENTIRE || item.itemAction == ODA_SELECT))
          {
            if (item.itemAction == ODA_DRAWENTIRE)
            {
              auto parent_context = win32::gdi::drawing_context_ref(::GetDC(item.hwndItem));

              auto tabs = win32::tab_control(item.hwndItem);
              auto rect = tabs.GetClientRect();

              auto count = tabs.GetItemCount();

              if (count > 0)
              {
                auto tab_rect = tabs.GetItemRect(count - 1);
                rect->left = tab_rect->right;
                rect->bottom = tab_rect->bottom;
              }

              auto bk_color = tabs.FindPropertyExW<COLORREF>(win32::properties::tab_control::bk_color);

              parent_context.FillRect(*rect, get_solid_brush(*bk_color));
            }

            auto context = win32::gdi::drawing_context_ref(item.hDC);

            SetBkMode(context, TRANSPARENT);

            win32::tab_control control(item.hwndItem);
            auto text_highlight_color = control.FindPropertyExW<COLORREF>(win32::properties::tab_control::text_highlight_color);
            auto text_bk_color = control.FindPropertyExW<COLORREF>(win32::properties::tab_control::text_bk_color);

            if (item.itemState & ODS_HOTLIGHT)
            {
              context.FillRect(item.rcItem, get_solid_brush(*text_highlight_color));
            }
            else if (item.itemState & ODS_SELECTED)
            {
              context.FillRect(item.rcItem, get_solid_brush(*text_highlight_color));
            }
            else
            {
              context.FillRect(item.rcItem, get_solid_brush(*text_bk_color));
            }

            auto text_color = control.FindPropertyExW<COLORREF>(win32::properties::tab_control::text_color);
            ::SetTextColor(context, *text_color);

            auto item_info = control.GetItem(item.itemID, TCITEMW{ .mask = TCIF_TEXT, .pszText = buffer.data(), .cchTextMax = int(buffer.size()) });

            if (item_info)
            {
              ::DrawTextW(context, (LPCWSTR)item_info->pszText, -1, &item.rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            }
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

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      auto style = control.GetWindowStyle();
      control.SetWindowStyle(style | TCS_OWNERDRAWFIXED);

      colors.ForEachPropertyExW([&](auto, auto key, auto value) {
        if (key.find(win32::tab_control::class_name) != std::wstring_view::npos)
        {
          control.SetPropW(key, value);
        }
      });

      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
    }
    else
    {
      auto style = control.GetWindowStyle();
      control.SetWindowStyle(style & ~TCS_OWNERDRAWFIXED);

      control.RemovePropW(win32::properties::tab_control::bk_color);
      control.RemovePropW(win32::properties::tab_control::text_color);
      control.RemovePropW(win32::properties::tab_control::text_bk_color);
      control.RemovePropW(win32::properties::tab_control::text_highlight_color);
      ::RemoveWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get());
    }
  }

  void apply_theme(const win32::window_ref& colors, win32::list_view& control)
  {
    static auto default_bk_color = ListView_GetBkColor(control);
    auto color = colors.FindPropertyExW<COLORREF>(properties::list_view::bk_color).value_or(default_bk_color);
    ListView_SetBkColor(control, color);

    static auto default_text_color = ListView_GetTextColor(control);
    color = colors.FindPropertyExW<COLORREF>(properties::list_view::text_color).value_or(default_text_color);
    ListView_SetTextColor(control, color);

    static auto default_text_bk_color = ListView_GetTextBkColor(control);
    color = colors.FindPropertyExW<COLORREF>(properties::list_view::text_bk_color).value_or(default_text_bk_color);
    ListView_SetTextBkColor(control, color);

    static auto default_outline_color = ListView_GetOutlineColor(control);
    color = colors.FindPropertyExW<COLORREF>(properties::list_view::outline_color).value_or(default_outline_color);
    ListView_SetOutlineColor(control, color);

    auto header = control.GetHeader();

    if (header)
    {
      win32::apply_theme(colors, header);
    }

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      win32::theme_module().SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
    }

    ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
  }

  void apply_theme(const win32::window_ref& colors, win32::tree_view& control)
  {
    auto color = colors.FindPropertyExW<COLORREF>(properties::tree_view::bk_color).value_or(CLR_NONE);
    TreeView_SetBkColor(control, color);

    color = colors.FindPropertyExW<COLORREF>(properties::tree_view::text_color).value_or(CLR_NONE);
    TreeView_SetTextColor(control, color);

    color = colors.FindPropertyExW<COLORREF>(properties::tree_view::line_color).value_or(CLR_NONE);
    TreeView_SetLineColor(control, color);

    auto font = win32::load_font(LOGFONTW{
      .lfPitchAndFamily = VARIABLE_PITCH,
      .lfFaceName = L"Segoe UI" });

    SendMessageW(control, WM_SETFONT, (WPARAM)font.get(), FALSE);


    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      win32::theme_module().SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
    }
    ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
  }

  void apply_theme(const win32::window_ref& colors, win32::tool_bar& control)
  {
    auto highlight_color = colors.FindPropertyExW(properties::tool_bar::btn_highlight_color);
    auto shadow_color = colors.FindPropertyExW(properties::tool_bar::btn_shadow_color);

    bool change_theme = false;

    if (highlight_color || shadow_color)
    {
      change_theme = true;
      COLORSCHEME scheme{ .dwSize = sizeof(COLORSCHEME), .clrBtnHighlight = CLR_DEFAULT, .clrBtnShadow = CLR_DEFAULT };

      if (highlight_color)
      {
        scheme.clrBtnHighlight = (COLORREF)*highlight_color;
      }

      if (shadow_color)
      {
        scheme.clrBtnShadow = (COLORREF)*shadow_color;
      }

      ::SendMessageW(control, TB_SETCOLORSCHEME, 0, (LPARAM)&scheme);
    }

    struct sub_class
    {


      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (uMsg == WM_NOTIFY && lParam != 0)
        {
          NMHDR* header = (NMHDR*)lParam;

          if (header->code == NM_CUSTOMDRAW && header->hwndFrom == (HWND)uIdSubclass)
          {
            NMTBCUSTOMDRAW* custom_draw = (NMTBCUSTOMDRAW*)lParam;

            if (custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
            {
              auto font = win32::load_font(LOGFONTW{
                .lfPitchAndFamily = VARIABLE_PITCH,
                .lfFaceName = L"Segoe UI" });

              SelectFont(custom_draw->nmcd.hdc, font);
              return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
            }

            if (custom_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT || custom_draw->nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
            {
              win32::window_ref(header->hwndFrom).ForEachPropertyExW([=](win32::hwnd_t, std::wstring_view key, HANDLE value) {
                if (key == properties::tool_bar::btn_face_color)
                {
                  custom_draw->clrBtnFace = (COLORREF)value;
                }

                if (key == properties::tool_bar::btn_highlight_color)
                {
                  custom_draw->clrBtnHighlight = (COLORREF)value;
                }

                if (key == properties::tool_bar::text_color)
                {
                  custom_draw->clrText = (COLORREF)value;
                }
              });
              return CDRF_NEWFONT | TBCDRF_USECDCOLORS;
            }

            if (custom_draw->nmcd.dwDrawStage == CDDS_POSTPAINT)
            {
              win32::gdi::drawing_context_ref context(custom_draw->nmcd.hdc);

              auto buttons = win32::tool_bar(custom_draw->nmcd.hdr.hwndFrom);
              auto count = buttons.ButtonCount();
              auto rect = buttons.GetClientRect();
              if (count > 0)
              {
                auto button_rect = buttons.GetItemRect(count - 1);

                rect->left = button_rect->right;
                rect->bottom = button_rect->bottom;
                auto bk_color = buttons.FindPropertyExW<COLORREF>(properties::tool_bar::bk_color);

                context.FillRect(*rect, get_solid_brush(*bk_color));
              }
            }

            return CDRF_DODEFAULT;
          }
        }

        if (uMsg == WM_DESTROY)
        {
          ::RemoveWindowSubclass(hWnd, sub_class::HandleMessage, uIdSubclass);
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
      }
    };

    if (colors.FindPropertyExW(properties::tool_bar::btn_face_color) || colors.FindPropertyExW(properties::tool_bar::text_color))
    {
      change_theme = true;

      colors.ForEachPropertyExW([&](auto, auto key, auto value) {
        if (key.find(win32::tool_bar::class_name) != std::wstring_view::npos)
        {
          control.SetPropW(key, value);
        }
      });
    }

    if (change_theme)
    {
      ::SetWindowSubclass(
        *control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
      win32::theme_module().SetWindowTheme(control, L"", L"");
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
      ::RemoveWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get());
    }
    ::RedrawWindow(control, nullptr, nullptr, RDW_INVALIDATE);
  }

}// namespace win32