#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include "exe_controller.hpp"

namespace siege::views
{
  struct exe_view : win32::window_ref
  {
    exe_controller controller;

    // TODO implement tabbing of each table
    win32::tab_control tabs;

    win32::list_view resource_table;
    win32::list_view string_table;
    win32::list_view launch_table;

    exe_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto on_create(const win32::create_message&)
    {
      auto control_factory = win32::window_factory(ref());

      resource_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

      resource_table.InsertColumn(-1, LVCOLUMNW{
                                        .pszText = const_cast<wchar_t*>(L"Name"),
                                      });

      resource_table.InsertColumn(-1, LVCOLUMNW{
                                        .pszText = const_cast<wchar_t*>(L"Action"),
                                      });

      resource_table.EnableGroupView(true);

      string_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Text"),
                                    });

      launch_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Type"),
                                    });

      launch_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Name"),
                                    });

      launch_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Value"),
                                    });

      return 0;
    }

    auto on_size(win32::size_message sized)
    {
      auto one_third = SIZE{ .cx = sized.client_size.cx, .cy = sized.client_size.cy / 3 };

      resource_table.SetWindowPos(one_third);
      resource_table.SetWindowPos(POINT{});

      string_table.SetWindowPos(one_third);
      string_table.SetWindowPos(POINT{ .y = one_third.cy });

      launch_table.SetWindowPos(one_third);
      launch_table.SetWindowPos(POINT{ .y = one_third.cy * 2 });

      std::array<std::reference_wrapper<win32::list_view>, 3> tables = { { std::ref(resource_table), std::ref(string_table), std::ref(launch_table) } };

      for (auto& table : tables)
      {
        auto column_count = table.get().GetColumnCount();

        if (!column_count)
        {
          continue;
        }

        auto column_width = sized.client_size.cx / column_count;

        for (auto i = 0u; i < column_count; ++i)
        {
          table.get().SetColumnWidth(i, column_width);
        }
      }

      return 0;
    }

    auto on_copy_data(win32::copy_data_message<char> message)
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

          auto& group = groups.emplace_back(value.first, std::move(items));
          group.state = LVGS_COLLAPSIBLE;
        }

        resource_table.InsertGroups(groups);

        auto strings = controller.get_strings();

        return TRUE;
      }

      return FALSE;
    }

    std::optional<win32::lresult_t> on_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {

        return 0;
      }

      return std::nullopt;
    }
  };
}// namespace siege::views

#endif