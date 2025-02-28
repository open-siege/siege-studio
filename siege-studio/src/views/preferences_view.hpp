#ifndef THEME_VIEW_HPP
#define THEME_VIEW_HPP

#include <siege/platform/win/window_factory.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/theming.hpp>
#include <set>
#include <type_traits>

namespace siege::views
{
  struct preferences_view final : win32::window_ref
  {
    win32::list_box options;
    std::function<void()> options_unbind;

    win32::list_box advanced_options;

    struct item_context
    {
      std::size_t item_index;
      std::wstring_view key;
      std::optional<COLORREF> value;
    };

    std::vector<item_context> color_items;

    struct
    {
      win32::button button;
      win32::combo_box combo_box;
      win32::combo_box_ex combo_box_ex;
      win32::edit edit;
      win32::header header;
      win32::tree_view tree_view;
      win32::list_view list_view;
      win32::tool_bar toolbar;
      win32::list_box list_box;
      win32::menu menu;
      win32::window window;
      win32::static_control static_control;
      win32::tab_control tab_control;
      win32::scroll_bar scroll_bar;
    } sample;

    std::map<std::wstring_view, std::wstring_view> control_labels = {
      { win32::button::class_name, L"Button" },
      { win32::combo_box::class_name, L"Combo Box" },
      { win32::combo_box_ex::class_name, L"Combo Box Ex" },
      { win32::edit::class_name, L"Edit" },
      { win32::list_box::class_name, L"List Box" },
      { win32::scroll_bar::class_name, L"Scroll Bar" },
      { win32::static_control::class_name, L"Static Control" },
      { win32::header::class_name, L"Header" },
      { win32::list_view::class_name, L"List View" },
      { win32::tab_control::class_name, L"Tab Control" },
      { win32::tree_view::class_name, L"Tree View" },

      { win32::tool_bar::class_name, L"Toolbar" },
      { L"Menu", L"Menu" },
      { L"Window", L"Window" }
    };

    std::map<std::wstring_view, std::wstring_view> property_labels = {
      { L"BkColor", L"Background Color" },
      { L"TextColor", L"Text Color" },
      { L"LineColor", L"Line Color" },
      { L"TextBkColor", L"Text Background Color" },
      { L"OutlineColor", L"Outline Color" },
      { L"BtnHighlightColor", L"Button Highlight Color" },
      { L"BtnShadowColor", L"Button Shadow Color" },
      { L"BtnFaceColor", L"Button Face Color" },
      { L"TextHighlightColor", L"Text Highlight Color" },
      { L"MarkColor", L"Mark Color" },
    };

    std::array<COLORREF, 16> colors{};

    // simple settings has preferred theme option (from system or user-defined)
    // simple settings has preferred accent color (from system or user-defined)
    // simple settings has theme selection (light, dark)
    // advanced allows theme settings to be changed
    // allows theme settings to be saved
    // theme settings changed per control type

    preferences_view(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      auto style = this->GetWindowStyle();
      this->SetWindowStyle(style | WS_CLIPCHILDREN);
      auto control_factory = win32::window_factory(ref());

      options = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

      options.InsertString(-1, L"Theming (Simple)");
      options.InsertString(-1, L"Theming (Advanced)");
      options.SetCurrentSelection(0);
      options_unbind = options.bind_lbn_sel_change(std::bind_front(&preferences_view::options_lbn_sel_change, this));

      advanced_options = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

      advanced_options.InsertString(-1, L"Button"); // Button + SysLink
      advanced_options.InsertString(-1, L"Text Input"); // Edit + IP Address + Up Down Control
      advanced_options.InsertString(-1, L"Text Output"); // Static + Tooltip
      advanced_options.InsertString(-1, L"Combo Box"); // Combo Box + Combo Box Ex
      advanced_options.InsertString(-1, L"List Box");
      advanced_options.InsertString(-1, L"Header");
      advanced_options.InsertString(-1, L"Toolbar");
      advanced_options.InsertString(-1, L"Tree View");
      advanced_options.InsertString(-1, L"List View");
      advanced_options.InsertString(-1, L"Tab Control");
      
      // Push Button + Primary Push Button
      // Flat Push Button + Flat Primary Push Button
      // Split Button + Primary Primary Split Button
      // Flat Split Button + Flat Primary Primary Split Button
      // Command Link + Primary Command Link
      // Sys Link + Push Box
      // Two State Checkbox + Three State Checkbox
      // Group Box + Radio Button + Radio Button
      // Push Like + Flat Push Like
      // Bitmap Button + Icon Button

      // Edit + Edit Password
      // IP Address
      // Up-Down Control 

      // Combo box simple
      // Combo box dropdown + combo box dropdown list
      // Combo box ex

      // List Box

      // Header
      // Header with checkboxes

      // Toolbar

      // Tree View
      // Tree View Checkboxes

      // List View
      
      // Tab Control 

      ListBox_SetItemHeight(options, 0, options.GetItemHeight(0) * 2);

      return 0;
    }

