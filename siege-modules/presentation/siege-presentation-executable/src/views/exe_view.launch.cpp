
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/extension/input_filter.hpp>
#include "exe_view.hpp"
#include <detours.h>
#include <imagehlp.h>
#include <xinput.h>
#include "input_injector.hpp"

using game_command_line_caps = siege::platform::game_command_line_caps;
namespace stl = std::ranges;

namespace siege::views
{
  decltype(exe_view::launch) exe_view::create_launch_controls()
  {
    decltype(exe_view::launch) launch{};
    launch.launch_table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
      .style = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

    launch.launch_table.InsertColumn(-1, LVCOLUMNW{
                                           .pszText = const_cast<wchar_t*>(L"Name"),
                                         });

    launch.launch_table.InsertColumn(-1, LVCOLUMNW{
                                           .pszText = const_cast<wchar_t*>(L"Value"),
                                         });

    launch.launch_table.EnableGroupView(true);

    launch.launch_table.InsertGroup(-1, LVGROUP{
                                          .pszHeader = const_cast<wchar_t*>(L"Multiplayer Options"),
                                          .iGroupId = 1,
                                          .state = LVGS_COLLAPSIBLE,
                                        });
    launch.launch_table.InsertGroup(-1, LVGROUP{
                                          .pszHeader = const_cast<wchar_t*>(L"Other Options"),
                                          .iGroupId = 2,
                                          .state = LVGS_COLLAPSIBLE,
                                        });

    launch.launch_table_edit = *win32::CreateWindowExW<win32::edit>({ .hwndParent = *this, .style = WS_CHILD });
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
          ::SendMessageW(this->launch.launch_table_combo, CB_RESETCONTENT, 0, 0);

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
            ::SendMessageW(this->launch.launch_table_combo, CBEM_INSERTITEMW, 0, (LPARAM)&new_item);
          }
        }
      }
      else if ((setting->get().type == extension_setting_type::int_setting || setting->get().type == extension_setting_type::env_setting) && setting->get().get_predefined_int)
      {
        if (auto values = setting->get().get_predefined_int(setting->get().setting_name); !values.empty())
        {
          ::SendMessageW(this->launch.launch_table_combo, CB_RESETCONTENT, 0, 0);

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
            ::SendMessageW(this->launch.launch_table_combo, CBEM_INSERTITEMW, 0, (LPARAM)&new_item);
          }
        }
      }

      if (this->launch.launch_table_edit_unbind)
      {
        this->launch.launch_table_edit_unbind();
        this->launch.launch_table_edit_unbind = nullptr;
      }

      if (uses_combo)
      {
        this->launch.launch_table_combo.SetWindowPos(result->second);
        this->launch.launch_table_combo.SetWindowPos(HWND_TOP);
        this->launch.launch_table_combo.SetWindowStyle(this->launch.launch_table_combo.GetWindowStyle() | WS_VISIBLE);
        ::SendMessageW(this->launch.launch_table_combo, CB_SHOWDROPDOWN, TRUE, 0);

        this->launch.launch_table_edit_unbind = this->launch.launch_table_combo.bind_cbn_sel_change([this, info, setting](auto, const auto&) {
          std::fill_n(text.data(), text.size(), L'\0');
          ::COMBOBOXEXITEMW new_item{
            .mask = CBEIF_LPARAM | CBEIF_TEXT,
            .iItem = ::SendMessageW(this->launch.launch_table_combo, CB_GETCURSEL, 0, 0),
            .pszText = text.data(),
            .cchTextMax = (int)text.size(),
          };

          if (::SendMessageW(this->launch.launch_table_combo, CBEM_GETITEMW, 0, (LPARAM)&new_item))
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
        this->launch.launch_table_combo.SetWindowStyle(this->launch.launch_table_combo.GetWindowStyle() & ~WS_VISIBLE);

        if (setting->get().setting_name == zt_fallback_ip || (has_extension_module(state) && get_extension(state).caps->ip_connect_setting != nullptr && setting->get().setting_name == get_extension(state).caps->ip_connect_setting))
        {
          this->launch.launch_table_ip_address.SetWindowPos(result->second);
          this->launch.launch_table_ip_address.SetWindowPos(HWND_TOP);
          this->launch.launch_table_ip_address.SetWindowStyle(this->launch.launch_table_ip_address.GetWindowStyle() | WS_VISIBLE);

          ListView_GetItemText(launch_table, info.iItem, info.iSubItem, text.data(), text.size());

          ::SetWindowTextW(this->launch.launch_table_ip_address, text.data());

          this->launch.launch_table_edit_unbind = this->launch.launch_table_ip_address.bind_en_kill_focus([this, info, setting](auto, const auto&) {
            this->launch.launch_table_ip_address.SetWindowStyle(this->launch.launch_table_ip_address.GetWindowStyle() & ~WS_VISIBLE);
            ::GetWindowTextW(this->launch.launch_table_ip_address, text.data(), (int)text.size());
            setting->get().update_value(text.data());
          });
        }
        else
        {
          this->launch.launch_table_edit.SetWindowPos(result->second);
          this->launch.launch_table_edit.SetWindowPos(HWND_TOP);
          this->launch.launch_table_edit.SetWindowStyle(this->launch.launch_table_edit.GetWindowStyle() | WS_VISIBLE | WS_BORDER);


          ListView_GetItemText(launch_table, info.iItem, info.iSubItem, text.data(), text.size());

          ::SetWindowTextW(this->launch.launch_table_edit, text.data());


          this->launch.launch_table_edit_unbind = this->launch.launch_table_edit.bind_en_kill_focus([this, info, setting](auto, const auto&) {
            this->launch.launch_table_edit.SetWindowStyle(this->launch.launch_table_edit.GetWindowStyle() & ~WS_VISIBLE);
            ::GetWindowTextW(this->launch.launch_table_edit, text.data(), (int)text.size());

            setting->get().update_value(text.data());
          });
        }
      }
    });

    exe_actions.bind_nm_click([this](win32::tool_bar exe_actions, const NMMOUSE& message) {
      if (message.dwItemSpec != this->launch.launch_selected_id)
      {
        return false;
      }
      auto backends = has_extension_module(state) ? get_extension(state).controller_input_backends : std::span<const wchar_t*>{};
      auto actions = has_extension_module(state) ? get_extension(state).game_actions : std::span<siege::platform::game_action>{};

      auto bound_actions = get_action_bindings(state);
      auto controller_actions = std::vector(bound_actions.begin(), bound_actions.end());
      controller_actions.erase(std::remove_if(controller_actions.begin(), controller_actions.end(), [](auto& item) {
        return !is_vkey_for_controller(item.vkey);
      }),
        controller_actions.end());

      std::vector<siege::platform::input_mapping_ex> action_bindings;
      action_bindings.reserve(actions.size());

      if (!actions.empty() && has_extension_module(state))
      {
        auto& extension = get_extension(state);

        for (auto i = 0; i < input.keyboard_table.GetItemCount(); ++i)
        {
          auto item = input.keyboard_table.GetItem(LVITEMW{
            .mask = LVIF_PARAM,
            .iItem = i });

          if (item && item->lParam)
          {
            auto virtual_key = bound_actions[item->lParam].vkey;
            auto context = bound_actions[item->lParam].context;
            auto action_index = bound_actions[item->lParam].action_index;

            auto& action = actions[action_index];

            auto& binding = action_bindings.emplace_back();
            using siege::platform::hardware_context;
            using siege::platform::hardware_context_caps;

            if (context == hardware_context::keyboard || context == hardware_context::keyboard_shifted || context == hardware_context::keypad)
            {
              binding.vkey = virtual_key;
              binding.action_name = action.action_name;
              binding.hardware_index = ::MapVirtualKeyW(virtual_key, MAPVK_VK_TO_VSC);
              binding.hardware_input_type = input_type::button;
              binding.context = context;

              if (!extension.is_input_mapping_valid({ .context = context,
                                                      .button_count = context == hardware_context::keypad ? 64u : 256u },
                    binding))
              {
                action_bindings.pop_back();
              }
            }
            else
            {
              binding.vkey = virtual_key;
              binding.action_name = action.action_name;
              binding.context = context;

              auto is_mouse_button = [&] {
                return binding.vkey == VK_LBUTTON || binding.vkey == VK_RBUTTON || binding.vkey == VK_MBUTTON || binding.vkey == VK_XBUTTON1 || binding.vkey == VK_XBUTTON2;
              };

              if ((binding.context == hardware_context::mouse || binding.context == hardware_context::mouse_wheel) && !is_mouse_button())
              {
                binding.hardware_input_type = input_type::axis;

                if (binding.vkey == VK_LEFT || binding.vkey == VK_RIGHT)
                {
                  binding.hardware_index = 1;
                }
              }
              else if (is_mouse_button())
              {
                binding.hardware_input_type = input_type::button;
                binding.hardware_index = binding.vkey - VK_LBUTTON;
              }

              if (!extension.is_input_mapping_valid({
                                                      .context = context,
                                                      .button_count = binding.context == hardware_context::mouse ? 5u : 0u,
                                                      .axis_count = binding.context == hardware_context::mouse_wheel ? 1u : 2u,

                                                    },
                    binding))
              {
                action_bindings.pop_back();
              }
            }
          }
        }
      }

      std::vector<siege::platform::game_command_line_args::controller_to_send_input_mapping> controller_to_send_input_mappings;
      using siege::platform::hardware_context;

      auto add_fallback = [&](auto& bound_action, hardware_context from_context = {}) {
        auto virtual_key = bound_action.vkey;

        auto action_iter = stl::find_if(bound_actions, [&](auto& other) {
          return other.action_index == bound_action.action_index && (other.vkey != bound_action.vkey && !(other.context == bound_action.context || other.context == from_context));
        });

        if (action_iter == bound_actions.end())
        {
          return;
        }

        if (stl::any_of(controller_to_send_input_mappings, [&](auto& mapping) {
              return mapping.from_context == from_context && mapping.from_vkey == bound_action.vkey && mapping.to_context == action_iter->context && mapping.to_vkey == action_iter->vkey;
            }))
        {
          return;
        }

        auto& mapping = controller_to_send_input_mappings.emplace_back();

        mapping.from_context = from_context;
        mapping.from_vkey = bound_action.vkey;
        mapping.to_context = action_iter->context;
        mapping.to_vkey = action_iter->vkey;
      };

      // For games which have no known actions - the user manually binds controller inputs directly to the keyboard and mouse.
      if (actions.empty())
      {
        for (auto i = 0; i < input.controller_table.GetItemCount(); ++i)
        {
          auto item = input.controller_table.GetItem(LVITEMW{
            .mask = LVIF_PARAM,
            .iItem = i });

          if (!(item && item->lParam))
          {
            continue;
          }

          auto& mapping = controller_to_send_input_mappings.emplace_back();

          mapping.from_context = input.bound_inputs.at(item->lParam).from_context;
          mapping.from_vkey = input.bound_inputs.at(item->lParam).from_vkey;
          mapping.to_context = input.bound_inputs.at(item->lParam).to_context;
          mapping.to_vkey = input.bound_inputs.at(item->lParam).to_vkey;
        }
      }
      // For games with no controller mappings, we automatically map controller inputs to keyboard and mouse inputs.
      else if (backends.empty())
      {
        stl::for_each(controller_actions, add_fallback);
      }
      else if (has_extension_module(state))
      {
        auto& extension = get_extension(state);

        auto controllers = get_connected_controllers();

        if (controllers.empty())
        {
          stl::for_each(controller_actions, add_fallback);
          // TODO disable controller input
          // or trigger controller config refresh
        }
        else
        {
          const bool uses_winmm = std::count_if(backends.begin(), backends.end(), [](std::wstring_view value) {
            return value == L"winmm";
          });

          std::uint32_t controller_index = 0;
          for (auto& controller : controllers)
          {
            auto _ = std::shared_ptr<void>{
              nullptr, [&](...) { controller_index++; }
            };

            if (controller.get_hardware_index == nullptr)
            {
              continue;
            }

            if (!siege::platform::is_for_controller(controller.detected_context))
            {
              continue;
            }

            auto buttons = mapped_buttons(controller);
            auto axes = mapped_axes(controller);
            siege::platform::hardware_context_caps controller_caps{
              .context = controller.detected_context,
              .hardware_index = controller_index,
              .button_count = buttons.empty() ? 0 : *buttons.rbegin() + 1,
              .axis_count = axes.empty() ? 0 : *axes.rbegin() + 1,
              .hat_count = mapped_hat_count(controller),
            };

            for (auto& bound_action : controller_actions)
            {
              auto virtual_key = bound_action.vkey;

              auto preference = actions[bound_action.action_index].type == actions[bound_action.action_index].analog ? controller_info::prefer_axis : controller_info::prefer_button;
              auto hardware_index = controller.get_hardware_index(virtual_key, preference);

              if (!hardware_index)
              {
                add_fallback(bound_action);
                continue;
              }

              auto& binding = action_bindings.emplace_back();
              binding.vkey = virtual_key;
              binding.action_name = actions[bound_action.action_index].action_name;
              binding.context = controller.detected_context;

              if (uses_winmm)
              {
                auto mapped = map_hid_to_winmm(*hardware_index, axes);
                binding.hardware_index = mapped.index;
                binding.hardware_input_type = mapped.type;
              }
              else
              {
                binding.hardware_index = hardware_index->index;
                binding.hardware_input_type = hardware_index->type;
              }

              if (!extension.is_input_mapping_valid(controller_caps, binding))
              {
                action_bindings.pop_back();
                add_fallback(bound_action, controller.detected_context);
              }
            }
          }
        }
      }
      ::LVITEMW info{
        .mask = LVIF_PARAM
      };

      auto existing_state = ::SendMessageW(exe_actions, TB_GETSTATE, this->launch.launch_selected_id, 0);

      existing_state &= ~TBSTATE_ENABLED;
      ::SendMessageW(exe_actions, TB_SETSTATE, this->launch.launch_selected_id, MAKEWORD(existing_state, 0));

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

      auto game_args = get_packaged_args(state);
      game_args.action_bindings = std::move(action_bindings);
      game_args.controller_to_send_input_mappings = controller_to_send_input_mappings;
      game_args.controller_to_send_input_mappings.resize(256);

      this->launch.injector = bind_to_window(ref(), input_injector_args{ .args = std::move(game_args), .launch_game_with_extension = [this](auto& args, auto* process_info) -> HRESULT { return launch_game_with_extension(state, args, process_info); }, .on_process_closed = [this, existing_state, global] mutable {
                                                    existing_state |= TBSTATE_ENABLED;
                                                    ::SendMessageW(this->exe_actions, TB_SETSTATE, this->launch.launch_selected_id, MAKEWORD(existing_state, 0));
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
                                                          set_ip_for_current_network(state, ip_address);
                                                        }

                                                        ::UnmapViewOfFile(data);
                                                      }
                                                      ::CloseHandle(global);
                                                      ::SetEnvironmentVariableW(L"ZERO_TIER_CURRENT_IP_GLOBAL_HANDLE", nullptr); }

                                                    });


      return true;
    });

    return launch;
  }
}// namespace siege::views