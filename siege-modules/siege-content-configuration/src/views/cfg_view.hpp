#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/window_factory.hpp>
#include <siege/platform/win/theming.hpp>
#include "cfg_controller.hpp"

namespace siege::views
{
  // TODO implement saving of configurations for supported games
  struct cfg_view final : win32::window_ref
  {
    cfg_controller controller;

    win32::list_view table;

    cfg_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

      table.InsertColumn(-1, LVCOLUMNW{
                               .pszText = const_cast<wchar_t*>(L"Key"),
                             });

      table.InsertColumn(-1, LVCOLUMNW{
                               .pszText = const_cast<wchar_t*>(L"Value"),
                             });

      table.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

      auto header = table.GetHeader();

      auto style = header.GetWindowStyle();

      header.SetWindowStyle(style | HDS_NOSIZING | HDS_FLAT);

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      table.SetWindowPos(client_size);
      table.SetWindowPos(POINT{});

      auto column_count = table.GetColumnCount();

      auto column_width = client_size.cx / column_count;

      for (auto i = 0u; i < column_count; ++i)
      {
        table.SetColumnWidth(i, column_width);
      }

      return 0;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::ispanstream stream(message.data);

      if (controller.is_cfg(stream))
      {
        auto size = controller.load_config(stream);

        if (size > 0)
        {
          auto values = controller.get_key_values();

          for (auto& value : values)
          {
            win32::list_view_item item{ value.first };
            item.sub_items = {
              value.second
            };

            table.InsertRow(item);
          }
          return TRUE;
        }
      }

      return FALSE;
    }
  };
}// namespace siege::views

#endif