    std::map<std::wstring, COLORREF> hover_colors;

    win32::lresult_t control_settings_nm_custom_draw(win32::list_view, NMLVCUSTOMDRAW& custom_draw)
    {
      if (custom_draw.nmcd.dwDrawStage == CDDS_PREPAINT)
      {
        return CDRF_NOTIFYITEMDRAW;
      }

      if (custom_draw.nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
      {
        return CDRF_NOTIFYSUBITEMDRAW;
      }

      if (custom_draw.nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT) && custom_draw.dwItemType == LVCDI_ITEM)
      {
        if (custom_draw.iSubItem == 0)
        {
          return CDRF_DODEFAULT;
        }

        std::wstring temp;
        temp.push_back((wchar_t)custom_draw.nmcd.dwItemSpec);
        temp.push_back((wchar_t)custom_draw.iSubItem);

        auto color = hover_colors.find(temp);

        if (color != hover_colors.end())
        {
          custom_draw.clrTextBk = color->second;
        }

        return CDRF_DODEFAULT;
      }

      return CDRF_DODEFAULT;
    }

    void options_lbn_sel_change(win32::list_box hwndFrom, const NMHDR&)
    {
      if (options.GetCurrentSelection() == 1)
      {
        ShowWindow(advanced_options, SW_SHOW);
      }
      else
      {
        ShowWindow(advanced_options, SW_HIDE);
      }
    }

    void control_settings_lvn_end_scroll(win32::list_view, const NMLVSCROLL& notice)
    {
      auto client_size = this->GetClientSize();

      if (client_size)
      {
        auto min_width = client_size->cx / 12;
        auto left_size = SIZE{ .cx = min_width * 2, .cy = client_size->cy };
        auto middle_size = SIZE{ .cx = min_width * 4, .cy = client_size->cy };
        auto right_size = SIZE{ .cx = client_size->cx - middle_size.cx - left_size.cx, .cy = client_size->cy };
      }
    }

    static UINT_PTR CALLBACK DialogColorHook(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam)
    {
      if (message == WM_INITDIALOG)
      {
        auto* item = (CHOOSECOLORW*)lParam;
        auto x = LOWORD(item->lCustData);
        auto y = HIWORD(item->lCustData);
        win32::window_ref(dialog).SetWindowPos(POINT{ x, y });
      }

      return 0;
    }

    std::wstring temp_text = std::wstring(255, L'\0');

    auto wm_size(std::size_t, SIZE client_size)
    {
      auto min_width = client_size.cx / 12;
      auto left_size = SIZE{ .cx = min_width * 2, .cy = client_size.cy };
      auto middle_size = SIZE{ .cx = min_width * 4, .cy = client_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - middle_size.cx - left_size.cx, .cy = client_size.cy };

      options.SetWindowPos(POINT{});
      options.SetWindowPos(left_size);

      advanced_options.SetWindowPos(POINT{.x = left_size.cx });
      advanced_options.SetWindowPos(left_size);

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      return std::nullopt;
    }
  };
}// namespace siege::views

#endif