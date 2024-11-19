
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/dialog.hpp>
#include "input-filter.hpp"
#include "input_injector.hpp"
#include "exe_views.hpp"

using game_command_line_caps = siege::platform::game_command_line_caps;

namespace siege::views
{
  void exe_view::populate_launch_table(game_command_line_caps& caps)
  {
    auto& settings = controller.get_game_settings();

    for (auto& value : caps.string_settings)
    {
      if (!value)
      {
        break;
      }

      std::wstring_view player_name_setting = caps.player_name_setting ? caps.player_name_setting : L"";

      if (player_name_setting == value)
      {
        win32::list_view_item column(L"Player Name");
        column.sub_items = {
          settings.last_player_name.data()
        };

        launch_table.InsertRow(std::move(column));

        continue;
      }

      std::wstring_view ip_connect_setting = caps.ip_connect_setting ? caps.ip_connect_setting : L"";

      if (ip_connect_setting == value)
      {
        win32::list_view_item column(L"Server IP Address");
        column.sub_items = {
          settings.last_ip_address.data()
        };

        ip_address_row_index = launch_table.InsertRow(std::move(column));
        continue;
      }

      win32::list_view_item column(value);
      column.sub_items = {
        L""
      };

      launch_table.InsertRow(std::move(column));
    }
  }

  void exe_view::launch_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message)
  {
    POINT point;

    if (::GetCursorPos(&point) && ::ScreenToClient(launch_table, &point))
    {
      LVHITTESTINFO info{};
      info.pt = point;
      info.flags = LVHT_ONITEM;
      if (ListView_SubItemHitTest(launch_table, &info) != -1)
      {
        if (info.iSubItem == 0)
        {
          return;
        }

        RECT temp;

        if (ListView_GetSubItemRect(launch_table, info.iItem, info.iSubItem, LVIR_BOUNDS, &temp))
        {
          auto result = launch_table.MapWindowPoints(*this, temp);

          if (result)
          {
            static std::array<wchar_t, 256> text{};

            if (launch_table_edit_unbind)
            {
              launch_table_edit_unbind();
              launch_table_edit_unbind = nullptr;
            }

            if (info.iItem == ip_address_row_index)
            {
              launch_table_ip_address.SetWindowPos(result->second);
              launch_table_ip_address.SetWindowPos(HWND_TOP);
              launch_table_ip_address.SetWindowStyle(launch_table_ip_address.GetWindowStyle() | WS_VISIBLE);
              
              ListView_GetItemText(launch_table, info.iItem, info.iSubItem, text.data(), text.size());

              ::SetWindowTextW(launch_table_ip_address, text.data());


              launch_table_edit_unbind = launch_table_ip_address.bind_en_kill_focus([this, info](auto, const auto&) {
                launch_table_ip_address.SetWindowStyle(launch_table_ip_address.GetWindowStyle() & ~WS_VISIBLE);
                ::GetWindowTextW(launch_table_ip_address, text.data(), (int)text.size());
                ListView_SetItemText(launch_table, info.iItem, info.iSubItem, text.data());
              });
            }
            else
            {
              launch_table_edit.SetWindowPos(result->second);
              launch_table_edit.SetWindowPos(HWND_TOP);
              launch_table_edit.SetWindowStyle(launch_table_edit.GetWindowStyle() | WS_VISIBLE | WS_BORDER);


              ListView_GetItemText(launch_table, info.iItem, info.iSubItem, text.data(), text.size());

              ::SetWindowTextW(launch_table_edit, text.data());


              launch_table_edit_unbind = launch_table_edit.bind_en_kill_focus([this, info](auto, const auto&) {
                launch_table_edit.SetWindowStyle(launch_table_edit.GetWindowStyle() & ~WS_VISIBLE);
                ::GetWindowTextW(launch_table_edit, text.data(), (int)text.size());
                ListView_SetItemText(launch_table, info.iItem, info.iSubItem, text.data());
              });
            }
          }
        }
      }
    }
  }

  BOOL exe_view::exe_actions_nm_click(win32::tool_bar, const NMMOUSE& message)
  {
    ::SetFocus(this->exe_actions);
    if (message.dwItemSpec == add_to_firewall_selected_id)
    {
      auto path = this->controller.get_exe_path();

      std::wstring args;
      args.reserve(256);

      args.append(L"advfirewall firewall add rule dir=out enable=yes name=");

      args.append(1, L'\"');
      args.append(path.parent_path().stem());
      args.append(1, L'\"');
      args.append(L" action=allow program=");

      args.append(1, L'\"');
      args.append(path);
      args.append(1, L'\"');

      ::ShellExecuteW(nullptr,
        L"runas",
        L"netsh.exe",
        args.c_str(),
        nullptr,// default dir
        SW_SHOWNORMAL);

      return TRUE;
    }

    if (message.dwItemSpec == extract_selected_id)
    {
      return TRUE;
    }

    if (message.dwItemSpec == launch_selected_id)
    {
      if (controller.has_extension_module())
      {
        std::map<WORD, WORD> input_mapping{};

        for (auto i = 0; i < controller_table.GetItemCount(); ++i)
        {
          auto item = controller_table.GetItem(LVITEMW{
            .mask = LVIF_PARAM,
            .iItem = i });

          if (item && item->lParam)
          {
            auto controller_key = LOWORD(item->lParam);
            auto keyboard_key = HIWORD(item->lParam);
            input_mapping[controller_key] = keyboard_key;
          }
        }

        siege::platform::game_command_line_args game_args{};

        std::vector<std::array<std::wstring, 2>> launch_strings;
        launch_strings.reserve(launch_table.GetItemCount());

        auto& extension = controller.get_extension();
        auto* caps = extension.caps;

        siege::platform::persistent_game_settings settings{};

        for (auto i = 0; i < launch_table.GetItemCount(); ++i)
        {
          std::wstring name(255, L'\0');
          std::wstring value(255, L'\0');

          ListView_GetItemText(launch_table, i, 0, name.data(), name.size());
          ListView_GetItemText(launch_table, i, 1, value.data(), value.size());

          name.resize(name.find(L'\0'));
          value.resize(value.find(L'\0'));

          if (name == L"Player Name")
          {
            name = caps->player_name_setting;
            auto max_size = value.size() > settings.last_player_name.size() ? settings.last_player_name.size() : value.size();
            std::copy_n(value.data(), max_size, settings.last_player_name.data());
          }
          else if (name == L"Server IP Address")
          {
            name = caps->ip_connect_setting;
            auto max_size = value.size() > settings.last_ip_address.size() ? settings.last_ip_address.size() : value.size();
            std::copy_n(value.data(), max_size, settings.last_ip_address.data());
          }

          launch_strings.emplace_back(std::array<std::wstring, 2>{ { std::move(name), std::move(value) } });

          game_args.string_settings[i].name = launch_strings[i][0].c_str();
          game_args.string_settings[i].value = launch_strings[i][1].c_str();
        }

        controller.set_game_settings(settings);

        input_injector_args args{
          .exe_path = controller.get_exe_path(),
          .extension_path = controller.get_extension().GetModuleFileName(),
          .args = std::move(game_args),
          .controller_key_mappings = std::move(input_mapping),
          .extension = &controller.get_extension()
        };

        win32::DialogBoxIndirectParamW<siege::input_injector>(win32::module_ref::current_application(),
          win32::default_dialog({}),
          ref(),
          (LPARAM)&args);
      }

      return TRUE;
    }
    return FALSE;
  }
}// namespace siege::views