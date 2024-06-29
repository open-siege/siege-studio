#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include "cfg_controller.hpp"

namespace siege::views
{
  struct cfg_view : win32::window_ref
  {
    cfg_controller controller;

    win32::list_view table;

    cfg_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto on_create(const win32::create_message&)
    {
      auto control_factory = win32::window_factory(ref());

      table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

      table.InsertColumn(-1, LVCOLUMNW{
                               .pszText = const_cast<wchar_t*>(L"Key"),
                             });

      table.InsertColumn(-1, LVCOLUMNW{
                               .pszText = const_cast<wchar_t*>(L"Value"),
                             });

      on_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    auto on_size(win32::size_message sized)
    {
      table.SetWindowPos(sized.client_size);
      table.SetWindowPos(POINT{});

      auto column_count = table.GetColumnCount();

      auto column_width = sized.client_size.cx / column_count;

      for (auto i = 0u; i < column_count; ++i)
      {
        table.SetColumnWidth(i, column_width);
      }

      return 0;
    }

    auto on_copy_data(win32::copy_data_message<char> message)
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

    std::optional<win32::lresult_t> on_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        auto parent = this->GetParent();
        win32::apply_theme(*parent, table);
        return 0;
      }

      return std::nullopt;
    }
  };
}// namespace siege::views

#endif