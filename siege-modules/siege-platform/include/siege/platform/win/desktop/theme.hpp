#ifndef OPEN_SIEGE_THEME_MODULE_HPP
#define OPEN_SIEGE_THEME_MODULE_HPP

#include <siege/platform/win/core/module.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <map>
#include <string>
#include <uxtheme.h>

namespace win32
{
  struct theme_module : private win32::module
  {
    theme_module() : win32::module("uxtheme.dll", true)
    {
      set_window_theme = this->GetProcAddress<decltype(set_window_theme)>("SetWindowTheme");

      if (set_window_theme == nullptr)
      {
        throw std::exception("Could not load theme window functions");
      }
    }

    HRESULT SetWindowTheme(HWND window, const wchar_t* app_name, const wchar_t* id)
    {
      return set_window_theme(window, app_name, id);
    }

  private:
    std::add_pointer_t<decltype(::SetWindowTheme)> set_window_theme;
  };


  struct properties
  {
    struct tree_view
    {
      inline static auto bk_color = std::wstring(win32::tree_view::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::tree_view::class_name) + L"." + L"TextColor";
      inline static auto line_color = std::wstring(win32::tree_view::class_name) + L"." + L"LineColor";
    };

    struct list_view
    {
      inline static auto bk_color = std::wstring(win32::list_view::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::list_view::class_name) + L"." + L"TextColor";
      inline static auto text_bk_color = std::wstring(win32::list_view::class_name) + L"." + L"TextBkColor";
      inline static auto outline_color = std::wstring(win32::list_view::class_name) + L"." + L"OutlineColor";
    };

    struct tool_bar
    {
      inline static auto btn_highlight_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BtnHighlightColor";
      inline static auto btn_shadow_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BtnShadowColor";
      inline static auto btn_face_color = std::wstring(win32::tool_bar::class_name) + L"." + L"BtnFaceColor";
      inline static auto text_color = std::wstring(win32::tool_bar::class_name) + L"." + L"TextColor";
      inline static auto text_highlight_color = std::wstring(win32::tool_bar::class_name) + L"." + L"TextHighlightColor";
      inline static auto mark_color = std::wstring(win32::tool_bar::class_name) + L"." + L"MarkColor";
    };

    struct list_box
    {
      inline static auto bk_color = std::wstring(win32::list_box::class_name) + L"." + L"BkColor";
      inline static auto text_color = std::wstring(win32::list_box::class_name) + L"." + L"TextColor";
      inline static auto highlight_color = std::wstring(win32::list_box::class_name) + L"." + L"HighlightColor";
    };

    struct menu
    {
      constexpr static auto bk_color = std::wstring_view(L"Menu.BkColor");
    };

    struct window
    {
      constexpr static auto bk_color = std::wstring_view(L"Window.BkColor");
    };
  };

  inline void apply_theme(const win32::window_ref& colors, win32::header& control)
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
            static auto black_brush = ::CreateSolidBrush(0x00000000);
            static auto white_brush = ::CreateSolidBrush(0x00FFFFFF);
            static auto grey_brush = ::CreateSolidBrush(0x00383838);

            auto context = win32::gdi_drawing_context_ref(item.hDC);
            auto header = win32::header(item.hwndItem);

            auto rect = item.rcItem;
            auto bottom = item.rcItem;

            if (item.itemAction == ODA_DRAWENTIRE)
            {
              context.FillRect(rect, grey_brush);
            }

            if (header.GetWindowStyle() & HDS_FILTERBAR)
            {
              rect.bottom = rect.bottom / 2;
              bottom.top = rect.bottom;
            }

            SetBkMode(context, TRANSPARENT);

            if (item.itemState & ODS_HOTLIGHT)
            {
              context.FillRect(rect, grey_brush);
            }
            else if (item.itemState & ODS_SELECTED)
            {
              context.FillRect(rect, grey_brush);
            }
            else
            {
              context.FillRect(rect, black_brush);
            }

