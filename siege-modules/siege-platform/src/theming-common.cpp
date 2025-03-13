#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <siege/platform/stream.hpp>
#include <unordered_set>

namespace win32
{
  gdi::brush_ref get_solid_brush(COLORREF color);
  bool is_parent_from_system(HWND parent);

  struct superclass_handle
  {
    HWND dummy = nullptr;
    std::add_pointer_t<decltype(DefWindowProcW)> control_proc = DefWindowProcW;

    void init(const wchar_t* class_name, LONG_PTR sub_class)
    {
      dummy = ::CreateWindowExW(0, class_name, L"Dummy", 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
      control_proc = (decltype(control_proc))::SetClassLongPtrW(dummy, GCLP_WNDPROC, sub_class);
    }

    void dispose(std::unordered_set<HWND>& controls, LONG_PTR sub_class)
    {
      if (!dummy)
      {
        return;
      }

      for (auto control : controls)
      {
        auto existing_proc = ::GetWindowLongPtrW(control, GWLP_WNDPROC);

        if (existing_proc == sub_class)
        {
          ::SetWindowLongW(control, GWLP_WNDPROC, (LONG_PTR)control_proc);
        }
      }

      ::SetClassLongPtrW(dummy, GCLP_WNDPROC, (LONG_PTR)control_proc);
      ::DestroyWindow(dummy);
      dummy = nullptr;
    }

    ~superclass_handle()
    {
      if (dummy)
      {
        ::SetClassLongPtrW(dummy, GCLP_WNDPROC, (LONG_PTR)control_proc);
        ::DestroyWindow(dummy);
        dummy = nullptr;
      }
    }
  };

  struct root_window_handler
  {
    static LRESULT __stdcall handle_root_message(HWND root, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
      if (dwRefData && message == WM_SETTINGCHANGE && lParam && std::wstring_view((wchar_t*)lParam) == L"ImmersiveColorSet")
      {
        auto result = DefSubclassProc(root, message, wParam, lParam);
        auto& controls = *(std::unordered_set<HWND>*)dwRefData;

        for (auto& control : controls)
        {
          SendMessage(control, WM_SETTINGCHANGE, lParam, wParam);
          SendMessage(control, WM_THEMECHANGED, 0, 0);
        }

        return result;
      }

      if (message == WM_NCDESTROY)
      {
        ::RemoveWindowSubclass(root, handle_root_message, 0);
      }

      return DefSubclassProc(root, message, wParam, lParam);
    }
  };

  void apply_header_theme(bool remove)
  {
    static superclass_handle superclass;
    static std::unordered_set<HWND> controls;

    struct handlers
    {
      static LRESULT __stdcall handle_control_message(HWND self, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if (message == WM_NCCREATE)
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;

          if (!create_params)
          {
            return FALSE;
          }

          if (!create_params->hwndParent || is_parent_from_system(create_params->hwndParent))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          auto root = ::GetAncestor(create_params->hwndParent, GA_ROOT);
          auto parent = create_params->hwndParent;
          DWORD_PTR existing_object{};

          if (!::GetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, &existing_object))
          {
            ::SetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, (DWORD_PTR)&controls);
          }

          if (!::GetWindowSubclass(parent, handle_parent_message, 0, &existing_object))
          {
            ::SetWindowSubclass(parent, handle_parent_message, 0, 0);
          }
          controls.emplace(self);
          return superclass.control_proc(self, message, wParam, lParam);
        }

        if (message == WM_CREATE && controls.contains(self))
        {
          auto result = superclass.control_proc(self, message, wParam, lParam);

          auto best_font = get_best_system_font();

          if (best_font)
          {
            auto font = win32::load_font(LOGFONTW{
                                           .lfPitchAndFamily = VARIABLE_PITCH },
              *best_font);
            SendMessageW(self, WM_SETFONT, (WPARAM)font.get(), FALSE);
          }

          ::SendMessageW(self, CCM_DPISCALE, TRUE, 0);
          win32::theme_module().SetWindowTheme(self, L"", L"");
          return result;
        }

