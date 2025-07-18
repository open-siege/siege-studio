
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/extension/input_filter.hpp>
#include "input_injector.hpp"
#include "exe_views.hpp"
#include <detours.h>
#include <imagehlp.h>

using game_command_line_caps = siege::platform::game_command_line_caps;

namespace siege::views
{
  constexpr auto hosting_pref_name = L"MULTIPLAYER_HOSTING_PREFERENCE";
  constexpr auto zt_fallback_ip = L"ZERO_TIER_FALLBACK_BROADCAST_IP_V4";
  constexpr static auto pref_options = std::array<std::wstring_view, 4>{ { L"Use Game UI", L"Client/Connect to Server", L"Listen/Host & Connect", L"Dedicated Server" } };

  auto convert_to_string = [](auto& item) -> std::wstring {
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

    if constexpr (std::is_same_v<std::decay_t<decltype(item)>, std::wstring>)
    {
      return item;
    }

    return L"";
  };

  auto enable_setting = [](game_setting& setting) {
    switch (setting.type)
    {
    case game_command_line_caps::env_setting:
      [[fallthrough]];
    case game_command_line_caps::string_setting: {
      setting.value = L"0.0.0.0";
      break;
    }
    case game_command_line_caps::int_setting: {
      setting.value = 1;
      break;
    }
    case game_command_line_caps::float_setting: {
      setting.value = 1.0f;
      break;
    }
    case game_command_line_caps::flag_setting: {
      setting.value = true;
      break;
    }
    case game_command_line_caps::computed_setting:
      [[fallthrough]];
    case game_command_line_caps::unknown: {
      break;
    }
    }
  };

