module;
#include <siege/platform/windows.hpp>
#include <cassert>
#include <CommCtrl.h>

module exe.views;

import std;
import win32.com;
import win32.window;
import win32.drawing;
import win32.dialog;

import win32.common_controls;
import win32.theming;

import siege.extension.input_filter;

import exe.input_injector;
import exe.shared;

using game_command_line_caps = siege::platform::game_command_line_caps;
namespace stl = std::ranges;

namespace siege::views
{
  using vkey = siege::platform::vkey;
  void apply_launch_button_state(exe_view& self);

  decltype(exe_view::launch) exe_view::create_launch_controls()
  {
    decltype(exe_view::launch) launch{};
    launch.launch_table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
      .style = WS_CHILD | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

    launch.launch_table.InsertColumn(-1, LVCOLUMNW{
                                           .pszText = const_cast<wchar_t*>(L"Name"),
                                         });

    launch.launch_table.InsertColumn(-1, LVCOLUMNW{
                                           .pszText = const_cast<wchar_t*>(L"Value"),
                                         });

    launch.launch_table.EnableGroupView(true);

    launch.launch_table.InsertGroup(-1, LVGROUP{
                                          .pszHeader = const_cast<wchar_t*>(L"Multiplayer Options"),
                                          .iGroupId = static_cast<int>(setting_group::networking),
                                          .state = LVGS_COLLAPSIBLE,
                                        });

    launch.launch_table.InsertGroup(-1, LVGROUP{
                                          .pszHeader = const_cast<wchar_t*>(L"Graphics Options"),
                                          .iGroupId = static_cast<int>(setting_group::graphics),
                                          .state = LVGS_COLLAPSIBLE,
                                        });

    launch.launch_table.InsertGroup(-1, LVGROUP{
                                          .pszHeader = const_cast<wchar_t*>(L"Input Options"),
                                          .iGroupId = static_cast<int>(setting_group::input),
                                          .state = LVGS_COLLAPSIBLE,
                                        });

    launch.launch_table.InsertGroup(-1, LVGROUP{
                                          .pszHeader = const_cast<wchar_t*>(L"Sound Options"),
                                          .iGroupId = static_cast<int>(setting_group::sound),
                                          .state = LVGS_COLLAPSIBLE,
                                        });

    launch.launch_table.InsertGroup(-1, LVGROUP{
                                          .pszHeader = const_cast<wchar_t*>(L"Other Options"),
                                          .iGroupId = static_cast<int>(setting_group::other),
                                          .state = LVGS_COLLAPSIBLE,
                                        });

    launch.launch_table_edit = *win32::CreateWindowExW<win32::edit>({ .hwndParent = *this, .style = WS_CHILD | ES_AUTOHSCROLL });
    launch.launch_table_combo = *win32::CreateWindowExW<win32::combo_box_ex>({ .hwndParent = *this, .cy = 300, .cx = 300, .style = WS_CHILD | CBS_DROPDOWNLIST });

    launch.launch_table_ip_address = *win32::CreateWindowExW<win32::ip_address_edit>({ .hwndParent = *this, .cy = 100, .cx = 300, .style = WS_CHILD });

    launch.launch_table.bind_nm_click([this](win32::list_view launch_table, const NMITEMACTIVATE& message) {
      POINT point;

      if (!(::GetCursorPos(&point) && ::ScreenToClient(launch_table, &point)))
      {
        return;
      }

      LVHITTESTINFO info{};
      info.pt = point;
      info.flags = LVHT_ONITEM;

      if (launch_table.SubItemHitTest(info) == -1)
      {
        return;
      }

      if (info.iSubItem == 0)
      {
        return;
      }

      auto temp_rect = launch_table.GetSubItemRect(info.iItem, info.iSubItem, LVIR_BOUNDS);
      auto client = launch_table.GetClientRect();

      if (!temp_rect || !client)
      {
        return;
      }

      ::IntersectRect(&*temp_rect, &*temp_rect, &*client);

      auto result = launch_table.MapWindowPoints(*this, *temp_rect);

      if (!result)
      {
        return;
      }

      // Large enough for long launch arg strings (CreateProcess limit is ~32K).
      static std::array<wchar_t, 4096> text{};

      ::LVITEMW item{
        .mask = LVIF_PARAM,
        .iItem = info.iItem
      };

      item = launch_table.GetItem(item).value_or(item);

      auto setting = get_game_setting(state, (std::size_t)item.lParam - 1);

      if (!setting)
      {
        return;
      }

      launch_table.GetItemText(info.iItem, 0, text);

      bool uses_combo = false;

      std::wstring temp;
      if ((setting->get().type == extension_setting_type::string_setting || setting->get().type == extension_setting_type::env_setting) && setting->get().get_predefined_string)
      {
        if (auto values = setting->get().get_predefined_string(setting->get().setting_name); !values.empty())
        {
          this->launch.launch_table_combo.ResetContent();

          for (auto& item : values)
          {
            uses_combo = true;
            temp = item.label;
            this->launch.launch_table_combo.InsertItem(::COMBOBOXEXITEMW{
              .mask = CBEIF_LPARAM | CBEIF_TEXT,
              .iItem = -1,
              .pszText = temp.data(),
              .cchTextMax = (int)temp.size(),
              .lParam = (LPARAM)item.value });
          }
        }
      }
      else if ((setting->get().type == extension_setting_type::int_setting || setting->get().type == extension_setting_type::env_setting) && setting->get().get_predefined_int)
      {
        if (auto values = setting->get().get_predefined_int(setting->get().setting_name); !values.empty())
        {
          this->launch.launch_table_combo.ResetContent();

          for (auto& item : values)
          {
            uses_combo = true;
            temp = item.label;
            this->launch.launch_table_combo.InsertItem(::COMBOBOXEXITEMW{
              .mask = CBEIF_LPARAM | CBEIF_TEXT,
              .iItem = -1,
              .pszText = temp.data(),
              .cchTextMax = (int)temp.size(),
              .lParam = (LPARAM)item.value });
          }
        }
      }

      if (this->launch.launch_table_post_edit_unbind)
      {
        this->launch.launch_table_post_edit_unbind();
        this->launch.launch_table_post_edit_unbind = nullptr;
      }

      ::ShowWindow(this->launch.launch_table_ip_address, SW_HIDE);

      if (uses_combo)
      {
        this->launch.launch_table_combo.SetWindowPos(result->second);
        this->launch.launch_table_combo.SetWindowPos(HWND_TOP);
        this->launch.launch_table_combo.SetWindowStyle(this->launch.launch_table_combo.GetWindowStyle() | WS_VISIBLE);
        this->launch.launch_table_combo.ShowDropDown();

        auto unbind_update = this->launch.launch_table_combo.bind_cbn_sel_change([this, info, setting](auto, const auto&) {
          std::fill_n(text.data(), text.size(), L'\0');
          auto selected_index = this->launch.launch_table_combo.GetCurSel();

          if (!selected_index)
          {
            return;
          }

          if (auto fetched = this->launch.launch_table_combo.GetItem(::COMBOBOXEXITEMW{
                .mask = CBEIF_LPARAM | CBEIF_TEXT,
                .iItem = *selected_index,
                .pszText = text.data(),
                .cchTextMax = (int)text.size() }))
          {
            if (setting->get().get_predefined_string)
            {
              setting->get().update_value(fetched->lParam ? (wchar_t*)fetched->lParam : L"", text.data());
            }
            else
            {
              setting->get().update_value((int)fetched->lParam, text.data());
            }

            apply_launch_button_state(*this);
          }
        });

        auto unbind_close = this->launch.launch_table_combo.bind_cbn_close_up([this](auto, const auto&) {
          ::ShowWindow(this->launch.launch_table_combo, SW_HIDE);
        });

        this->launch.launch_table_post_edit_unbind = [unbind_update = std::move(unbind_update), unbind_close = std::move(unbind_close)]() {
          assert(unbind_update);
          unbind_update();

          assert(unbind_close);
          unbind_close();
        };
      }
      else if (setting->get().type != extension_setting_type::computed_setting)
      {
        this->launch.launch_table_combo.SetWindowStyle(this->launch.launch_table_combo.GetWindowStyle() & ~WS_VISIBLE);

        if (setting->get().setting_name == zt_fallback_ip || (has_extension_module(state) && get_extension(state).caps()->ip_connect_setting != nullptr && setting->get().setting_name == get_extension(state).caps()->ip_connect_setting))
        {
          this->launch.launch_table_ip_address.SetWindowPos(result->second);
          this->launch.launch_table_ip_address.SetWindowPos(HWND_TOP);
          this->launch.launch_table_ip_address.SetWindowStyle(this->launch.launch_table_ip_address.GetWindowStyle() | WS_VISIBLE);

          launch_table.GetItemText(info.iItem, info.iSubItem, text);

          ::SetWindowTextW(this->launch.launch_table_ip_address, text.data());

          this->launch.launch_table_post_edit_unbind = this->launch.launch_table_ip_address.bind_en_kill_focus([this, info, setting](auto, const auto&) {
            ::GetWindowTextW(this->launch.launch_table_ip_address, text.data(), (int)text.size());
            setting->get().update_value(text.data());

            ::ShowWindow(this->launch.launch_table_ip_address, SW_HIDE);
          });
        }
        else
        {
          this->launch.launch_table_edit.SetWindowPos(result->second);
          this->launch.launch_table_edit.SetWindowPos(HWND_TOP);
          this->launch.launch_table_edit.SetWindowStyle(this->launch.launch_table_edit.GetWindowStyle() | WS_BORDER);
          ::ShowWindow(this->launch.launch_table_edit, SW_SHOW);

          launch_table.GetItemText(info.iItem, info.iSubItem, text);

          ::SetWindowTextW(this->launch.launch_table_edit, text.data());

          auto unbind_update = this->launch.launch_table_edit.bind_en_change([this, info, setting](auto, const auto&) {
            ::GetWindowTextW(this->launch.launch_table_edit, text.data(), (int)text.size());
            setting->get().update_value(text.data());
          });

          auto unbind_focus = this->launch.launch_table_edit.bind_en_kill_focus([this, info, setting](auto, const auto&) {
            ::ShowWindow(this->launch.launch_table_edit, SW_HIDE);
          });

          this->launch.launch_table_post_edit_unbind = [unbind_update = std::move(unbind_update), unbind_focus = std::move(unbind_focus)]() {
            assert(unbind_update);
            unbind_update();

            assert(unbind_focus);
            unbind_focus();
          };
        }
      }
    });

    using siege::platform::hardware_context;
    using siege::platform::hardware_context_caps;

    exe_actions.bind_nm_click([this](win32::tool_bar exe_actions, const NMMOUSE& message) {
      if (message.dwItemSpec != this->launch.launch_selected_id)
      {
        return false;
      }
      auto user_intent = create_user_intent(state);
      auto config_intent = has_extension_module(state) ? generate_config(user_intent, get_extension(state)) : generate_config(user_intent);

      if (config_intent.should_disable_controller)
      {
        disable_controller_for_extension(state);
      }
      else
      {
        enable_controller_for_extension(state);
      }

      auto ip_name = has_client_preference(state) ? L"ZeroTierCurrentClientIpGlobalHandle" : L"ZeroTierCurrentServerIpGlobalHandle";

      auto global = ::CreateFileMappingW(
        INVALID_HANDLE_VALUE,// use paging file
        NULL,// default security
        PAGE_READWRITE,// read/write access
        0,// maximum object size (high-order DWORD)
        256,// maximum object size (low-order DWORD)
        ip_name);

      if (global)
      {
        ::SetEnvironmentVariableW(L"SIEGE_WSOCK_CURRENT_IP_GLOBAL_HANDLE", ip_name);
      }

      auto game_args = get_packaged_args(state);
      game_args.action_bindings = std::move(config_intent.action_bindings);
      game_args.controller_to_send_input_mappings = config_intent.controller_to_send_input_mappings;
      game_args.controller_to_send_input_mappings.resize(256);

      auto simulate_mouse = stl::any_of(game_args.environment_settings, [&](const auto& setting) {
        if (has_extension_module(state))
        {
          return true;
        }

        if (!setting.name)
        {
          return false;
        }

        if (!setting.value)
        {
          return false;
        }

        return setting.name == mouse_simulation_enabled_env && (setting.value == L"Yes"sv || setting.value == L"1"sv);
      });
      auto simulate_keyboard = stl::any_of(game_args.environment_settings, [&](const auto& setting) {
        if (has_extension_module(state))
        {
          return true;
        }

        if (!setting.name)
        {
          return false;
        }

        if (!setting.value)
        {
          return false;
        }

        return setting.name == keyboard_simulation_enabled_env && (setting.value == L"Yes"sv || setting.value == L"1"sv);
      });


      double mouse_sensitivity = 750.0;
      for (auto& setting : game_args.environment_settings)
      {
        if (setting.name && setting.value && std::wstring_view(setting.name) != mouse_sensitivity_env)
        {
          continue;
        }

        try
        {
          mouse_sensitivity = std::stod(std::wstring(setting.value));
        }
        catch (...)
        {
        }
        break;
      }

      double refresh_rate_hz = [this] {
        MONITORINFOEXW monitor_info{};
        monitor_info.cbSize = sizeof(monitor_info);
        DEVMODEW display_mode{
          .dmSize = sizeof(DEVMODEW)
        };

        if (::GetMonitorInfoW(::MonitorFromWindow(ref(), MONITOR_DEFAULTTONEAREST), &monitor_info)
            && ::EnumDisplaySettingsW(monitor_info.szDevice, ENUM_CURRENT_SETTINGS, &display_mode)
            && display_mode.dmDisplayFrequency > 1)
        {
          return (double)display_mode.dmDisplayFrequency;
        }
        return 60.0;
      }();

      this->launch.injector = bind_to_window(ref(), input_injector_args{ .args = std::move(game_args), .simulate_mouse = simulate_mouse, .simulate_keyboard = simulate_keyboard, .launch_game_with_extension = [this](auto& args, auto* process_info) -> HRESULT { return launch_game_with_extension(state, args, process_info); }, .on_process_closed = [this, global, for_client = has_client_preference(state)] mutable {
                                                    reset_launch_state(state);
                                                    apply_launch_button_state(*this);
                                                    auto _ = std::shared_ptr<void>{
                                                      nullptr, [this](...) {
                                                        this->launch.injector.reset();
                                                      }
                                                    };
                                                    if (!global)
                                                    {
                                                      return;
                                                    } 
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
                                                          set_ip_for_current_network(state, ip_address, for_client);
                                                        }

                                                        ::UnmapViewOfFile(data);
                                                      }
                                                      ::CloseHandle(global);
                                                      ::SetEnvironmentVariableW(L"SIEGE_WSOCK_CURRENT_IP_GLOBAL_HANDLE", nullptr); }

                                                      ,
                                                      .refresh_rate_hz = refresh_rate_hz,
                                                      .max_turn_speed = mouse_sensitivity });

      disable_launch(state);
      apply_launch_button_state(*this);

      return true;
    });

    return launch;
  }

  void apply_launch_button_state(exe_view& self)
  {
    auto btn_state = self.exe_actions.GetState(self.launch.launch_selected_id);
    if (is_launch_enabled(self.state))
    {
      btn_state |= TBSTATE_ENABLED;
    }
    else
    {
      btn_state &= ~TBSTATE_ENABLED;
    }
    self.exe_actions.SetState(self.launch.launch_selected_id, btn_state);
  }
}// namespace siege::views
