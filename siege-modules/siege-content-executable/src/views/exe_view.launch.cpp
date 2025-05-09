
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include "input-filter.hpp"
#include "input_injector.hpp"
#include "exe_views.hpp"
#include <detours.h>
#include <imagehlp.h>

using game_command_line_caps = siege::platform::game_command_line_caps;

namespace siege::views
{
  constexpr auto pref_name = L"MULTIPLAYER_HOSTING_PREFERENCE";
  constexpr static auto pref_options = std::array<std::wstring_view, 4>{ { L"Offline/Singleplayer", L"Client/Connect to Server", L"Listen/Host & Connect", L"Dedicated Server" } };

  void exe_view::populate_launch_table(const game_command_line_caps& caps)
  {
    auto& settings = controller.get_game_settings();

    bool has_ip = (caps.ip_connect_setting == nullptr || !std::wstring_view(caps.ip_connect_setting).empty());
    bool has_listen = (caps.listen_setting == nullptr || !std::wstring_view(caps.listen_setting).empty());
    bool has_dedicated = (caps.dedicated_setting == nullptr || !std::wstring_view(caps.dedicated_setting).empty());


    std::vector<std::wstring_view> real_options;
    real_options.reserve(4);
    for (auto i = 0; i < pref_options.size(); ++i)
    {
      if (i == 1 && !has_listen)
      {
        continue;
      }

      if (i == 2 && !has_listen)
      {
        continue;
      }
      if (i == 3 && !has_dedicated)
      {
        continue;
      }
      real_options.emplace_back(pref_options[i]);
    }


    auto has_networking = has_ip || has_listen || has_dedicated;

    if (has_networking)
    {
      launch_settings.emplace_back(game_setting{
        .setting_name = pref_name,
        .type = game_command_line_caps::env_setting,
        .value = std::wstring{ pref_options[1] },
        .display_name = L"Hosting",
        .group_id = 1,
        .get_predefined_string = [real_options = std::move(real_options), results = std::vector<siege::platform::predefined_string>{}](auto name) mutable -> std::span<siege::platform::predefined_string> {
          if (name == pref_name)
          {
            if (!results.empty())
            {
              return results;
            }
            for (auto& string : real_options)
            {
              results.emplace_back(siege::platform::predefined_string{
                .label = string.data(),
                .value = string.data() });
            }
            return results;
          }

          return std::span<siege::platform::predefined_string>{};
        } });
    }

    if (has_networking && controller.has_zero_tier_extension())
    {
      launch_settings.emplace_back(game_setting{
        .setting_name = L"ZERO_TIER_NETWORK_ID",
        .type = game_command_line_caps::env_setting,
        .value = std::wstring{ settings.last_zero_tier_network_id.data() },
        .display_name = L"Zero Tier Network ID",
        .group_id = 1,
      });

      std::string_view zt_node_id = settings.last_zero_tier_node_id_and_private_key.data();

      if (zt_node_id.contains(':'))
      {
        zt_node_id = zt_node_id.substr(0, zt_node_id.find(':'));

        launch_settings.emplace_back(game_setting{
          .setting_name = L"ZERO_TIER_PEER_ID",
          .type = game_command_line_caps::computed_setting,
          .value = std::wstring(zt_node_id.begin(), zt_node_id.end()),
          .display_name = L"Zero Tier Node ID",
          .group_id = 1 });
      }
    }

    std::wstring_view dedicated_setting = caps.dedicated_setting ? caps.dedicated_setting : L"";
    std::wstring_view listen_setting = caps.listen_setting ? caps.listen_setting : L"";

    for (auto& setting : caps.string_settings)
    {
      if (!setting)
      {
        break;
      }

      std::wstring_view player_name_setting = caps.player_name_setting ? caps.player_name_setting : L"";

      if (!player_name_setting.empty() && setting == player_name_setting)
      {
        launch_settings.emplace_back(game_setting{
          .setting_name = std::wstring(player_name_setting),
          .type = game_command_line_caps::string_setting,
          .value = settings.last_player_name.data(),
          .display_name = L"Player Name",
          .group_id = has_ip ? 1 : 2 });
        continue;
      }

      std::wstring_view ip_connect_setting = caps.ip_connect_setting ? caps.ip_connect_setting : L"";

      if (!ip_connect_setting.empty() && setting == ip_connect_setting)
      {
        launch_settings.emplace_back(game_setting{
          .setting_name = std::wstring(ip_connect_setting),
          .type = game_command_line_caps::string_setting,
          .value = settings.last_ip_address.data(),
          .display_name = L"Server IP Address",
          .group_id = 1 });
        continue;
      }

      // has to be checked after in case one of the predefined settings are the same as these two.
      if (!dedicated_setting.empty() && setting == dedicated_setting)
      {
        dedicated_setting_type = game_command_line_caps::string_setting;
        continue;
      }

      if (!listen_setting.empty() && setting == listen_setting)
      {
        listen_setting_type = game_command_line_caps::string_setting;
        continue;
      }

      auto proc = controller.get_extension().get_predefined_string_command_line_settings_proc;

      launch_settings.emplace_back(game_setting{
        .setting_name = setting,
        .type = game_command_line_caps::string_setting,
        .value = L"",
        .display_name = setting,
        .group_id = 2 });

      if (proc)
      {
        launch_settings.back().get_predefined_string = [proc](std::wstring_view name) {
          auto result = proc(name.data());

          auto size = 0;

          auto* first = result;

          do
          {
            if (!first)
            {
              break;
            }
            if (!first->label)
            {
              break;
            }
            size++;
            first++;
          } while (first->label);

          return std::span(result, size);
        };
      }
    }


    auto is_valid = [&](auto& setting, game_command_line_caps::type type) {
      if (!dedicated_setting.empty() && setting == dedicated_setting)
      {
        dedicated_setting_type = type;
        return false;
      }

      if (!listen_setting.empty() && setting == listen_setting)
      {
        listen_setting_type = type;
        return false;
      }

      return true;
    };

    for (auto& setting : caps.int_settings)
    {
      if (!setting)
      {
        break;
      }

      if (!is_valid(setting, game_command_line_caps::int_setting))
      {
        continue;
      }

      launch_settings.emplace_back(game_setting{
        .setting_name = setting,
        .type = game_command_line_caps::int_setting,
        .value = L"",
        .display_name = setting,
        .group_id = 2 });

      auto proc = controller.get_extension().get_predefined_int_command_line_settings_proc;

      if (proc)
      {
        launch_settings.back().get_predefined_int = [proc](std::wstring_view name) {
          auto result = proc(name.data());

          auto size = 0;

          auto* first = result;

          do
          {
            if (!first->label)
            {
              break;
            }
            size++;
            first++;
          } while (first->label);

          return std::span(result, size);
        };
      }
    }

    for (auto& setting : caps.float_settings)
    {
      if (!setting)
      {
        break;
      }

      if (!is_valid(setting, game_command_line_caps::float_setting))
      {
        continue;
      }

      launch_settings.emplace_back(game_setting{
        .setting_name = setting,
        .type = game_command_line_caps::float_setting,
        .value = L"",
        .display_name = setting,
        .group_id = 2 });
    }

    for (auto& setting : caps.flags)
    {
      if (!setting)
      {
        break;
      }

      if (!is_valid(setting, game_command_line_caps::flag_setting))
      {
        continue;
      }

      launch_settings.emplace_back(game_setting{
        .setting_name = setting,
        .type = game_command_line_caps::flag_setting,
        .value = L"",
        .display_name = setting,
        .group_id = 2 });
    }

    int setting_index = 0;
    for (auto& setting : launch_settings)
    {
      std::shared_ptr<void> deferred(nullptr, [&](...) { setting_index++; });
      if (setting.display_name.empty())
      {
        continue;
      }

      win32::list_view_item column(setting.display_name);
      column.mask = column.mask | LVIF_PARAM | LVIF_GROUPID;
      column.sub_items = { std::visit([](auto& item) -> std::wstring {
        if constexpr (std::is_same_v<std::decay_t<decltype(item)>, std::wstring>)
        {
          return item;
        }

        if constexpr (std::is_same_v<std::decay_t<decltype(item)>, bool>)
        {
          return item ? L"Yes" : L"No";
        }

        if constexpr (std::is_same_v<std::decay_t<decltype(item)>, int>)
        {
          return std::to_wstring(item);
        }

        if constexpr (std::is_same_v<std::decay_t<decltype(item)>, float>)
        {
          return std::to_wstring(item);
        }

        return L"";
      },
        setting.value) };
      column.lParam = (LPARAM)setting_index;
      column.iGroupId = setting.group_id;

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

            ::LVITEMW item{
              .mask = LVIF_PARAM,
              .iItem = info.iItem
            };

            ListView_GetItem(launch_table, &item);

            auto& setting = launch_settings.at((std::size_t)item.lParam);

            auto& extension = controller.get_extension();

            ListView_GetItemText(launch_table, info.iItem, 0, text.data(), text.size());

            bool uses_combo = false;

            std::wstring temp;
            if ((setting.type == game_command_line_caps::string_setting || setting.type == game_command_line_caps::env_setting) && setting.get_predefined_string)
            {
              if (auto values = setting.get_predefined_string(setting.setting_name); !values.empty())
              {
                ::SendMessageW(launch_table_combo, CB_RESETCONTENT, 0, 0);

                for (auto& item : values)
                {
                  uses_combo = true;
                  temp = item.label;
                  ::COMBOBOXEXITEMW new_item{
                    .mask = CBEIF_LPARAM | CBEIF_TEXT,
                    .iItem = -1,
                    .pszText = temp.data(),
                    .cchTextMax = (int)temp.size(),
                    .lParam = (LPARAM)item.value
                  };
                  ::SendMessageW(launch_table_combo, CBEM_INSERTITEMW, 0, (LPARAM)&new_item);
                }
              }
            }
            else if (setting.type == game_command_line_caps::int_setting && setting.get_predefined_int)
            {
              if (auto values = setting.get_predefined_int(text.data()); !values.empty())
              {
                ::SendMessageW(launch_table_combo, CB_RESETCONTENT, 0, 0);

                for (auto& item : values)
                {
                  uses_combo = true;
                  temp = item.label;
                  ::COMBOBOXEXITEMW new_item{
                    .mask = CBEIF_LPARAM | CBEIF_TEXT,
                    .iItem = -1,
                    .pszText = temp.data(),
                    .cchTextMax = (int)temp.size(),
                    .lParam = (LPARAM)item.value
                  };
                  ::SendMessageW(launch_table_combo, CBEM_INSERTITEMW, 0, (LPARAM)&new_item);
                }
              }
            }

            if (launch_table_edit_unbind)
            {
              launch_table_edit_unbind();
              launch_table_edit_unbind = nullptr;
            }

            if (uses_combo)
            {
              launch_table_combo.SetWindowPos(result->second);
              launch_table_combo.SetWindowPos(HWND_TOP);
              launch_table_combo.SetWindowStyle(launch_table_combo.GetWindowStyle() | WS_VISIBLE);
              ::SendMessageW(launch_table_combo, CB_SHOWDROPDOWN, TRUE, 0);

              launch_table_edit_unbind = launch_table_combo.bind_cbn_sel_change([this, info, control_type = setting.type](auto, const auto&) {
                std::fill_n(text.data(), text.size(), L'\0');
                ::COMBOBOXEXITEMW new_item{
                  .mask = CBEIF_LPARAM | CBEIF_TEXT,
                  .iItem = ::SendMessage(launch_table_combo, CB_GETCURSEL, 0, 0),
                  .pszText = text.data(),
                  .cchTextMax = (int)text.size(),
                };

                if (::SendMessageW(launch_table_combo, CBEM_GETITEMW, 0, (LPARAM)&new_item))
                {
                  if (control_type == game_command_line_caps::string_setting || control_type == game_command_line_caps::env_setting)
                  {
                    ListView_SetItemText(launch_table, info.iItem, info.iSubItem, (wchar_t*)new_item.lParam);
                  }
                  else
                  {
                    auto item = std::to_wstring(new_item.lParam);
                    ListView_SetItemText(launch_table, info.iItem, info.iSubItem, item.data());
                  }
                }
              });
            }
            else if (setting.type != game_command_line_caps::computed_setting)
            {
              launch_table_combo.SetWindowStyle(launch_table_combo.GetWindowStyle() & ~WS_VISIBLE);

              // TODO I need to start moving these hard-coded strings
              // into a central location for my sanity
              if (setting.display_name == L"Server IP Address")
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
      extract_selected_files();
      return TRUE;
    }

    if (message.dwItemSpec == launch_selected_id)
    {
      if (controller.has_extension_module())
      {
        auto backends = controller.get_extension().controller_input_backends;
        auto actions = controller.get_extension().game_actions;
        std::size_t binding_index = 0;
        auto game_args = std::make_unique<siege::platform::game_command_line_args>();

        if (backends.empty())
        {
          for (auto i = 0; i < controller_table.GetItemCount(); ++i)
          {
            auto item = controller_table.GetItem(LVITEMW{
              .mask = LVIF_PARAM,
              .iItem = i });

            if (item && item->lParam)
            {
              auto iter = std::find_if(game_args->controller_to_send_input_mappings.begin(), game_args->controller_to_send_input_mappings.end(), [](auto& item) {
                return item.from_vkey == 0;
              });

              if (iter != game_args->controller_to_send_input_mappings.end())
              {
                iter->from_context = bound_inputs.at(item->lParam).from_context;
                iter->from_vkey = bound_inputs.at(item->lParam).from_vkey;
                iter->to_context = bound_inputs.at(item->lParam).to_context;
                iter->to_vkey = bound_inputs.at(item->lParam).to_vkey;
              }
            }
          }
        }
        else
        {
          std::array<RAWINPUTDEVICELIST, 64> controllers{};

          UINT size = controllers.size();
          ::GetRawInputDeviceList(controllers.data(), &size, sizeof(RAWINPUTDEVICELIST));


          for (auto i = 0; i < controller_table.GetItemCount(); ++i)
          {
            auto item = controller_table.GetItem(LVITEMW{
              .mask = LVIF_PARAM,
              .iItem = i });

            if (item && item->lParam)
            {
              try
              {
                auto virtual_key = bound_actions.at(item->lParam).vkey;
                auto context = bound_actions[item->lParam].context;
                auto action_index = bound_actions[item->lParam].action_index;

                auto& action = actions[action_index];

                auto hardware_index = hardware_index_for_controller_vkey(std::span<RAWINPUTDEVICELIST>(controllers.data(), size), 0, virtual_key);
                game_args->action_bindings[binding_index].vkey = virtual_key;
                game_args->action_bindings[binding_index].action_name = action.action_name;
                game_args->action_bindings[binding_index].context = hardware_index.first;
                game_args->action_bindings[binding_index++].hardware_index = hardware_index.second;
              }
              catch (...)
              {
              }
            }
          }
        }

        for (auto i = 0; i < keyboard_table.GetItemCount(); ++i)
        {
          auto item = keyboard_table.GetItem(LVITEMW{
            .mask = LVIF_PARAM,
            .iItem = i });

          if (item && item->lParam)
          {
            auto virtual_key = bound_actions[item->lParam].vkey;
            auto context = bound_actions[item->lParam].context;
            auto action_index = bound_actions[item->lParam].action_index;

            auto& action = actions[action_index];

            if (context == siege::platform::hardware_context::keyboard || context == siege::platform::hardware_context::keypad)
            {
              game_args->action_bindings[binding_index].vkey = virtual_key;
              game_args->action_bindings[binding_index].action_name = action.action_name;

              game_args->action_bindings[binding_index].hardware_index = MapVirtualKeyW(virtual_key, MAPVK_VK_TO_VSC);
              game_args->action_bindings[binding_index++].context = context;
            }
            else
            {
              game_args->action_bindings[binding_index].vkey = virtual_key;
              game_args->action_bindings[binding_index].action_name = action.action_name;
              game_args->action_bindings[binding_index++].context = context;
            }
          }
        }

        std::vector<std::array<std::wstring, 2>> launch_strings;
        launch_strings.reserve(launch_table.GetItemCount());

        auto& extension = controller.get_extension();
        auto* caps = extension.caps;

        siege::platform::persistent_game_settings settings{};

        ::LVITEMW info{
          .mask = LVIF_PARAM
        };

        auto string_index = 0;
        auto env_index = 0;

        bool should_connect = false;

        for (auto i = 0; i < launch_table.GetItemCount(); ++i)
        {
          info.iItem = i;
          ListView_GetItem(launch_table, &info);

          if (!info.lParam)
          {
            launch_strings.emplace_back(std::array<std::wstring, 2>{ { L"", L"" } });
            continue;
          }

          std::wstring name(255, L'\0');
          std::wstring value(255, L'\0');

          ListView_GetItemText(launch_table, i, 0, name.data(), name.size());
          ListView_GetItemText(launch_table, i, 1, value.data(), value.size());

          auto& setting = launch_settings.at(info.lParam);

          name.resize(name.find(L'\0'));
          value.resize(value.find(L'\0'));

          // TODO simplify this lookup and share the constants
          if (setting.display_name == L"Player Name")
          {
            name = caps->player_name_setting;
            auto max_size = value.size() > settings.last_player_name.size() ? settings.last_player_name.size() : value.size();
            std::copy_n(value.data(), max_size, settings.last_player_name.data());
          }
          else if (setting.display_name == L"Server IP Address")
          {
            name = caps->ip_connect_setting;
            auto max_size = value.size() > settings.last_ip_address.size() ? settings.last_ip_address.size() : value.size();
            std::copy_n(value.data(), max_size, settings.last_ip_address.data());
          }
          else if (setting.setting_name == L"ZERO_TIER_NETWORK_ID")
          {
            auto max_size = value.size() > settings.last_zero_tier_network_id.size() ? settings.last_zero_tier_network_id.size() : value.size();
            std::copy_n(value.data(), max_size, settings.last_zero_tier_network_id.data());
          }

          launch_strings.emplace_back(std::array<std::wstring, 2>{ { setting.setting_name, std::move(value) } });

          if (setting.setting_name == pref_name)
          {
            // constexpr static auto options = std::array<std::wstring_view, 4>{ { L"Offline/Singleplayer", L"Client/Connect to Server", L"Listen/Host & Connect", L"Dedicated Server" } };
            should_connect = value == pref_options[1];// connect to server

            // TODO there is a lot more computation needed here
            if (value == pref_options[2])// listen
            {
              game_args->string_settings[string_index].name = caps->listen_setting;
              game_args->string_settings[string_index++].value = L"0.0.0.0";
            }
            else if (value == pref_options[3])// dedicated
            {
              game_args->string_settings[string_index].name = caps->dedicated_setting;
              game_args->string_settings[string_index++].value = L"1";
            }
            continue;
          }

          if (setting.display_name == L"Server IP Address" && !should_connect)
          {
            continue;
          }

          // TODO store the UI values inside of the setting and then populate the
          // args with the setting and cover all the possible cases
          if (setting.type == game_command_line_caps::env_setting)
          {
            game_args->environment_settings[env_index].name = launch_strings[i][0].c_str();
            game_args->environment_settings[env_index++].value = launch_strings[i][1].c_str();
          }
          else if (setting.type != game_command_line_caps::computed_setting)
          {
            game_args->string_settings[string_index].name = launch_strings[i][0].c_str();
            game_args->string_settings[string_index++].value = launch_strings[i][1].c_str();
          }
        }

        controller.set_game_settings(settings);

        input_injector_args args{
          .args = std::move(game_args),
          .launch_game_with_extension = [this](const auto* args, auto* process_info) -> HRESULT {
            return controller.launch_game_with_extension(args, process_info);
          }
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