            ::SetTextColor(context, 0x00FFFFFF);

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

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
    }
    else
    {
      ::RemoveWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get());
    }
  }

  inline void apply_theme(const win32::window_ref& colors, win32::tab_control& control)
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
            static auto black_brush = ::CreateSolidBrush(0x00000000);
            static auto grey_brush = ::CreateSolidBrush(0x00383838);

            if (item.itemAction == ODA_DRAWENTIRE)
            {
              auto parent_context = win32::gdi_drawing_context_ref(::GetDC(item.hwndItem));
              
              auto tabs = win32::tab_control(item.hwndItem);
              auto rect = tabs.GetClientRect();

              auto count = tabs.GetItemCount();

              if (count > 0)
              {
                auto tab_rect = tabs.GetItemRect(count - 1);
                rect->left = tab_rect->right;
                rect->bottom = tab_rect->bottom;
              }
                
              parent_context.FillRect(*rect, grey_brush);
            }

            auto context = win32::gdi_drawing_context_ref(item.hDC);

            SetBkMode(context, TRANSPARENT);

            if (item.itemState & ODS_HOTLIGHT)
            {
              context.FillRect(item.rcItem, grey_brush);
            }
            else if (item.itemState & ODS_SELECTED)
            {
              context.FillRect(item.rcItem, grey_brush);
            }
            else
            {
              context.FillRect(item.rcItem, black_brush);
            }

            ::SetTextColor(context, 0x00FFFFFF);

            auto item_info = win32::tab_control(item.hwndItem).GetItem(item.itemID, TCITEMW{ .mask = TCIF_TEXT, .pszText = buffer.data(), .cchTextMax = int(buffer.size()) });

            if (item_info)
            {
              ::DrawTextW(context, (LPCWSTR)item_info->pszText, -1, &item.rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            }
            return TRUE;
          }
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
      }
    };

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      auto style = control.GetWindowStyle();
      control.SetWindowStyle(style | TCS_OWNERDRAWFIXED);
      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
    }
    else
    {
      auto style = control.GetWindowStyle();
      control.SetWindowStyle(style & ~TCS_OWNERDRAWFIXED);
      ::RemoveWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get());
    }
  }

  inline void apply_theme(const win32::window_ref& colors, win32::list_box& control)
  {
    auto bk_color = colors.FindPropertyExW<COLORREF>(properties::list_box::bk_color);
    auto text_color = colors.FindPropertyExW<COLORREF>(properties::list_box::text_color);
    auto highlight_color = colors.FindPropertyExW<COLORREF>(properties::list_box::highlight_color);

    struct sub_class
    {
      static LRESULT __stdcall HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
      {
        if (uMsg == WM_CTLCOLORLISTBOX && lParam == uIdSubclass)
        {
          // win32::gdi_brush list_background;
          //  if (list_background)
          //{
          //  return (LRESULT)list_background.get();
          //}
        }

        if (uMsg == WM_MEASUREITEM && lParam)
        {
          MEASUREITEMSTRUCT& item = *(MEASUREITEMSTRUCT*)lParam;

          if (item.CtlType == ODT_LISTBOX)
          {
            auto themed_selection = win32::list_box((HWND)uIdSubclass);

            item.itemHeight = themed_selection.GetItemHeight(item.itemID);
          }
        }

        if (uMsg == WM_DRAWITEM && lParam)
        {
          thread_local std::wstring buffer;
          DRAWITEMSTRUCT& item = *(DRAWITEMSTRUCT*)lParam;
          if (item.hwndItem == (HWND)uIdSubclass && (item.itemAction == ODA_DRAWENTIRE || item.itemAction == ODA_SELECT))
          {
            static auto black_brush = ::CreateSolidBrush(0x00000000);
            static auto grey_brush = ::CreateSolidBrush(0x00383838);
            auto context = win32::gdi_drawing_context_ref(item.hDC);

            context.FillRect(item.rcItem, item.itemState & ODS_SELECTED ? grey_brush : black_brush);
            ::SetTextColor(context, 0x00FFFFFF);

            auto themed_selection = win32::list_box(item.hwndItem);
            buffer.resize(themed_selection.GetTextLength(item.itemID));

            themed_selection.GetText(item.itemID, buffer.data());

            ::TextOut(context, item.rcItem.left, item.rcItem.top, buffer.c_str(), buffer.size());
          }
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

      ::SetWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
    }
    else
    {
      auto style = control.GetWindowStyle();
      copy_control(style & ~LBS_OWNERDRAWFIXED);
      ::RemoveWindowSubclass(*control.GetParent(), sub_class::HandleMessage, (UINT_PTR)control.get());
    }
  }


  inline void apply_theme(const win32::window_ref& colors, win32::list_view& control)
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

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      win32::theme_module().SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
    }
  }

  inline void apply_theme(const win32::window_ref& colors, win32::tree_view& control)
  {
    auto color = colors.FindPropertyExW<COLORREF>(properties::tree_view::bk_color).value_or(CLR_NONE);
    TreeView_SetBkColor(control, color);

    color = colors.FindPropertyExW<COLORREF>(properties::tree_view::text_color).value_or(CLR_NONE);
    TreeView_SetTextColor(control, color);

    color = colors.FindPropertyExW<COLORREF>(properties::tree_view::line_color).value_or(CLR_NONE);
    TreeView_SetLineColor(control, color);

    if (colors.GetPropW<bool>(L"AppsUseDarkTheme"))
    {
      win32::theme_module().SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
    }
  }

  inline void apply_theme(const win32::window_ref& colors, win32::tool_bar& control)
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

    static auto change_color = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) -> LRESULT __stdcall
    {
      // TODO remove this sub class on destroy
      if (uMsg == WM_NOTIFY && lParam != 0)
      {
        NMHDR* header = (NMHDR*)lParam;

        if (header->code == NM_CUSTOMDRAW && header->hwndFrom == (HWND)uIdSubclass)
        {
          NMTBCUSTOMDRAW* custom_draw = (NMTBCUSTOMDRAW*)lParam;

          if (custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
          {
            return CDRF_NOTIFYITEMDRAW;
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

          return CDRF_DODEFAULT;
        }
      }

      return DefSubclassProc(hWnd, uMsg, wParam, lParam);
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
        *control.GetParent(), change_color, (UINT_PTR)control.get(), (DWORD_PTR)control.get());
      win32::theme_module().SetWindowTheme(control, L"", L"");
    }
    else
    {
      win32::theme_module().SetWindowTheme(control, nullptr, nullptr);
      ::RemoveWindowSubclass(*control.GetParent(), change_color, (UINT_PTR)control.get());
    }
  }

}// namespace win32

#endif