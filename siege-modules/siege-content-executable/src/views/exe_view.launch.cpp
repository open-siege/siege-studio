
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
  void add_modify_import()
  {
    struct handler
    {
      static BOOL CALLBACK byway_callback(PVOID pContext, PCSTR pszFile, LPCSTR* ppszOutFile)
      {
        return TRUE;
      }

      static BOOL CALLBACK file_callback(PVOID pContext, LPCSTR pszOrigFile, LPCSTR file, LPCSTR* ppszOutFile)
      {
        std::ofstream log("log.txt", std::ios::app);

        if (file)
        {
          log << "Import: " << file << '\n';
        }

        if (file && std::string_view(file) == "WSOCK32.dll")
        {
          *ppszOutFile = "wsock32-on-zerotier.dll";
        }
        else
        {
          *ppszOutFile = file;
        }

        return TRUE;
      }

      static BOOL CALLBACK symbol_callback(PVOID pContext, ULONG nOrigOrdinal, ULONG nOrdinal, ULONG* pnOutOrdinal, LPCSTR original_symbol, LPCSTR pszSymbol, LPCSTR* ppszOutSymbol)
      {
        *ppszOutSymbol = pszSymbol;
        return TRUE;
      }

      static BOOL CALLBACK commit_callback(PVOID pContext)
      {
        return TRUE;
      }
    };


    // LOADED_IMAGE image{};
    // if (::MapAndLoad("ConsoleApplication1.exe",
    //     "C:\\open-siege\\ConsoleApplication1\\Release\\", &image, FALSE, FALSE))
    // {
    //   image.FileHeader->OptionalHeader.MajorOperatingSystemVersion = 4;
    //   image.FileHeader->OptionalHeader.MinorOperatingSystemVersion = 0;
    //   image.FileHeader->OptionalHeader.MajorSubsystemVersion = 4;
    //   image.FileHeader->OptionalHeader.MinorSubsystemVersion = 0;
    //   ::UnMapAndLoad(&image);
    // }


    // TODO Set correct flags
    HANDLE exe_file = ::CreateFileW(L"C:\\GOG Games\\Soldier of Fortune\\SoF.exe", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (!exe_file)
    {
      return;
    }

    auto hbinary = ::DetourBinaryOpen(exe_file);

    if (hbinary)
    {
      if (::DetourBinaryEditImports(hbinary, nullptr, nullptr, handler::file_callback, handler::symbol_callback, nullptr))
      {
        OutputDebugStringW(L"Updated imports of exe\n");
      }

      if (::DetourBinaryWrite(hbinary, exe_file))
      {
        OutputDebugStringW(L"Wrote data to file\n");
      }

      ::DetourBinaryClose(hbinary);
    }

    ::CloseHandle(exe_file);
  }

  void exe_view::populate_launch_table(const game_command_line_caps& caps)
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
        column.mask = column.mask | LVIF_PARAM;
        column.sub_items = {
          settings.last_player_name.data()
        };
        column.lParam = (LPARAM)game_command_line_caps::string_setting;

        launch_table.InsertRow(std::move(column));

        continue;
      }

      std::wstring_view ip_connect_setting = caps.ip_connect_setting ? caps.ip_connect_setting : L"";

      if (ip_connect_setting == value)
      {
        win32::list_view_item column(L"Server IP Address");
        column.mask = column.mask | LVIF_PARAM;
        column.sub_items = {
          settings.last_ip_address.data()
        };
        column.lParam = (LPARAM)game_command_line_caps::string_setting;

        ip_address_row_index = launch_table.InsertRow(std::move(column));
        continue;
      }

      win32::list_view_item column(value);
      column.mask = column.mask | LVIF_PARAM;
      column.sub_items = {
        L""
      };
      column.lParam = (LPARAM)game_command_line_caps::string_setting;

      launch_table.InsertRow(std::move(column));
    }

    for (auto& value : caps.int_settings)
    {
      if (!value)
      {
        break;
      }

      win32::list_view_item column(value);
      column.mask = column.mask | LVIF_PARAM;
      column.sub_items = {
        L""
      };
      column.lParam = (LPARAM)game_command_line_caps::int_setting;

      launch_table.InsertRow(std::move(column));
    }

    for (auto& value : caps.float_settings)
    {
      if (!value)
      {
        break;
      }

      win32::list_view_item column(value);
      column.mask = column.mask | LVIF_PARAM;
      column.sub_items = {
        L""
      };
      column.lParam = (LPARAM)game_command_line_caps::float_setting;

      launch_table.InsertRow(std::move(column));
    }

    for (auto& value : caps.flags)
    {
      if (!value)
      {
        break;
      }

      win32::list_view_item column(value);
      column.mask = column.mask | LVIF_PARAM;
      column.sub_items = {
        L""
      };
      column.lParam = (LPARAM)game_command_line_caps::flag_setting;

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

            auto control_type = (game_command_line_caps::type)item.lParam;

            auto& extension = controller.get_extension();

            ListView_GetItemText(launch_table, info.iItem, 0, text.data(), text.size());

            bool uses_combo = false;

            std::wstring temp;
            if (control_type == game_command_line_caps::string_setting && extension.get_predefined_string_command_line_settings_proc)
            {
              if (auto* values = extension.get_predefined_string_command_line_settings_proc(text.data()); values)
              {
                ::SendMessageW(launch_table_combo, CB_RESETCONTENT, 0, 0);

                auto* first = values;

                do
                {
                  if (!first->label)
                  {
                    break;
                  }
                  uses_combo = true;
                  temp = first->label;
                  ::COMBOBOXEXITEMW new_item{
                    .mask = CBEIF_LPARAM | CBEIF_TEXT,
                    .iItem = -1,
                    .pszText = temp.data(),
                    .cchTextMax = (int)temp.size(),
                    .lParam = (LPARAM)first->value
                  };
                  ::SendMessageW(launch_table_combo, CBEM_INSERTITEMW, 0, (LPARAM)&new_item);

                  first++;
                } while (first->label);
              }
            }
            else if (control_type == game_command_line_caps::int_setting && extension.get_predefined_int_command_line_settings_proc)
            {
              if (auto* values = extension.get_predefined_int_command_line_settings_proc(text.data()); values)
              {
                ::SendMessageW(launch_table_combo, CB_RESETCONTENT, 0, 0);

                auto* first = values;

                do
                {
                  if (!first->label)
                  {
                    break;
                  }
                  uses_combo = true;
                  temp = first->label;
                  ::COMBOBOXEXITEMW new_item{
                    .mask = CBEIF_LPARAM | CBEIF_TEXT,
                    .iItem = -1,
                    .pszText = temp.data(),
                    .cchTextMax = (int)temp.size(),
                    .lParam = (LPARAM)first->value,
                  };

                  ::SendMessageW(launch_table_combo, CBEM_INSERTITEMW, 0, (LPARAM)&new_item);

                  first++;
                } while (first->label);
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

              launch_table_edit_unbind = launch_table_combo.bind_cbn_sel_change([this, info, control_type](auto, const auto&) {
                std::fill_n(text.data(), text.size(), L'\0');
                ::COMBOBOXEXITEMW new_item{
                  .mask = CBEIF_LPARAM | CBEIF_TEXT,
                  .iItem = ::SendMessage(launch_table_combo, CB_GETCURSEL, 0, 0),
                  .pszText = text.data(),
                  .cchTextMax = (int)text.size(),
                };

                if (::SendMessageW(launch_table_combo, CBEM_GETITEMW, 0, (LPARAM)&new_item))
                {
                  if (control_type == game_command_line_caps::string_setting)
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
            else
            {
              launch_table_combo.SetWindowStyle(launch_table_combo.GetWindowStyle() & ~WS_VISIBLE);
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
              auto virtual_key = bound_actions[item->lParam].vkey;
              auto context = bound_actions[item->lParam].context;
              auto action_index = bound_actions[item->lParam].action_index;

              auto& action = actions[action_index];

              auto hardware_index = hardware_index_for_controller_vkey(std::span<RAWINPUTDEVICELIST>(controllers.data(), size), 0, virtual_key);
              game_args->action_bindings[binding_index].vkey = virtual_key;
              game_args->action_bindings[binding_index].action_name = action.action_name;
              game_args->action_bindings[binding_index].context = hardware_index.first;
              game_args->action_bindings[binding_index++].hardware_index = hardware_index.second;
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

        for (auto i = 0; i < launch_table.GetItemCount(); ++i)
        {
          std::wstring name(255, L'\0');
          std::wstring value(255, L'\0');

          ListView_GetItemText(launch_table, i, 0, name.data(), name.size());
          ListView_GetItemText(launch_table, i, 1, value.data(), value.size());

          info.iItem = i;

          ListView_GetItem(launch_table, &info);

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

          game_args->string_settings[i].name = launch_strings[i][0].c_str();
          game_args->string_settings[i].value = launch_strings[i][1].c_str();
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