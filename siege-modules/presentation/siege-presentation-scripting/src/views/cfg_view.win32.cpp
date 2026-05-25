#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/basic_window.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include "cfg_shared.hpp"

namespace siege::views
{
  // TODO implement saving of configurations for supported games
  struct cfg_view final : win32::basic_window<cfg_view>
  {
    std::any state;

    win32::list_view table;

    cfg_view(win32::hwnd_t self, CREATESTRUCTW& params) : basic_window(self, params)
    {
    }

    auto wm_create()
    {
      table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

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

      if (is_cfg(stream))
      {
        auto size = load_config(state, stream);

        if (size > 0)
        {
          auto values = get_key_values(state);

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

    std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override
    {
      switch (message)
      {
      case WM_CREATE:
        return wm_create();
      case WM_SIZE:
        return (LRESULT)wm_size((std::size_t)wparam, SIZE(LOWORD(lparam), HIWORD(lparam)));
      case WM_COPYDATA:
        return (LRESULT)wm_copy_data(win32::copy_data_message<char>(wparam, lparam));
      default:
        return std::nullopt;
      }
    }
  };

  ATOM register_cfg_view(win32::window_module_ref module)
  {
    WNDCLASSEXW info{
      .cbSize = sizeof(info),
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = win32::basic_window<cfg_view>::window_proc,
      .cbWndExtra = sizeof(void*),
      .hInstance = module,
      .lpszClassName = win32::type_name<cfg_view>().c_str(),
    };
    return ::RegisterClassExW(&info);
  }
}// namespace siege::views