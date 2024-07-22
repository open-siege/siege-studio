#ifndef THEME_VIEW_HPP
#define THEME_VIEW_HPP

#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>


namespace siege::views
{
  struct theme_view : win32::window_ref
  {
    win32::window_ref theme_properties;
    win32::list_box options;

    win32::list_view control_settings;

    // list box for simple and advanced settings
    // simple settings has preferred theme option (from system or user-defined)
    // simple settings has preferred accent color (from system or user-defined)
    // simple settings has theme selection (light, dark)
    // advanced allows theme settings to be changed
    // allows theme settings to be saved
    // theme settings changed per control type

    theme_view(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
    {
      if (IsWindow((win32::hwnd_t)params.lpCreateParams))
      {
        theme_properties.reset((win32::hwnd_t)params.lpCreateParams);
      }
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      options = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

      options.InsertString(-1, L"Simple");
      options.InsertString(-1, L"Advanced");
      options.SetCurrentSelection(0);
      ListBox_SetItemHeight(options, 0, options.GetItemHeight(0) * 2);

      control_settings = *control_factory.CreateWindowExW<win32::list_view>(::CREATESTRUCTW{
        .style = WS_CHILD | LVS_REPORT });

      control_settings.EnableGroupView(true);

      control_settings.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Property"),
                                    });

      control_settings.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Value"),
                                    });

      std::vector<win32::list_view_group> groups;
      groups.reserve(16);

      groups.emplace_back(L"Window", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });

      groups.emplace_back(L"Menu", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });

      groups.emplace_back(L"Static", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });

      groups.emplace_back(L"Button", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });

      groups.emplace_back(L"List Box", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });

      groups.emplace_back(L"Combo Box", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });

      groups.emplace_back(L"Edit", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });

      groups.emplace_back(L"List View", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });

      groups.emplace_back(L"Tree View", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });

      groups.emplace_back(L"Tab Control", std::vector<win32::list_view_item>{ win32::list_view_item{ L"Background Color" }, win32::list_view_item{ L"" } });


      for (auto& group : groups)
      {
        group.state = LVGS_COLLAPSIBLE;
      }

      control_settings.InsertGroups(groups);

      return 0;
    }

    std::optional<win32::lresult_t> wm_notify(win32::notify_message message)
    {
      if (message.code == LBN_SELCHANGE && message.hwndFrom == options)
      {
        ShowWindow(control_settings, options.GetCurrentSelection() == 1 ? SW_SHOW : SW_HIDE);
      }

      return std::nullopt;
    }

    auto wm_size(std::size_t, SIZE client_size)
    {
      auto left_size = SIZE{ .cx = client_size.cx / 3, .cy = client_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - left_size.cx, .cy = client_size.cy };

      options.SetWindowPos(POINT{});
      options.SetWindowPos(left_size);

      control_settings.SetWindowPos(POINT{ .x = left_size.cx });
      control_settings.SetWindowPos(right_size);

      auto column_count = control_settings.GetColumnCount();

      if (!column_count)
      {
        return 0;
      }

      auto column_width = right_size.cx / column_count;

      for (auto i = 0u; i < column_count; ++i)
      {
        control_settings.SetColumnWidth(i, column_width);
      }

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      return std::nullopt;
    }
  };
}// namespace siege::views

#endif