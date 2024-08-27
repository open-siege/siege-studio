#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <map>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include "exe_controller.hpp"

namespace siege::views
{
  using namespace std::literals;

  struct exe_view final : win32::window_ref
    , win32::list_box::notifications
    , win32::list_view::notifications
  {
    exe_controller controller;

    win32::list_box options;
    win32::list_view resource_table;
    win32::list_view string_table;
    win32::list_view launch_table;

    std::map<std::wstring_view, std::wstring_view> group_names = {
      { L"#1"sv, L"Hardware Dependent Cursor"sv },
      { L"#2"sv, L"Bitmap"sv },
      { L"#3"sv, L"Hardware Dependent Icon"sv },
      { L"#4"sv, L"Menu"sv },
      { L"#5"sv, L"Dialog"sv },
      { L"#6"sv, L"String Table"sv },
      { L"#8"sv, L"Font"sv },
      { L"#9"sv, L"Accelerator"sv },
      { L"#10"sv, L"Raw Data"sv },
      { L"#12"sv, L"Hardware Independent Cursor"sv },
      { L"#14"sv, L"Hardware Independent Icon"sv },
      { L"#16"sv, L"Version"sv },
      { L"#22"sv, L"Animated Icon"sv },
      { L"#23"sv, L"HTML"sv },
      { L"#24"sv, L"Side-by-Side Assembly Manifest"sv },
    };

    exe_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      options = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });


      options.InsertString(-1, L"Resources");
      options.InsertString(-1, L"Scripting");
      options.InsertString(-1, L"Launch Options");
      options.SetCurrentSelection(0);

      resource_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER });

      resource_table.InsertColumn(-1, LVCOLUMNW{
                                        .pszText = const_cast<wchar_t*>(L"Name"),
                                      });

      resource_table.InsertColumn(-1, LVCOLUMNW{
                                        .pszText = const_cast<wchar_t*>(L"Action"),
                                      });

      resource_table.EnableGroupView(true);

      string_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER });
      string_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Text"),
                                    });

      string_table.EnableGroupView(true);

      launch_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

      launch_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Type"),
                                    });

      launch_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Name"),
                                    });

      launch_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Value"),
                                    });

      auto header = launch_table.GetHeader();

      auto style = header.GetWindowStyle();

      header.SetWindowStyle(style | HDS_NOSIZING | HDS_FLAT);


      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      auto one_quarter = SIZE{ .cx = client_size.cx / 4, .cy = client_size.cy };
      auto three_quarters = SIZE{ .cx = client_size.cx - one_quarter.cx, .cy = client_size.cy };

      resource_table.SetWindowPos(three_quarters);
      resource_table.SetWindowPos(POINT{});

      string_table.SetWindowPos(three_quarters);
      string_table.SetWindowPos(POINT{});

      launch_table.SetWindowPos(three_quarters);
      launch_table.SetWindowPos(POINT{});

      options.SetWindowPos(one_quarter);
      options.SetWindowPos(POINT{ .x = three_quarters.cx });

      std::array<std::reference_wrapper<win32::list_view>, 3> tables = { { std::ref(resource_table), std::ref(string_table), std::ref(launch_table) } };

      for (auto& table : tables)
      {
        auto column_count = table.get().GetColumnCount();

        if (!column_count)
        {
          continue;
        }

        auto column_width = three_quarters.cx / column_count;

        for (auto i = 0u; i < column_count; ++i)
        {
          table.get().SetColumnWidth(i, column_width);
        }
      }

      return 0;
    }

    std::optional<win32::lresult_t> wm_command(win32::list_box hwndFrom, int code) override
    {
      if (code == LBN_SELCHANGE && hwndFrom == options)
      {
        auto selected = options.GetCurrentSelection();
        ::ShowWindow(resource_table, SW_HIDE);
        ::ShowWindow(string_table, SW_HIDE);
        ::ShowWindow(launch_table, SW_HIDE);

        if (selected == 0)
        {
          ::ShowWindow(resource_table, SW_SHOW);
        }
        else if (selected == 1)
        {
          ::ShowWindow(string_table, SW_SHOW);
        }
        else if (selected == 2)
        {
          ::ShowWindow(launch_table, SW_SHOW);
        }


        return 0;
      }

      return std::nullopt;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      std::optional<std::filesystem::path> path;

      if (wchar_t* filename = this->GetPropW<wchar_t*>(L"FilePath"); filename)
      {
        path = filename;
      }

      auto count = controller.load_executable(stream, std::move(path));

      if (count > 0)
      {
        auto values = controller.get_resource_names();

        std::vector<win32::list_view_group> groups;

        groups.reserve(values.size());

        for (auto& value : values)
        {
          std::vector<win32::list_view_item> items;
          items.reserve(value.second.size());

          for (auto& child : value.second)
          {
            items.emplace_back(win32::list_view_item(child));
          }

          if (group_names.contains(value.first))
          {
            auto& group = groups.emplace_back(std::wstring(group_names[value.first]), std::move(items));
            group.state = LVGS_COLLAPSIBLE;
          }
          else
          {
            auto& group = groups.emplace_back(value.first, std::move(items));
            group.state = LVGS_COLLAPSIBLE;
          }
        }

        resource_table.InsertGroups(groups);

        groups.clear();
        groups.reserve(2);

        auto functions = controller.get_function_names();
        std::vector<win32::list_view_item> items;
        items.reserve(functions.size());

        for (auto& child : functions)
        {
          items.emplace_back(win32::list_view_item(win32::widen(child)));
        }

        auto& group1 = groups.emplace_back(L"Script/Console Functions", std::move(items));
        group1.state = LVGS_COLLAPSIBLE;

        auto variables = controller.get_variable_names();
        items.clear();
        items.reserve(variables.size());

        for (auto& child : variables)
        {
          items.emplace_back(win32::list_view_item(win32::widen(child)));
        }

        auto& group2 = groups.emplace_back(L"Script/Console Variables", std::move(items));
        group2.state = LVGS_COLLAPSIBLE;

        string_table.InsertGroups(groups);

        return TRUE;
      }

      return FALSE;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(resource_table);
        win32::apply_theme(launch_table);
        win32::apply_theme(string_table);
        win32::apply_theme(options);
        win32::apply_theme(*this);

        return 0;
      }

      return std::nullopt;
    }
  };
}// namespace siege::views

#endif