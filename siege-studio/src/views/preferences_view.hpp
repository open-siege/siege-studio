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

      sample.button = *control_factory.CreateWindowExW<win32::button>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        .lpszName = L"Sample button" });

      sample.combo_box = *control_factory.CreateWindowExW<win32::combo_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
        .lpszName = L"Sample combo box" });

      sample.combo_box_ex = *control_factory.CreateWindowExW<win32::combo_box_ex>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | CBS_DROPDOWN,
        .lpszName = L"Sample combo box ex" });

      sample.edit = *control_factory.CreateWindowExW<win32::edit>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD });

      sample.header = *control_factory.CreateWindowExW<win32::header>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD });

      sample.header.InsertItem(-1, HDITEMW{ .mask = HDI_TEXT | HDI_WIDTH, .cxy = 30, .pszText = const_cast<wchar_t*>(L"Sample") });
      sample.header.InsertItem(-1, HDITEMW{ .mask = HDI_TEXT | HDI_WIDTH, .cxy = 30, .pszText = const_cast<wchar_t*>(L"Header"), .cchTextMax = 5 });
      sample.header.InsertItem(-1, HDITEMW{ .mask = HDI_TEXT | HDI_WIDTH, .cxy = 30, .pszText = const_cast<wchar_t*>(L"Test"), .cchTextMax = 5 });

      sample.tree_view = *control_factory.CreateWindowExW<win32::tree_view>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD });

      sample.tree_view.InsertItem(TVINSERTSTRUCTW{
        .hInsertAfter = TVI_ROOT,
        .item = TVITEMW{
          .mask = TVIF_TEXT,
          .pszText = const_cast<wchar_t*>(L"Tree") } });

      sample.tree_view.InsertItem(TVINSERTSTRUCTW{
        .hInsertAfter = TVI_ROOT,
        .item = TVITEMW{
          .mask = TVIF_TEXT,
          .pszText = const_cast<wchar_t*>(L"View") } });

      sample.tree_view.InsertItem(TVINSERTSTRUCTW{
        .hInsertAfter = TVI_ROOT,
        .item = TVITEMW{
          .mask = TVIF_TEXT,
          .pszText = const_cast<wchar_t*>(L"Sample") } });

      sample.list_view = *control_factory.CreateWindowExW<win32::list_view>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD });

      sample.list_view.InsertItem(-1, LVITEMW{ .mask = LVIF_TEXT, .pszText = const_cast<wchar_t*>(L"List") });

      sample.list_view.InsertItem(-1, LVITEMW{ .mask = LVIF_TEXT, .pszText = const_cast<wchar_t*>(L"View") });

      sample.list_view.InsertItem(-1, LVITEMW{ .mask = LVIF_TEXT, .pszText = const_cast<wchar_t*>(L"Sample") });

      sample.toolbar = *control_factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | CCS_NOPARENTALIGN | CCS_NODIVIDER });

      sample.toolbar.InsertButton(-1, TBBUTTON{ .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON, .iString = (INT_PTR) const_cast<wchar_t*>(L"Tool") });

      sample.toolbar.InsertButton(-1, TBBUTTON{ .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON, .iString = (INT_PTR) const_cast<wchar_t*>(L"Bar") });

      sample.toolbar.InsertButton(-1, TBBUTTON{ .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON, .iString = (INT_PTR) const_cast<wchar_t*>(L"Sample") });

      sample.list_box = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS });

      sample.list_box.InsertString(-1, L"Sample");
      sample.list_box.InsertString(-1, L"List");
      sample.list_box.InsertString(-1, L"Test");

      sample.static_control = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD,
        .lpszName = L"Static Control Example" });

      sample.tab_control = *control_factory.CreateWindowExW<win32::tab_control>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD });

      sample.tab_control.InsertItem(-1, TCITEMW{ .mask = TCIF_TEXT, .pszText = const_cast<wchar_t*>(L"Tab") });
      sample.tab_control.InsertItem(-1, TCITEMW{ .mask = TCIF_TEXT, .pszText = const_cast<wchar_t*>(L"Control") });
      sample.tab_control.InsertItem(-1, TCITEMW{ .mask = TCIF_TEXT, .pszText = const_cast<wchar_t*>(L"Sample") });

      options = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

      options.InsertString(-1, L"Theming (Simple)");
      options.InsertString(-1, L"Theming (Advanced)");
      options.SetCurrentSelection(0);
      options_unbind = options.bind_lbn_sel_change(std::bind_front(&preferences_view::options_lbn_sel_change, this));
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

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      return std::nullopt;
    }
  };
}// namespace siege::views

#endif