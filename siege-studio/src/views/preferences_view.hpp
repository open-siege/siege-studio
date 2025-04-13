#ifndef THEME_VIEW_HPP
#define THEME_VIEW_HPP

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

    win32::button dark_mode_selection;
    win32::button by_system;
    win32::button forced_dark;
    win32::button forced_light;

    win32::window buttons;
    win32::window text_inputs;
    win32::window text_outputs;
    win32::window combo_boxes;
    win32::window list_box;
    win32::window header;
    win32::window toolbars;
    win32::window tree_view;
    win32::window list_views;
    win32::window tab_controls;

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

      options = *win32::CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

      options.InsertString(-1, L"Theming (Simple)");
      options.InsertString(-1, L"Theming (Advanced - Incomplete)");
      options.SetCurrentSelection(0);
      options_unbind = options.bind_lbn_sel_change(std::bind_front(&preferences_view::options_lbn_sel_change, this));

      dark_mode_selection = *win32::CreateWindowExW<win32::button>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        .lpszName = L"Application Theme Mode" });

      auto owner = ::GetAncestor(*this, GA_ROOTOWNER);
      auto preference = (int)::GetPropW(owner, L"UserThemePreference");


      auto invalidate = [this]() {
        ::InvalidateRect(dark_mode_selection, nullptr, TRUE);
        ::InvalidateRect(by_system, nullptr, TRUE);
        ::InvalidateRect(forced_light, nullptr, TRUE);
        ::InvalidateRect(forced_dark, nullptr, TRUE);
        ::InvalidateRect(options, nullptr, TRUE);
        ::InvalidateRect(*this, nullptr, TRUE);
      };

      auto update_registry = [](DWORD preference) {
        HKEY user_key = nullptr;
        HKEY main_key = nullptr;
        auto access = KEY_QUERY_VALUE | KEY_READ | KEY_WRITE;

        if (::RegOpenCurrentUser(access, &user_key) == ERROR_SUCCESS && ::RegCreateKeyExW(user_key, L"Software\\The Siege Hub\\Siege Studio", 0, nullptr, 0, access, nullptr, &main_key, nullptr) == ERROR_SUCCESS)
        {
          ::RegSetValueExW(main_key, L"UserThemePreference", 0, REG_DWORD, (BYTE*)&preference, sizeof(preference));
          ::RegCloseKey(main_key);
          ::RegCloseKey(user_key);
        }
      };

      by_system = *win32::CreateWindowExW<win32::button>(::CREATESTRUCTW{
        .hMenu = (HMENU)10,
        .hwndParent = *this,
        .style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        .lpszName = L"Follow System" });


      by_system.bind_bn_clicked([this, invalidate, update_registry](auto, auto&) {
        auto owner = ::GetAncestor(*this, GA_ROOTOWNER);
        ::RemovePropW(owner, L"UserThemePreference");
        update_registry(0);
        ::SendMessageW(owner, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet");
        win32::apply_window_theme(*this);
        invalidate();
      });

      if (preference == 0)
      {
        Button_SetCheck(by_system, BST_CHECKED);
      }

      forced_light = *win32::CreateWindowExW<win32::button>(::CREATESTRUCTW{
        .hMenu = (HMENU)10,
        .hwndParent = *this,
        .style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        .lpszName = L"Prefer Light" });

      forced_light.bind_bn_clicked([this, invalidate, update_registry](auto, auto&) {
        auto owner = ::GetAncestor(*this, GA_ROOTOWNER);
        update_registry(1);
        ::SetPropW(owner, L"UserThemePreference", (HANDLE)1);
        ::SendMessageW(owner, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet");
        win32::apply_window_theme(*this);
        invalidate();
      });

      if (preference == 1)
      {
        Button_SetCheck(forced_light, BST_CHECKED);
      }

      forced_dark = *win32::CreateWindowExW<win32::button>(::CREATESTRUCTW{
        .hMenu = (HMENU)10,
        .hwndParent = *this,
        .style = WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        .lpszName = L"Prefer Dark" });

      forced_dark.bind_bn_clicked([this, invalidate, update_registry](auto, auto&) {
        auto owner = ::GetAncestor(*this, GA_ROOTOWNER);
        update_registry(2);
        ::SetPropW(owner, L"UserThemePreference", (HANDLE)2);
        ::SendMessageW(owner, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet");
        win32::apply_window_theme(*this);
        invalidate();
      });

      if (preference == 2)
      {
        Button_SetCheck(forced_dark, BST_CHECKED);
      }

      advanced_options = *win32::CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

      advanced_options.InsertString(-1, L"Button");// Button + SysLink
      advanced_options.InsertString(-1, L"Text Input");// Edit + IP Address + Up Down Control
      advanced_options.InsertString(-1, L"Text Output");// Static + Tooltip
      advanced_options.InsertString(-1, L"Combo Box");// Combo Box + Combo Box Ex
      advanced_options.InsertString(-1, L"List Box");
      advanced_options.InsertString(-1, L"Header");
      advanced_options.InsertString(-1, L"Toolbar");
      advanced_options.InsertString(-1, L"Tree View");
      advanced_options.InsertString(-1, L"List View");
      advanced_options.InsertString(-1, L"Tab Control");
      advanced_options.bind_lbn_sel_change(std::bind_front(&preferences_view::advanced_options_lbn_sel_change, this));

      buttons = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_CHILD,
        .lpszClass = L"StackLayout" });

      buttons.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_PORTRAIT);
      buttons.SetPropW(L"DefaultHeight", win32::get_system_metrics(SM_CYSIZE) * 2);

      {
        auto temp = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = buttons,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = L"StackLayout" });

        temp.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_LANDSCAPE);

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
          .lpszName = L"Push Button",
          .lpszClass = L"Button" });

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
          .lpszName = L"Primary Push Button",
          .lpszClass = L"Button" });

        temp = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = buttons,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = L"StackLayout" });

        temp.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_LANDSCAPE);
        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
          .lpszName = L"Flat Push Button",
          .lpszClass = L"Button" });

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_FLAT,
          .lpszName = L"Flat Primary Push Button",
          .lpszClass = L"Button" });

        temp = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = buttons,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = L"StackLayout" });

        temp.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_LANDSCAPE);

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_SPLITBUTTON,
          .lpszName = L"Split Button",
          .lpszClass = L"Button" });

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_DEFSPLITBUTTON,
          .lpszName = L"Primary Split Button",
          .lpszClass = L"Button" });

        temp = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = buttons,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = L"StackLayout" });

        temp.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_LANDSCAPE);

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_SPLITBUTTON | BS_FLAT,
          .lpszName = L"Flat Split Button",
          .lpszClass = L"Button" });

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_DEFSPLITBUTTON | BS_FLAT,
          .lpszName = L"Flat Primary Split Button",
          .lpszClass = L"Button" });

        temp = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = buttons,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = L"StackLayout" });

        temp.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_LANDSCAPE);

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_COMMANDLINK,
          .lpszName = L"Command Link",
          .lpszClass = L"Button" });

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_DEFCOMMANDLINK,
          .lpszName = L"Primary Command Link",
          .lpszClass = L"Button" });

        temp = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = buttons,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = L"StackLayout" });

        temp.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_LANDSCAPE);

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszName = L"https://store.steampowered.com/app/3193420/Siege_Studio/",
          .lpszClass = WC_LINK });

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_PUSHBOX,
          .lpszName = L"Push Box",
          .lpszClass = L"Button" });

        temp = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = buttons,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = L"StackLayout" });
        temp.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_LANDSCAPE);

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
          .lpszName = L"Two State Checkbox",
          .lpszClass = L"Button" });

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_AUTO3STATE,
          .lpszName = L"Three State Checkbox",
          .lpszClass = L"Button" });


        temp = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = buttons,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = L"StackLayout" });
        temp.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_LANDSCAPE);

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_PUSHLIKE,
          .lpszName = L"Push-like",
          .lpszClass = L"Button" });

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = temp,
          .style = WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_FLAT,
          .lpszName = L"Flat Push-like",
          .lpszClass = L"Button" });
      }


      {
        text_inputs = *win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = *this,
          .style = WS_CHILD,
          .lpszClass = L"StackLayout" });

        text_inputs.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_PORTRAIT);
        text_inputs.SetPropW(L"DefaultHeight", win32::get_system_metrics(SM_CYSIZE) * 2);

        auto text = win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = text_inputs,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = L"Edit" });

        Edit_SetCueBannerText(*text, L"Edit Control");

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = text_inputs,
          .style = WS_CHILD | WS_VISIBLE | ES_PASSWORD,
          .lpszClass = L"Edit" });

        Edit_SetCueBannerText(*text, L"Password Edit Control");

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = text_inputs,
          .cy = 100,
          .cx = 300,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = WC_IPADDRESSW });

        win32::CreateWindowExW<win32::window>(::CREATESTRUCTW{
          .hwndParent = text_inputs,
          .style = WS_CHILD | WS_VISIBLE,
          .lpszClass = UPDOWN_CLASSW });
      }

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
      ShowWindow(advanced_options, SW_HIDE);
      ShowWindow(dark_mode_selection, SW_HIDE);
      ShowWindow(by_system, SW_HIDE);
      ShowWindow(forced_dark, SW_HIDE);
      ShowWindow(forced_light, SW_HIDE);

      if (options.GetCurrentSelection() == 0)
      {
        ShowWindow(dark_mode_selection, SW_SHOW);
        ShowWindow(by_system, SW_SHOW);
        ShowWindow(forced_dark, SW_SHOW);
        ShowWindow(forced_light, SW_SHOW);

        ShowWindow(buttons, SW_HIDE);
        ShowWindow(text_inputs, SW_HIDE);
        ShowWindow(text_outputs, SW_HIDE);
        ShowWindow(combo_boxes, SW_HIDE);
        ShowWindow(list_box, SW_HIDE);
        ShowWindow(header, SW_HIDE);
        ShowWindow(toolbars, SW_HIDE);
        ShowWindow(tree_view, SW_HIDE);
        ShowWindow(list_views, SW_HIDE);
        ShowWindow(tab_controls, SW_HIDE);
      }
      else if (options.GetCurrentSelection() == 1)
      {
        ShowWindow(advanced_options, SW_SHOW);
      }
    }

    void advanced_options_lbn_sel_change(win32::list_box hwndFrom, const NMHDR&)
    {
      ShowWindow(buttons, SW_HIDE);
      ShowWindow(text_inputs, SW_HIDE);
      ShowWindow(text_outputs, SW_HIDE);
      ShowWindow(combo_boxes, SW_HIDE);
      ShowWindow(list_box, SW_HIDE);
      ShowWindow(header, SW_HIDE);
      ShowWindow(toolbars, SW_HIDE);
      ShowWindow(tree_view, SW_HIDE);
      ShowWindow(list_views, SW_HIDE);
      ShowWindow(tab_controls, SW_HIDE);

      if (advanced_options.GetCurrentSelection() == 0)
      {
        ShowWindow(buttons, SW_SHOW);
      }
      else if (advanced_options.GetCurrentSelection() == 1)
      {
        ShowWindow(text_inputs, SW_SHOW);
      }
      else if (advanced_options.GetCurrentSelection() == 2)
      {
        ShowWindow(text_outputs, SW_SHOW);
      }
      else if (advanced_options.GetCurrentSelection() == 3)
      {
        ShowWindow(combo_boxes, SW_SHOW);
      }
      else if (advanced_options.GetCurrentSelection() == 4)
      {
        ShowWindow(list_box, SW_SHOW);
      }
      else if (advanced_options.GetCurrentSelection() == 5)
      {
        ShowWindow(header, SW_SHOW);
      }
      else if (advanced_options.GetCurrentSelection() == 6)
      {
        ShowWindow(toolbars, SW_SHOW);
      }
      else if (advanced_options.GetCurrentSelection() == 7)
      {
        ShowWindow(tree_view, SW_SHOW);
      }
      else if (advanced_options.GetCurrentSelection() == 8)
      {
        ShowWindow(list_views, SW_SHOW);
      }
      else if (advanced_options.GetCurrentSelection() == 9)
      {
        ShowWindow(tab_controls, SW_SHOW);
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

      advanced_options.SetWindowPos(POINT{ .x = left_size.cx });
      advanced_options.SetWindowPos(middle_size);

      buttons.SetWindowPos(POINT{ .x = left_size.cx + middle_size.cx });
      buttons.SetWindowPos(right_size);

      text_inputs.SetWindowPos(POINT{ .x = left_size.cx + middle_size.cx });
      text_inputs.SetWindowPos(right_size);

      text_outputs.SetWindowPos(POINT{ .x = left_size.cx + middle_size.cx });
      text_outputs.SetWindowPos(right_size);

      dark_mode_selection.SetWindowPos(POINT{ .x = left_size.cx });
      dark_mode_selection.SetWindowPos(right_size);

      auto width = middle_size.cx / 3;
      width -= 15;

      auto height = middle_size.cy / 3;
      height -= 15;

      by_system.SetWindowPos(POINT{ .x = left_size.cx + 15, .y = 15 });
      by_system.SetWindowPos(SIZE{ .cx = width, .cy = height });

      forced_dark.SetWindowPos(POINT{ .x = left_size.cx + width + 15, .y = 15 });
      forced_dark.SetWindowPos(SIZE{ .cx = width, .cy = height });

      forced_light.SetWindowPos(POINT{ .x = left_size.cx + width + width + 15, .y = 15 });
      forced_light.SetWindowPos(SIZE{ .cx = width, .cy = height });

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      return std::nullopt;
    }
  };
}// namespace siege::views

#endif