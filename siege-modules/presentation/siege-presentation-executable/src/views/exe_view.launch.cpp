
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/extension/input_filter.hpp>
#include "input_injector.hpp"
#include "exe_view.hpp"
#include <detours.h>
#include <imagehlp.h>
#include <xinput.h>

using game_command_line_caps = siege::platform::game_command_line_caps;

namespace siege::views
{
  void exe_view::launch_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message)
  {
    POINT point;

    if (!(::GetCursorPos(&point) && ::ScreenToClient(launch_table, &point)))
    {
      return;
    }

    LVHITTESTINFO info{};
    info.pt = point;
    info.flags = LVHT_ONITEM;

    if (ListView_SubItemHitTest(launch_table, &info) == -1)
    {
      return;
    }

    if (info.iSubItem == 0)
    {
      return;
    }

    RECT temp_rect;

    if (!ListView_GetSubItemRect(launch_table, info.iItem, info.iSubItem, LVIR_BOUNDS, &temp_rect))
    {
      return;
    }

    auto result = launch_table.MapWindowPoints(*this, temp_rect);

    if (!result)
    {
      return;
    }

    static std::array<wchar_t, 256> text{};

    ::LVITEMW item{
      .mask = LVIF_PARAM,
      .iItem = info.iItem
    };

    ListView_GetItem(launch_table, &item);

    auto setting = get_game_setting(state, (std::size_t)item.lParam - 1);

    if (!setting)
    {
      return;
    }

    ListView_GetItemText(launch_table, info.iItem, 0, text.data(), text.size());

    bool uses_combo = false;

    std::wstring temp;
    if ((setting->get().type == extension_setting_type::string_setting || setting->get().type == extension_setting_type::env_setting) && setting->get().get_predefined_string)
    {
      if (auto values = setting->get().get_predefined_string(setting->get().setting_name); !values.empty())
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
    else if ((setting->get().type == extension_setting_type::int_setting || setting->get().type == extension_setting_type::env_setting) && setting->get().get_predefined_int)
    {
      if (auto values = setting->get().get_predefined_int(setting->get().setting_name); !values.empty())
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
          if (setting->get().get_predefined_string)
          {
            setting->get().update_value(new_item.lParam ? (wchar_t*)new_item.lParam : L"", text.data());
          }
          else
          {
            setting->get().update_value((int)new_item.lParam, text.data());
          }
        }
      });
    }
    else if (setting->get().type != extension_setting_type::computed_setting)
    {
      launch_table_combo.SetWindowStyle(launch_table_combo.GetWindowStyle() & ~WS_VISIBLE);

      if (setting->get().setting_name == zt_fallback_ip || (has_extension_module(state) && get_extension(state).caps->ip_connect_setting != nullptr && setting->get().setting_name == get_extension(state).caps->ip_connect_setting))
      {
        launch_table_ip_address.SetWindowPos(result->second);
        launch_table_ip_address.SetWindowPos(HWND_TOP);
        launch_table_ip_address.SetWindowStyle(launch_table_ip_address.GetWindowStyle() | WS_VISIBLE);

        ListView_GetItemText(launch_table, info.iItem, info.iSubItem, text.data(), text.size());

        ::SetWindowTextW(launch_table_ip_address, text.data());

        launch_table_edit_unbind = launch_table_ip_address.bind_en_kill_focus([this, info, setting](auto, const auto&) {
          launch_table_ip_address.SetWindowStyle(launch_table_ip_address.GetWindowStyle() & ~WS_VISIBLE);
          ::GetWindowTextW(launch_table_ip_address, text.data(), (int)text.size());
          setting->get().update_value(text.data());
        });
      }
      else
      {
        launch_table_edit.SetWindowPos(result->second);
        launch_table_edit.SetWindowPos(HWND_TOP);
        launch_table_edit.SetWindowStyle(launch_table_edit.GetWindowStyle() | WS_VISIBLE | WS_BORDER);


        ListView_GetItemText(launch_table, info.iItem, info.iSubItem, text.data(), text.size());

        ::SetWindowTextW(launch_table_edit, text.data());


        launch_table_edit_unbind = launch_table_edit.bind_en_kill_focus([this, info, setting](auto, const auto&) {
          launch_table_edit.SetWindowStyle(launch_table_edit.GetWindowStyle() & ~WS_VISIBLE);
          ::GetWindowTextW(launch_table_edit, text.data(), (int)text.size());

          setting->get().update_value(text.data());
        });
      }
    }
  }

  BOOL exe_view::exe_actions_nm_click(win32::tool_bar, const NMMOUSE& message)
  {
    ::SetFocus(this->exe_actions);
    if (message.dwItemSpec == add_to_firewall_selected_id)
    {
      auto path = get_exe_path(state);

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
      auto backends = has_extension_module(state) ? get_extension(state).controller_input_backends : std::span<const wchar_t*>{};
      auto actions = has_extension_module(state) ? get_extension(state).game_actions : std::span<siege::platform::game_action>{};
      std::size_t binding_index = 0;
      auto& game_args = get_final_args(state);


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
              game_args.action_bindings[binding_index].vkey = virtual_key;
              game_args.action_bindings[binding_index].action_name = action.action_name;

              game_args.action_bindings[binding_index].hardware_index = MapVirtualKeyW(virtual_key, MAPVK_VK_TO_VSC);
              game_args.action_bindings[binding_index++].context = context;
            }
            else
            {
              game_args.action_bindings[binding_index].vkey = virtual_key;
              game_args.action_bindings[binding_index].action_name = action.action_name;
              game_args.action_bindings[binding_index++].context = context;
            }
          }
        }
      }

      // Storing controller actions after keyboard/mouse actions is a workaround
      // to make Quake 3 based games work. The left analog stick is bound
      // to the same names as the arrow keys, so we want the controller settings to have higher priority.
      // Maybe this is not a bad idea in general, but it's better to have something more explicit for the user to control.
      if (backends.empty())
      {
        for (auto i = 0; i < controller_table.GetItemCount(); ++i)
        {
          auto item = controller_table.GetItem(LVITEMW{
            .mask = LVIF_PARAM,
            .iItem = i });

          if (item && item->lParam)
          {
            auto iter = std::find_if(game_args.controller_to_send_input_mappings.begin(), game_args.controller_to_send_input_mappings.end(), [](auto& item) {
              return item.from_vkey == 0;
            });

            if (iter != game_args.controller_to_send_input_mappings.end())
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

        // Raw input doesn't work properly under remote desktop :(
        bool has_controllers = std::any_of(controllers.begin(), controllers.end(), [](auto& entry) {
          return entry.dwType == RIM_TYPEHID;
        });

        bool has_xinput_controllers = false;

        if (!has_controllers)
        {
          auto version_and_name = win32::get_xinput_version();

          if (version_and_name)
          {
            win32::module xinput_module;
            xinput_module.reset(::LoadLibraryExW(version_and_name->second.data(), nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0));

            std::add_pointer_t<decltype(::XInputGetState)> xinput_get_state = xinput_module.GetProcAddress<decltype(xinput_get_state)>("XInputGetState");
            XINPUT_STATE temp{};
            has_xinput_controllers = xinput_get_state(0, &temp) == S_OK;
          }
        }

        auto binding_count = controller_table.GetItemCount();
        if (!(has_controllers || has_xinput_controllers))
        {
          binding_count = 0;
        }

        for (auto i = 0; i < binding_count; ++i)
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


              auto hardware_index = hardware_index_for_xbox_vkey(virtual_key);

              if (has_controllers)
              {
                hardware_index = hardware_index_for_controller_vkey(std::span<RAWINPUTDEVICELIST>(controllers.data(), size), 0, virtual_key);
              }

              auto key_name = string_for_vkey(virtual_key, hardware_index.first);

              game_args.action_bindings[binding_index].vkey = virtual_key;
              game_args.action_bindings[binding_index].action_name = action.action_name;
              game_args.action_bindings[binding_index].context = hardware_index.first;
              game_args.action_bindings[binding_index++].hardware_index = hardware_index.second;
            }
            catch (...)
            {
            }
          }
        }
      }
      ::LVITEMW info{
        .mask = LVIF_PARAM
      };

      auto existing_state = ::SendMessageW(exe_actions, TB_GETSTATE, launch_selected_id, 0);

      existing_state &= ~TBSTATE_ENABLED;
      ::SendMessageW(exe_actions, TB_SETSTATE, launch_selected_id, MAKEWORD(existing_state, 0));

      auto global = ::CreateFileMappingW(
        INVALID_HANDLE_VALUE,// use paging file
        NULL,// default security
        PAGE_READWRITE,// read/write access
        0,// maximum object size (high-order DWORD)
        256,// maximum object size (low-order DWORD)
        L"ZeroTierCurrentIpGlobalHandle");

      if (global)
      {
        ::SetEnvironmentVariableW(L"ZERO_TIER_CURRENT_IP_GLOBAL_HANDLE", L"ZeroTierCurrentIpGlobalHandle");
      }

      injector.emplace(*this, input_injector_args{ .args = game_args, .launch_game_with_extension = [this](const auto* args, auto* process_info) -> HRESULT { return launch_game_with_extension(state, args, process_info); }, .on_process_closed = [this, existing_state, global] mutable {
                                                    existing_state |= TBSTATE_ENABLED;
                                                    ::SendMessageW(exe_actions, TB_SETSTATE, launch_selected_id, MAKEWORD(existing_state, 0));
                                                    injector.reset();

                                                    if (global)
                                                    {
                                                      auto data = ::MapViewOfFile(global, FILE_MAP_READ, 0, 0, 256);

                                                      if (data)
                                                      {
                                                        std::string ip_address;
                                                        ip_address.resize(256);
                                                        std::memcpy(ip_address.data(), data, 256);

                                                        auto end = ip_address.find('\0');

                                                        if (end != std::string::npos)
                                                        {
                                                          ip_address.resize(end);
                                                        }

                                                        if (ip_address.contains("."))
                                                        {
                                                          set_ip_for_current_network(state, ip_address);
                                                        }

                                                        ::UnmapViewOfFile(data);
                                                      }
                                                      ::CloseHandle(global);
                                                      ::SetEnvironmentVariableW(L"ZERO_TIER_CURRENT_IP_GLOBAL_HANDLE", nullptr);
                                                    } }

                              });


      return TRUE;
    }
    return FALSE;
  }
}// namespace siege::views