        if (message == WM_NCDESTROY && controls.contains(self))
        {
          RemovePropW(self, L"PreviousWidth");
          RemovePropW(self, L"PreviousHeight");
          controls.erase(self);
        }

        return superclass.control_proc(self, message, wParam, lParam);
      }

      static LRESULT __stdcall handle_parent_message(HWND parent, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_NOTIFY && ((NMHDR*)lParam)->code == NM_CUSTOMDRAW && controls.contains(((NMHDR*)lParam)->hwndFrom))
        {
          auto& custom_draw = *(NMCUSTOMDRAW*)lParam;
          if (custom_draw.dwDrawStage == CDDS_PREPAINT)
          {
            auto text_bk_color = get_color_for_window(window_ref(custom_draw.hdr.hwndFrom), properties::header::text_bk_color);
            FillRect(custom_draw.hdc, &custom_draw.rc, get_solid_brush(text_bk_color));

            return CDRF_NOTIFYITEMDRAW | CDRF_NEWFONT;
          }


          if (custom_draw.dwDrawStage == CDDS_ITEMPREPAINT)
          {
            auto focused_item = Header_GetFocusedItem(custom_draw.hdr.hwndFrom);

            auto text_highlight_color = get_color_for_window(window_ref(custom_draw.hdr.hwndFrom), properties::header::text_highlight_color);
            auto text_bk_color = get_color_for_window(window_ref(custom_draw.hdr.hwndFrom), properties::header::text_bk_color);

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

            auto text_color = get_color_for_window(window_ref(custom_draw.hdr.hwndFrom), properties::header::text_color);
            ::SetTextColor(custom_draw.hdc, text_color);

            return CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
          }

          if (custom_draw.dwDrawStage == CDDS_ITEMPOSTPAINT)
          {
            auto rect = custom_draw.rc;

            if (::GetWindowLongPtrW(custom_draw.hdr.hwndFrom, GWL_STYLE) & HDS_FILTERBAR)
            {
              thread_local std::wstring filter_value;
              filter_value.clear();
              filter_value.resize(255, L'\0');
              HD_TEXTFILTERW string_filter{
                .pszText = filter_value.data(),
                .cchTextMax = (int)filter_value.size(),
              };

              win32::gdi::drawing_context_ref context(custom_draw.hdc);
              auto header_item = header(custom_draw.hdr.hwndFrom).GetItem(custom_draw.dwItemSpec, { .mask = HDI_FILTER, .type = HDFT_ISSTRING, .pvFilter = &string_filter });

              filter_value.erase(std::wcslen(filter_value.data()));


              auto bottom = custom_draw.rc;

              rect.bottom = rect.bottom / 2;
              bottom.top = rect.bottom;
              auto text_bk_color = get_color_for_window(window_ref(custom_draw.hdr.hwndFrom), properties::header::text_bk_color);
              SetDCBrushColor(custom_draw.hdc, text_bk_color);

              FillRect(custom_draw.hdc, &bottom, (HBRUSH)GetStockObject(DC_BRUSH));

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


        if (message == WM_NCDESTROY)
        {
          ::RemoveWindowSubclass(parent, handle_parent_message, 0);
        }

        return DefSubclassProc(parent, message, wParam, lParam);
      }
    };

    if (!superclass.dummy)
    {
      superclass.init(WC_HEADERW, (LONG_PTR)handlers::handle_control_message);
    }

    if (superclass.dummy && remove)
    {
      superclass.dispose(controls, (LONG_PTR)handlers::handle_control_message);
    }
  }

  void apply_tab_control_theme(bool remove)
  {
    static superclass_handle superclass;
    static std::unordered_set<HWND> controls;

    struct handlers
    {
      static LRESULT __stdcall handle_control_message(HWND self, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if (message == WM_NCCREATE)
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;

          if (!create_params)
          {
            return FALSE;
          }

          if (!create_params->hwndParent || is_parent_from_system(create_params->hwndParent))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          if (create_params->style & TCS_OWNERDRAWFIXED)
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          create_params->style = create_params->style | TCS_OWNERDRAWFIXED;

          auto root = ::GetAncestor(create_params->hwndParent, GA_ROOT);
          auto parent = create_params->hwndParent;
          DWORD_PTR existing_object{};

          if (!::GetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, &existing_object))
          {
            ::SetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, (DWORD_PTR)&controls);
          }

          if (!::GetWindowSubclass(parent, handle_parent_message, 0, &existing_object))
          {
            ::SetWindowSubclass(parent, handle_parent_message, 0, 0);
          }
          controls.emplace(self);
          return superclass.control_proc(self, message, wParam, lParam);
        }