  auto disable_setting = [](game_setting& setting) {
    switch (setting.type)
    {
    case game_command_line_caps::env_setting:
      [[fallthrough]];
    case game_command_line_caps::string_setting: {
      setting.value = L"";
      break;
    }
    case game_command_line_caps::int_setting: {
      setting.value = 0;
      break;
    }
    case game_command_line_caps::float_setting: {
      setting.value = 0.0f;
      break;
    }
    case game_command_line_caps::flag_setting: {
      setting.value = false;
      break;
    }
    case game_command_line_caps::computed_setting:
      [[fallthrough]];
    case game_command_line_caps::unknown: {
      break;
    }
    }
  };

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
      if (i == 1 && !has_ip)
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
        .setting_name = hosting_pref_name,
        .type = game_command_line_caps::env_setting,
        .value = std::wstring{ pref_options[1] },
        .display_name = L"Hosting",
        .group_id = 1,
        .get_predefined_string = [real_options = std::move(real_options), results = std::vector<siege::platform::predefined_string>{}](auto name) mutable -> std::span<siege::platform::predefined_string> {
          if (name == hosting_pref_name)
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

    if (controller.can_support_zero_tier() && controller.has_zero_tier_extension())
    {
      launch_settings.emplace_back(game_setting{
        .setting_name = L"ZERO_TIER_ENABLED",
        .type = game_command_line_caps::env_setting,
        .value = !settings.last_zero_tier_network_id.empty() ? 1 : 0,
        .display_name = L"Enable Zero Tier",
        .group_id = 1,
        .get_predefined_int = [results = std::vector<siege::platform::predefined_int>{}](auto name) mutable -> std::span<siege::platform::predefined_int> {
          if (name == L"ZERO_TIER_ENABLED")
          {
            if (!results.empty())
            {
              return results;
            }

            results.emplace_back(siege::platform::predefined_int{
              .label = L"Yes",
              .value = 1 });

            results.emplace_back(siege::platform::predefined_int{
              .label = L"No",
              .value = 0 });
          }
          return std::span<siege::platform::predefined_int>{};
        } });

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

      if (!has_ip)
      {
        launch_settings.emplace_back(game_setting{
          .setting_name = zt_fallback_ip,
          .type = game_command_line_caps::env_setting,
          .value = settings.last_ip_address.data(),
          .display_name = L"Fallback Broadcast IP",
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

        if (!listen_setting.empty() && setting == listen_setting)
        {
          listen_setting_type = game_command_line_caps::string_setting;
        }
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
      column.sub_items = { std::visit(convert_to_string, setting.value) };
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
            else if ((setting.type == game_command_line_caps::int_setting || setting.type == game_command_line_caps::env_setting) && setting.get_predefined_int)
            {
              if (auto values = setting.get_predefined_int(setting.setting_name); !values.empty())
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

              launch_table_edit_unbind = launch_table_combo.bind_cbn_sel_change([this, info, setting](auto, const auto&) {
                std::fill_n(text.data(), text.size(), L'\0');
                ::COMBOBOXEXITEMW new_item{
                  .mask = CBEIF_LPARAM | CBEIF_TEXT,
                  .iItem = ::SendMessage(launch_table_combo, CB_GETCURSEL, 0, 0),
                  .pszText = text.data(),
                  .cchTextMax = (int)text.size(),
                };

                if (::SendMessageW(launch_table_combo, CBEM_GETITEMW, 0, (LPARAM)&new_item))
                {
                  if (setting.get_predefined_string)
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

              if (setting.setting_name == zt_fallback_ip || (controller.has_extension_module() && controller.get_extension().caps->ip_connect_setting != nullptr && setting.setting_name == controller.get_extension().caps->ip_connect_setting))
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
      auto backends = controller.has_extension_module() ? controller.get_extension().controller_input_backends : std::span<const wchar_t*>{};
      auto actions = controller.has_extension_module() ? controller.get_extension().game_actions : std::span<siege::platform::game_action>{};
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

      if (!actions.empty())
      {
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
      }

      ::LVITEMW info{
        .mask = LVIF_PARAM
      };

      auto final_launch_settings = launch_settings;

      for (auto i = 0; i < launch_table.GetItemCount(); ++i)
      {
        try
        {
          info.iItem = i;
          ListView_GetItem(launch_table, &info);


          if (!info.lParam)
          {
            continue;
          }

          std::wstring value(255, L'\0');
          ListView_GetItemText(launch_table, i, 1, value.data(), value.size());
          value.resize(value.find(L'\0'));

          auto& setting = final_launch_settings.at(info.lParam);

          switch (setting.type)
          {
          case game_command_line_caps::env_setting: {
            setting.value = value;
            break;
          }
          case game_command_line_caps::string_setting: {
            setting.value = value;
            break;
          }
          case game_command_line_caps::int_setting: {
            setting.value = std::stoi(value);
            break;
          }
          case game_command_line_caps::float_setting: {
            setting.value = std::stof(value);
            break;
          }
          case game_command_line_caps::flag_setting: {
            setting.value = value == L"Enabled";
            break;
          }
          case game_command_line_caps::computed_setting:
            [[fallthrough]];
          case game_command_line_caps::unknown: {
            break;
          }
          }
        }
        catch (...)
        {
        }
      }

      final_launch_settings.erase(final_launch_settings.begin());

      siege::platform::persistent_game_settings settings{};

      siege::platform::game_command_line_caps default_caps;
      auto& caps = controller.has_extension_module() && controller.get_extension().caps ? *controller.get_extension().caps : default_caps;

      std::wstring_view player_name_setting = caps.player_name_setting ? caps.player_name_setting : L"";
      std::wstring_view ip_setting = caps.ip_connect_setting ? caps.ip_connect_setting : L"";
      constexpr static std::wstring_view zt_network_id = L"ZERO_TIER_NETWORK_ID";

      auto copy_to_array = [](auto& raw_value, auto& array) {
        auto value = std::visit(convert_to_string, raw_value);
        auto max_size = value.size() > array.size() ? array.size() : value.size();
        std::copy_n(value.data(), max_size, array.data());
      };

      if (auto setting_iter = std::find_if(final_launch_settings.begin(), final_launch_settings.end(), [&](game_setting& setting) {
            return !player_name_setting.empty() && setting.setting_name == player_name_setting;
          });
        setting_iter != final_launch_settings.end())
      {
        copy_to_array(setting_iter->value, settings.last_player_name);
      }

      if (auto setting_iter = std::find_if(final_launch_settings.begin(), final_launch_settings.end(), [&](game_setting& setting) {
            if (ip_setting.empty())
            {
              return setting.setting_name == zt_fallback_ip;
            }

            return !ip_setting.empty() && setting.setting_name == ip_setting;
          });
        setting_iter != final_launch_settings.end())
      {
        copy_to_array(setting_iter->value, settings.last_ip_address);
      }

      if (auto setting_iter = std::find_if(final_launch_settings.begin(), final_launch_settings.end(), [&](game_setting& setting) {
            return setting.setting_name == zt_network_id;
          });
        setting_iter != final_launch_settings.end())
      {
        copy_to_array(setting_iter->value, settings.last_zero_tier_network_id);
      }
      controller.set_game_settings(settings);


      if (auto setting_iter = std::find_if(final_launch_settings.begin(), final_launch_settings.end(), [&](game_setting& setting) {
            return setting.setting_name == hosting_pref_name;
          });
        setting_iter != final_launch_settings.end())
      {
        auto value = std::visit(convert_to_string, setting_iter->value);

        if (caps.ip_connect_setting)
        {
          auto connect_setting = std::find_if(final_launch_settings.begin(), final_launch_settings.end(), [&](game_setting& setting) {
            return setting.type == game_command_line_caps::string_setting && setting.setting_name == caps.ip_connect_setting;
          });

          auto should_connect = value == pref_options[1];// connect to server

          if (connect_setting != final_launch_settings.end() && !should_connect)
          {
            final_launch_settings.erase(connect_setting);
          }
        }

        if (listen_setting_type && caps.listen_setting)
        {
          auto listen_setting = std::find_if(final_launch_settings.begin(), final_launch_settings.end(), [&](game_setting& setting) {
            return setting.setting_name == caps.listen_setting;
          });

          bool enabled = value == pref_options[2];

          if (listen_setting != final_launch_settings.end())
          {
            if (enabled)
            {
              enable_setting(*listen_setting);
            }
          }
          else
          {
            auto& back = final_launch_settings.emplace_back();
            back.setting_name = caps.listen_setting;
            back.type = *listen_setting_type;
            enabled ? enable_setting(back) : disable_setting(back);
          }
        }

        if (dedicated_setting_type && caps.dedicated_setting)// dedicated
        {
          auto dedicated_setting = std::find_if(final_launch_settings.begin(), final_launch_settings.end(), [&](game_setting& setting) {
            return setting.setting_name == caps.dedicated_setting;
          });

          auto enabled = value == pref_options[3];

          if (dedicated_setting != final_launch_settings.end())
          {
            enabled ? enable_setting(*dedicated_setting) : disable_setting(*dedicated_setting);
          }
          else
          {
            auto& back = final_launch_settings.emplace_back();
            back.setting_name = caps.dedicated_setting;
            back.type = *dedicated_setting_type;
            enabled ? enable_setting(back) : disable_setting(back);
          }
        }
      }

      auto string_index = 0;
      auto env_index = 0;
      auto int_index = 0;
      auto float_index = 0;
      auto flag_index = 0;

      for (auto& setting : final_launch_settings)
      {
        try
        {
          switch (setting.type)
          {
          case game_command_line_caps::env_setting: {
            if (auto* value = std::get_if<std::wstring>(&setting.value); value)
            {
              game_args->environment_settings.at(env_index).name = setting.setting_name.c_str();
              game_args->environment_settings[env_index++].value = value->c_str();
            }
            break;
          }
          case game_command_line_caps::string_setting: {
            if (auto* value = std::get_if<std::wstring>(&setting.value); value)
            {
              game_args->string_settings.at(string_index).name = setting.setting_name.c_str();
              game_args->string_settings[string_index++].value = value->c_str();
            }
            break;
          }
          case game_command_line_caps::int_setting: {
            if (auto* value = std::get_if<int>(&setting.value); value)
            {
              game_args->int_settings.at(int_index).name = setting.setting_name.c_str();
              game_args->int_settings[int_index++].value = *value;
            }
            break;
          }
          case game_command_line_caps::float_setting: {
            if (auto* value = std::get_if<float>(&setting.value); value)
            {
              game_args->float_settings.at(float_index).name = setting.setting_name.c_str();
              game_args->float_settings[float_index++].value = *value;
            }
            break;
          }
          case game_command_line_caps::flag_setting: {
            if (auto* value = std::get_if<bool>(&setting.value); value && *value)
            {
              game_args->flags.at(flag_index++) = setting.setting_name.c_str();
            }
            break;
          }
          case game_command_line_caps::computed_setting:
            [[fallthrough]];
          case game_command_line_caps::unknown: {
            break;
          }
          }
        }
        catch (...)
        {
        }
      }

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

      return TRUE;
    }
    return FALSE;
  }
}// namespace siege::views