        if (message == WM_CREATE && controls.contains(self))
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;
          create_params->style = create_params->style | TCS_OWNERDRAWFIXED;
          auto result = superclass.control_proc(self, message, wParam, lParam);

          auto best_font = get_best_system_font();

          if (best_font)
          {
            auto font = win32::load_font(LOGFONTW{
                                           .lfPitchAndFamily = VARIABLE_PITCH },
              *best_font);
            SendMessageW(self, WM_SETFONT, (WPARAM)font.get(), FALSE);
          }
          return result;
        }

        if (message == TCM_SETPADDING && controls.contains(self))
        {
          ::SetPropW(self, L"PaddingX", (HANDLE)LOWORD(lParam));
          ::SetPropW(self, L"PaddingY", (HANDLE)HIWORD(lParam));
        }

        if ((message == WM_PAINT || message == WM_PRINTCLIENT) && controls.contains(self))
        {
          PAINTSTRUCT ps{};
          HDC hdc = wParam != 0 ? (HDC)wParam : message == WM_PAINT ? BeginPaint(self, &ps)
                                                                    : nullptr;

          if (wParam)
          {
            ::GetClientRect(self, &ps.rcPaint);
          }

          if (!hdc)
          {
            return 0;
          }

          auto tabs = win32::tab_control(self);
          auto parent_context = win32::gdi::drawing_context_ref(hdc);
          auto rect = tabs.GetClientRect();
          auto bk_color = get_color_for_window(tabs.ref(), win32::properties::tab_control::bk_color);

          if (ps.fErase)
          {
            parent_context.FillRect(*rect, get_solid_brush(bk_color));
          }

          auto result = superclass.control_proc(self, message, (WPARAM)hdc, lParam);

          auto text_bk_color = get_color_for_window(tabs.ref(), win32::properties::tab_control::text_bk_color);
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
          for (auto i = 0; i < TabCtrl_GetItemCount(self); ++i)
          {
            TabCtrl_GetItemRect(self, i, &item_rect);
            Rectangle(hdc, item_rect.left, item_rect.top, item_rect.right, item_rect.bottom);
          }

          SelectObject(hdc, old_pen);
          DeleteObject(pen);

          if (wParam == 0 && message == WM_PAINT)
          {
            EndPaint(self, &ps);
          }

          auto up_down = win32::window_ref(::FindWindowExW(self, nullptr, UPDOWN_CLASSW, nullptr));

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

        if (message == WM_NCDESTROY && controls.contains(self))
        {
          ::RemovePropW(self, L"PaddingX");
          ::RemovePropW(self, L"PaddingY");
          controls.erase(self);
        }


        return superclass.control_proc(self, message, wParam, lParam);
      }

      static LRESULT __stdcall handle_parent_message(HWND parent, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_DRAWITEM && controls.contains(((DRAWITEMSTRUCT*)lParam)->hwndItem))
        {
          auto& item = *((DRAWITEMSTRUCT*)lParam);

          thread_local std::wstring buffer(256, '\0');

          if (item.itemAction == ODA_DRAWENTIRE || item.itemAction == ODA_SELECT)
          {
            auto context = win32::gdi::drawing_context_ref(item.hDC);
            win32::tab_control control(item.hwndItem);

            SetBkColor(context, get_color_for_window(control.ref(), win32::properties::tab_control::bk_color));

            auto item_rect = item.rcItem;

            auto text_highlight_color = get_color_for_window(control.ref(), win32::properties::tab_control::text_highlight_color);
            auto text_bk_color = get_color_for_window(control.ref(), win32::properties::tab_control::text_bk_color);

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

            auto text_color = get_color_for_window(control.ref(), win32::properties::tab_control::text_color);
            ::SetTextColor(context, text_color);
            ::SetBkMode(context, TRANSPARENT);
            ::SelectObject(context, get_solid_brush(text_bk_color));

            auto item_info = control.GetItem(item.itemID, TCITEMW{ .mask = TCIF_TEXT, .pszText = buffer.data(), .cchTextMax = int(buffer.size()) });

            if (item_info)
            {
              auto real_rect = item.rcItem;

              auto padding_x = (WORD)::GetPropW(control, L"PaddingX");
              auto padding_y = (WORD)::GetPropW(control, L"PaddingY");

              if (padding_x || padding_y)
              {
                real_rect.left += padding_x;
                real_rect.right -= padding_x;
                real_rect.top += padding_y;
                real_rect.bottom -= padding_y;
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

        if (message == WM_NCDESTROY)
        {
          ::RemoveWindowSubclass(parent, handle_parent_message, 0);
        }

        return DefSubclassProc(parent, message, wParam, lParam);
      }
    };

    if (!superclass.dummy)
    {
      superclass.init(WC_TABCONTROLW, (LONG_PTR)handlers::handle_control_message);
    }

    if (superclass.dummy && remove)
    {
      superclass.dispose(controls, (LONG_PTR)handlers::handle_control_message);
    }
  }

  void apply_list_view_theme(bool remove)
  {
    static superclass_handle superclass;
    static std::unordered_set<HWND> controls;

    struct handlers
    {
      static LRESULT __stdcall handle_control_message(HWND self, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if (message == WM_NCCREATE)
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;

          if (!create_params)
          {
            return FALSE;
          }

          if (!create_params->hwndParent || is_parent_from_system(create_params->hwndParent))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          if (create_params->style & LVS_OWNERDRAWFIXED)
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          auto root = ::GetAncestor(create_params->hwndParent, GA_ROOT);
          auto parent = create_params->hwndParent;
          DWORD_PTR existing_object{};
          if (!::GetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, &existing_object))
          {
            ::SetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, (DWORD_PTR)&controls);
          }

          if (!::GetWindowSubclass(parent, handle_parent_message, 0, &existing_object))
          {
            ::SetWindowSubclass(parent, handle_parent_message, 0, 0);
          }
          controls.emplace(self);
          return superclass.control_proc(self, message, wParam, lParam);
        }

        if (message == WM_CREATE && controls.contains(self))
        {
          auto result = superclass.control_proc(self, message, wParam, lParam);

          auto best_font = get_best_system_font();

          if (best_font)
          {
            auto font = win32::load_font(LOGFONTW{
                                           .lfPitchAndFamily = VARIABLE_PITCH },
              *best_font);
            SendMessageW(self, WM_SETFONT, (WPARAM)font.get(), FALSE);
          }

          SendMessageW(self, WM_SETTINGCHANGE, 0, 0);
          return result;
        }

        if (message == WM_NCDESTROY)
        {
          controls.erase(self);
        }

        if (message == WM_SETTINGCHANGE)
        {
          if (win32::is_dark_theme())
          {
            win32::theme_module().SetWindowTheme(self, L"DarkMode_Explorer", nullptr);
          }
          else
          {
            win32::theme_module().SetWindowTheme(self, L"Explorer", nullptr);
          }

          ListView_SetBkColor(self, win32::get_color_for_window(win32::window_ref(self), properties::list_view::bk_color));
          ListView_SetTextColor(self, win32::get_color_for_window(win32::window_ref(self), properties::list_view::text_color));
          ListView_SetTextBkColor(self, win32::get_color_for_window(win32::window_ref(self), properties::list_view::text_bk_color));
          ListView_SetOutlineColor(self, win32::get_color_for_window(win32::window_ref(self), properties::list_view::outline_color));
        }

        if (message == WM_WINDOWPOSCHANGING && controls.contains(self))
        {
          SCROLLINFO scroll{
            .cbSize = sizeof(SCROLLINFO),
            .fMask = SIF_ALL
          };
          GetScrollInfo(self, SB_VERT, &scroll);

          WINDOWPOS* rect = (WINDOWPOS*)lParam;
          auto width = rect->cx;

          if (scroll.nMax > 0)
          {
            auto scroll_width = win32::get_system_metrics(SM_CXEDGE) + win32::get_system_metrics(SM_CXEDGE) + win32::get_system_metrics(SM_CXVSCROLL);
            width = width > scroll_width ? width - scroll_width : width;
            rect->cx = width;
          }
        }

        return superclass.control_proc(self, message, wParam, lParam);
      }

      static LRESULT __stdcall handle_parent_message(HWND parent, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (message == WM_NOTIFY && ((NMHDR*)lParam)->code == NM_CUSTOMDRAW && controls.contains(((NMHDR*)lParam)->hwndFrom))
        {
          auto& custom_draw = *(NMLVCUSTOMDRAW*)lParam;

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

            auto control = custom_draw.nmcd.hdr.hwndFrom;

            if (ListView_GetGroupInfo(control, nGroupId, &lvg))
            {
              RECT header_rect{};
              ListView_GetGroupRect(control, nGroupId, LVGGR_HEADER, &header_rect);

              SetTextColor(custom_draw.nmcd.hdc, win32::get_color_for_window(win32::window_ref(control), properties::list_view::text_color));

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

              auto best_font = get_best_system_font();

              if (best_font)
              {
                auto font = win32::load_font(LOGFONTW{
                                               .lfPitchAndFamily = VARIABLE_PITCH },
                  *best_font);
                SelectFont(custom_draw.nmcd.hdc, font);
              }

              DrawTextExW(custom_draw.nmcd.hdc, group_name.data(), -1, &header_rect, DT_SINGLELINE | DT_LEFT | DT_VCENTER, nullptr);
            }

            return CDRF_SKIPDEFAULT;
          }
        }

        if (message == WM_NCDESTROY)
        {
          ::RemoveWindowSubclass(parent, handle_parent_message, 0);
        }

        return DefSubclassProc(parent, message, wParam, lParam);
      }
    };

    if (!superclass.dummy)
    {
      superclass.init(WC_LISTVIEWW, (LONG_PTR)handlers::handle_control_message);
    }

    if (superclass.dummy && remove)
    {
      superclass.dispose(controls, (LONG_PTR)handlers::handle_control_message);
    }
  }

  void apply_tree_view_theme(bool remove)
  {
    static superclass_handle superclass;
    static std::unordered_set<HWND> controls;

    struct handlers
    {
      static LRESULT __stdcall handle_control_message(HWND self, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if (message == WM_NCCREATE)
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;

          if (!create_params)
          {
            return FALSE;
          }

          if (!create_params->hwndParent || is_parent_from_system(create_params->hwndParent))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          auto root = ::GetAncestor(create_params->hwndParent, GA_ROOT);
          auto parent = create_params->hwndParent;
          DWORD_PTR existing_object{};

          if (!::GetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, &existing_object))
          {
            ::SetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, (DWORD_PTR)&controls);
          }

          controls.emplace(self);
          return superclass.control_proc(self, message, wParam, lParam);
        }

        if (message == WM_CREATE && controls.contains(self))
        {
          auto result = superclass.control_proc(self, message, wParam, lParam);

          auto best_font = get_best_system_font();

          if (best_font)
          {
            auto font = win32::load_font(LOGFONTW{
                                           .lfPitchAndFamily = VARIABLE_PITCH },
              *best_font);
            SendMessageW(self, WM_SETFONT, (WPARAM)font.get(), FALSE);
          }

          SendMessageW(self, WM_SETTINGCHANGE, 0, 0);
          return result;
        }

        if (message == WM_SETTINGCHANGE && controls.contains(self))
        {
          if (win32::is_dark_theme())
          {
            win32::theme_module().SetWindowTheme(self, L"DarkMode_Explorer", nullptr);
          }
          else
          {
            win32::theme_module().SetWindowTheme(self, L"Explorer", nullptr);
          }

          auto color = win32::get_color_for_window(window_ref(self), properties::tree_view::bk_color);
          TreeView_SetBkColor(self, color);

          color = win32::get_color_for_window(window_ref(self), properties::tree_view::text_color);
          TreeView_SetTextColor(self, color);

          color = win32::get_color_for_window(window_ref(self), properties::tree_view::line_color);
          TreeView_SetLineColor(self, color);
        }

        if (message == WM_NCDESTROY && controls.contains(self))
        {
          controls.erase(self);
        }


        return superclass.control_proc(self, message, wParam, lParam);
      }
    };

    if (!superclass.dummy)
    {
      superclass.init(WC_TREEVIEWW, (LONG_PTR)handlers::handle_control_message);
    }

    if (superclass.dummy && remove)
    {
      superclass.dispose(controls, (LONG_PTR)handlers::handle_control_message);
    }
  }

  void apply_tool_bar_theme(bool remove)
  {
    static superclass_handle superclass;
    static std::unordered_set<HWND> controls;

    struct handlers
    {
      static LRESULT __stdcall handle_control_message(HWND self, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if (message == WM_NCCREATE)
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;

          if (!create_params)
          {
            return FALSE;
          }

          if (!create_params->hwndParent || is_parent_from_system(create_params->hwndParent))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          auto root = ::GetAncestor(create_params->hwndParent, GA_ROOT);
          auto parent = create_params->hwndParent;
          DWORD_PTR existing_object{};

          if (!::GetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, &existing_object))
          {
            ::SetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, (DWORD_PTR)&controls);
          }

          if (!::GetWindowSubclass(parent, handle_parent_message, 0, &existing_object))
          {
            ::SetWindowSubclass(parent, handle_parent_message, 0, 0);
          }
          controls.emplace(self);
          return superclass.control_proc(self, message, wParam, lParam);
        }

        if (message == WM_CREATE && controls.contains(self))
        {
          win32::theme_module().SetWindowTheme(self, L"", L"");
          auto result = superclass.control_proc(self, message, wParam, lParam);

          auto best_font = get_best_system_font();

          if (best_font)
          {
            auto font = win32::load_font(LOGFONTW{
                                           .lfPitchAndFamily = VARIABLE_PITCH },
              *best_font);
            SendMessageW(self, WM_SETFONT, (WPARAM)font.get(), FALSE);
          }
          ::SendMessageW(self, CCM_DPISCALE, TRUE, 0);
          ::SendMessageW(self, WM_SETTINGCHANGE, 0, 0);

          return result;
        }

        if (message == WM_SETTINGCHANGE && controls.contains(self))
        {
          COLORSCHEME scheme{ .dwSize = sizeof(COLORSCHEME), .clrBtnHighlight = CLR_DEFAULT, .clrBtnShadow = CLR_DEFAULT };


          auto highlight_color = get_color_for_window(window_ref(self), properties::tool_bar::btn_highlight_color);
          auto shadow_color = get_color_for_window(window_ref(self), properties::tool_bar::btn_shadow_color);

          scheme.clrBtnHighlight = highlight_color;
          scheme.clrBtnShadow = shadow_color;

          ::SendMessageW(self, TB_SETCOLORSCHEME, 0, (LPARAM)&scheme);
        }

        if (message == WM_NCDESTROY && controls.contains(self))
        {
          controls.erase(self);
        }


        return superclass.control_proc(self, message, wParam, lParam);
      }

      static LRESULT __stdcall handle_parent_message(HWND parent, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {

        if (message == WM_NOTIFY && ((NMHDR*)lParam)->code == NM_CUSTOMDRAW && controls.contains(((NMHDR*)lParam)->hwndFrom))
        {
          auto& custom_draw = *(NMTBCUSTOMDRAW*)lParam;

          auto buttons = win32::tool_bar(custom_draw.nmcd.hdr.hwndFrom);

          if (custom_draw.nmcd.dwDrawStage == CDDS_PREPAINT)
          {
            auto best_font = get_best_system_font();

            if (best_font)
            {
              auto font = win32::load_font(LOGFONTW{
                                             .lfPitchAndFamily = VARIABLE_PITCH },
                *best_font);
              SelectFont(custom_draw.nmcd.hdc, font);
            }

            return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
          }

          if (custom_draw.nmcd.dwDrawStage == CDDS_ITEMPREPAINT || custom_draw.nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
          {
            custom_draw.clrBtnFace = get_color_for_window(buttons.ref(), properties::tool_bar::btn_face_color);
            custom_draw.clrBtnHighlight = get_color_for_window(buttons.ref(), properties::tool_bar::btn_highlight_color);
            custom_draw.clrText = get_color_for_window(buttons.ref(), properties::tool_bar::text_color);

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

              context.FillRect(*rect, get_solid_brush(get_color_for_window(buttons.ref(), properties::tool_bar::bk_color)));
            }
          }

          return CDRF_DODEFAULT;
        }

        if (message == WM_NCDESTROY)
        {
          ::RemoveWindowSubclass(parent, handle_parent_message, 0);
        }

        return DefSubclassProc(parent, message, wParam, lParam);
      }
    };

    if (!superclass.dummy)
    {
      superclass.init(TOOLBARCLASSNAMEW, (LONG_PTR)handlers::handle_control_message);
    }

    if (superclass.dummy && remove)
    {
      superclass.dispose(controls, (LONG_PTR)handlers::handle_control_message);
    }
  }

  void apply_tooltip_theme(bool remove)
  {
    static superclass_handle superclass;
    static std::unordered_set<HWND> controls;

    struct handlers
    {
      static LRESULT __stdcall handle_control_message(HWND self, UINT message, WPARAM wParam, LPARAM lParam)
      {
        if (message == WM_NCCREATE)
        {
          CREATESTRUCTW* create_params = (CREATESTRUCTW*)lParam;

          if (!create_params)
          {
            return FALSE;
          }

          if (!create_params->hwndParent || is_parent_from_system(create_params->hwndParent))
          {
            return superclass.control_proc(self, message, wParam, lParam);
          }

          auto root = ::GetAncestor(create_params->hwndParent, GA_ROOT);
          auto parent = create_params->hwndParent;
          DWORD_PTR existing_object{};

          if (!::GetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, &existing_object))
          {
            ::SetWindowSubclass(root, root_window_handler::handle_root_message, (UINT_PTR)&controls, (DWORD_PTR)&controls);
          }

          controls.emplace(self);
          return superclass.control_proc(self, message, wParam, lParam);
        }

        if (message == WM_CREATE && controls.contains(self))
        {
          win32::theme_module().SetWindowTheme(self, L"", L"");
          auto result = superclass.control_proc(self, message, wParam, lParam);

          auto best_font = get_best_system_font();

          if (best_font)
          {
            auto font = win32::load_font(LOGFONTW{
                                           .lfPitchAndFamily = VARIABLE_PITCH },
              *best_font);
            ::SendMessageW(self, WM_SETFONT, (WPARAM)font.get(), FALSE);
          }

          ::SendMessageW(self, CCM_DPISCALE, TRUE, 0);
          ::SendMessageW(self, WM_SETTINGCHANGE, 0, 0);

          return result;
        }

        if (message == WM_SETTINGCHANGE && controls.contains(self))
        {
          if (win32::is_dark_theme())
          {
            win32::theme_module().SetWindowTheme(self, L"DarkMode_Explorer", nullptr);
          }
          else
          {
            win32::theme_module().SetWindowTheme(self, L"Explorer", nullptr);
          }
        }

        if (message == WM_NCDESTROY && controls.contains(self))
        {
          controls.erase(self);
        }

        return superclass.control_proc(self, message, wParam, lParam);
      }
    };

    if (!superclass.dummy)
    {
      superclass.init(TOOLTIPS_CLASSW, (LONG_PTR)handlers::handle_control_message);
    }

    if (superclass.dummy && remove)
    {
      superclass.dispose(controls, (LONG_PTR)handlers::handle_control_message);
    }
  }
}// namespace win32