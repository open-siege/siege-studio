#include <spanstream>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/threading.hpp>
#include <siege/extension/input_filter.hpp>
#include <siege/extension/input_filter.hpp>
#include "views/exe_view.hpp"

namespace siege::views
{
  using namespace std::literals;

  constexpr static int add_to_firewall_selected_id = 12;
  constexpr static int extract_selected_id = 13;

  void extract_selected_files(std::any& state, exe_view::resource_controls& resource);

  win32::lresult_t exe_view::wm_create()
  {
    options = *win32::CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
      .hwndParent = *this,
      .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

    options.InsertString(-1, L"Resources");
    options.SetCurrentSelection(0);

    exe_actions = *win32::CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{
      .hwndParent = *this,
      .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });

    exe_actions.InsertButton(-1, { .iBitmap = 0, .idCommand = add_to_firewall_selected_id, .fsStyle = BTNS_BUTTON, .iString = (INT_PTR)L"Add to Firewall" }, false);
    exe_actions.InsertButton(-1, { .iBitmap = 0, .idCommand = input.controllers_selected_id, .fsStyle = BTNS_BUTTON, .iString = (INT_PTR)L"Input Devices" }, false);
    exe_actions.InsertButton(-1, { .iBitmap = 1, .idCommand = launch.launch_selected_id, .fsStyle = BTNS_BUTTON, .iString = (INT_PTR)L"Launch" }, false);
    exe_actions.InsertButton(-1, { .iBitmap = 2, .idCommand = extract_selected_id, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Extract" }, false);
    exe_actions.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);
    exe_actions.bind_nm_click([this](win32::tool_bar exe_actions, const NMMOUSE& message) {
      if (message.dwItemSpec != extract_selected_id)
      {
        return false;
      }

      extract_selected_files(state, resource);
      return true;
    });


    exe_actions.bind_nm_click([this](win32::tool_bar, const NMMOUSE& message) {
      if (message.dwItemSpec != add_to_firewall_selected_id)
      {
        return false;
      }

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

      return true;
    });

    resource.extract_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 1, L"Open in New Tab");
    resource.extract_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 2, L"Extract");


    resource.resource_table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
      .style = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER });

    resource.resource_table.InsertColumn(-1, LVCOLUMNW{
                                               .pszText = const_cast<wchar_t*>(L"Name"),
                                             });

    resource.resource_table.InsertColumn(-1, LVCOLUMNW{
                                               .pszText = const_cast<wchar_t*>(L"Action"),
                                             });

    resource.resource_table.EnableGroupView(true);

    resource.resource_table.SetExtendedListViewStyle(LVS_EX_HEADERINALLVIEWS | LVS_EX_FULLROWSELECT, LVS_EX_HEADERINALLVIEWS | LVS_EX_FULLROWSELECT);

    resource.string_table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
      .style = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER });
    resource.string_table.InsertColumn(-1, LVCOLUMNW{
                                             .pszText = const_cast<wchar_t*>(L"Text"),
                                           });

    resource.string_table.EnableGroupView(true);

    launch = create_launch_controls();
    input = create_input_controls();

    auto header = launch.launch_table.GetHeader();

    auto style = header.GetWindowStyle();

    header.SetWindowStyle(style | HDS_NOSIZING | HDS_FLAT);

    wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

    resource.resource_table.bind_lvn_item_changed([this](win32::list_view, const NMLISTVIEW& message) {
      if (!message.lParam)
      {
        return;
      }

      std::wstring temp(255, '\0');

      temp.resize(::GetAtomNameW((ATOM)message.lParam, temp.data(), temp.size()));

      if (temp.size() == 0)
      {
        return;
      }

      auto group_id = temp.substr(0, temp.find(':'));
      auto item_id = temp.substr(temp.find(':') + 1);

      if (message.uNewState & LVIS_SELECTED)
      {
        resource.selected_resource_items.emplace(std::make_pair(group_id, item_id));
      }
      else if (message.uOldState & LVIS_SELECTED)
      {
        resource.selected_resource_items.erase(std::make_pair(group_id, item_id));
      }

      TBBUTTONINFOW button_info{
        .cbSize = sizeof(TBBUTTONINFOW),
        .dwMask = TBIF_STATE,
        .fsState = resource.selected_resource_items.empty() ? BYTE(0x00) : BYTE(TBSTATE_ENABLED),
      };

      ::SendMessageW(exe_actions, TB_SETBUTTONINFO, extract_selected_id, (LPARAM)&button_info);
    });

    resource.resource_table.bind_nm_rclick([this](win32::list_view resource_table, const NMITEMACTIVATE& message) {
      auto point = message.ptAction;

      if (!ClientToScreen(resource_table, &point))
      {
        return;
      }

      auto result = resource.extract_menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, point, ref());

      if (result == 1)
      {
        auto root = this->GetAncestor(GA_ROOT);

        if (root)
        {
          for (auto& item : resource.selected_resource_items)
          {
            auto extension = get_extension_for_name(state, item.first, item.second);

            if (extension)
            {
              auto raw_data = get_resource_data(state, item.first, item.second);
              auto filename = item.second + *extension;
              root->CopyData(*this, COPYDATASTRUCT{ .dwData = ::AddAtomW(filename.data()), .cbData = DWORD(raw_data.size()), .lpData = raw_data.data() });
            }
          }
        }
      }
      else if (result == 2)
      {
        extract_selected_files(state, resource);
      }
    });
    return 0;
  }

  win32::lresult_t exe_view::wm_size(std::size_t type, SIZE client_size)
  {
    auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 12 };

    auto one_quarter = SIZE{ .cx = client_size.cx / 4, .cy = client_size.cy - top_size.cy };
    auto three_quarters = SIZE{ .cx = client_size.cx - one_quarter.cx, .cy = client_size.cy - top_size.cy };

    recreate_image_lists(exe_actions.GetIdealIconSize(SIZE{ .cx = client_size.cx / exe_actions.ButtonCount(), .cy = top_size.cy }));
    SendMessageW(exe_actions, TB_SETIMAGELIST, 0, (LPARAM)exe_actions_icons.get());

    exe_actions.SetWindowPos(POINT{}, SWP_DEFERERASE | SWP_NOREDRAW);
    exe_actions.SetWindowPos(top_size, SWP_DEFERERASE);
    exe_actions.SetButtonSize(SIZE{ .cx = top_size.cx / exe_actions.ButtonCount(), .cy = top_size.cy });

    resource.resource_table.SetWindowPos(three_quarters);
    resource.resource_table.SetWindowPos(POINT{ .y = top_size.cy });

    resource.string_table.SetWindowPos(three_quarters);
    resource.string_table.SetWindowPos(POINT{ .y = top_size.cy });

    launch.launch_table.SetWindowPos(three_quarters);
    launch.launch_table.SetWindowPos(POINT{ .y = top_size.cy });

    input.keyboard_table.SetWindowPos(three_quarters);
    input.keyboard_table.SetWindowPos(POINT{ .y = top_size.cy });

    input.controller_table.SetWindowPos(three_quarters);
    input.controller_table.SetWindowPos(POINT{ .y = top_size.cy });
    input.controller_table.SetImageList(LVSIL_NORMAL, input.controller_table_icons);

    options.SetWindowPos(one_quarter);
    options.SetWindowPos(POINT{ .x = three_quarters.cx, .y = top_size.cy });

    std::array<std::reference_wrapper<win32::list_view>, 5> tables = { { std::ref(resource.resource_table),
      std::ref(resource.string_table),
      std::ref(input.keyboard_table),
      std::ref(launch.launch_table),
      std::ref(input.controller_table) } };

    for (auto& table : tables)
    {
      auto column_count = table.get().GetColumnCount();

      if (!column_count)
      {
        continue;
      }

      auto column_width = three_quarters.cx / column_count;

      for (auto i = 0u; i < column_count; ++i)
      {
        table.get().SetColumnWidth(i, column_width);
      }
    }

    return 0;
  }

  std::optional<std::filesystem::path> get_destination_directory()
  {
    auto dialog = win32::com::CreateFileOpenDialog();

    if (dialog)
    {
      auto open_dialog = *dialog;
      open_dialog->SetOptions(FOS_PICKFOLDERS);
      auto result = open_dialog->Show(nullptr);

      if (result == S_OK)
      {
        auto selection = open_dialog.GetResult();

        if (selection)
        {
          auto path = selection.value().GetFileSysPath();

          if (path)
          {
            return *path;
          }
        }
      }
    }

    return std::nullopt;
  }

  void extract_selected_files(std::any& state, exe_view::resource_controls& resource)
  {
    if (resource.has_saved == false)
    {
      return;
    }

    auto path = get_destination_directory();

    if (!path)
    {
      return;
    }

    auto items = resource.selected_resource_items;

    win32::queue_user_work_item([path = std::move(path), &state, &resource, items]() {
      resource.has_saved = false;

      std::for_each(items.begin(), items.end(), [&state, path](auto& item) {
        auto extension = get_extension_for_name(state, item.first, item.second);

        if (extension)
        {
          auto raw_data = get_resource_data(state, item.first, item.second);
          auto filename = item.second + *extension;

          std::ofstream extracted_file(*path / filename, std::ios::trunc | std::ios::binary);

          extracted_file.write((char*)raw_data.data(), raw_data.size());
        }
      });

      win32::launch_shell_process(*path);
      resource.has_saved = true;
    });
  }

  void exe_view::recreate_image_lists(std::optional<SIZE> possible_size)
  {

    SIZE icon_size = possible_size.or_else([this] {
                                    return exe_actions_icons.GetIconSize();
                                  })
                       .or_else([] {
                         return std::make_optional(SIZE{
                           .cx = ::GetSystemMetrics(SM_CXSIZE),
                           .cy = ::GetSystemMetrics(SM_CYSIZE) });
                       })
                       .value();

    if (exe_actions_icons)
    {
      exe_actions_icons.reset();
    }

    if (input.controller_table_icons)
    {
      input.controller_table_icons.reset();
    }


    std::array exe_icons{
      win32::segoe_fluent_icons::shield,
      win32::segoe_fluent_icons::new_window,
      win32::segoe_fluent_icons::folder_open,
    };

    exe_actions_icons = win32::create_icon_list(exe_icons, icon_size);

    std::array controller_icons{
      win32::segoe_fluent_icons::unknown,
      win32::segoe_fluent_icons::button_a,
      win32::segoe_fluent_icons::button_b,
      win32::segoe_fluent_icons::button_x,
      win32::segoe_fluent_icons::button_y,
      win32::segoe_fluent_icons::bumper_left,
      win32::segoe_fluent_icons::bumper_right,
      win32::segoe_fluent_icons::trigger_left,
      win32::segoe_fluent_icons::trigger_right,
      win32::segoe_fluent_icons::dpad,
      win32::segoe_fluent_icons::arrow_up,
      win32::segoe_fluent_icons::arrow_down,
      win32::segoe_fluent_icons::arrow_left,
      win32::segoe_fluent_icons::arrow_right,
      win32::segoe_fluent_icons::left_stick,
      win32::segoe_fluent_icons::right_stick,
      win32::segoe_fluent_icons::caret_solid_up,
      win32::segoe_fluent_icons::caret_solid_down,
      win32::segoe_fluent_icons::caret_solid_left,
      win32::segoe_fluent_icons::caret_solid_right,
      win32::segoe_fluent_icons::button_view,
      win32::segoe_fluent_icons::button_menu
    };
    input.controller_table_icons = win32::create_icon_list(controller_icons, icon_size);
  }

  win32::lresult_t exe_view::wm_copy_data(win32::copy_data_message<char> message)
  {
    std::spanstream stream(message.data);

    std::optional<std::filesystem::path> path = win32::get_path_from_handle((HANDLE)message.data_type);

    if (path && options.GetCount() < 3 && (path->extension() == ".exe" || path->extension() == ".EXE"))
    {
      options.InsertString(0, L"Launch Options");
      options.InsertString(1, L"Keyboard/Mouse Settings");
      options.InsertString(2, L"Controller");
      options.InsertString(3, L"Scripting");
      options.SetCurrentSelection(0);
      options.bind_lbn_sel_change([this](win32::list_box options, const NMHDR&) {
        auto selected = options.GetCurrentSelection();

        std::array tables{ launch.launch_table.ref(), input.keyboard_table.ref(), input.controller_table.ref(), resource.string_table.ref(), resource.resource_table.ref() };

        for (auto i = 0; i < tables.size(); ++i)
        {
          ::ShowWindow(tables[i], i == selected ? SW_SHOW : SW_HIDE);
        }
      });
      options.bind_lbn_sel_change([this](auto options, const auto&) {
        auto selected = options.GetCurrentSelection();
        if (selected != 0)
        {
          ::ShowWindow(launch.launch_table_edit, SW_HIDE);
          ::ShowWindow(launch.launch_table_combo, SW_HIDE);
          ::ShowWindow(launch.launch_table_ip_address, SW_HIDE);
        }
      });

      options.bind_lbn_sel_change([this](auto options, const auto&) {
        auto selected = options.GetCurrentSelection();

        if (selected != 4)
        {
          resource.selected_resource_items.clear();

          TBBUTTONINFOW button_info{
            .cbSize = sizeof(TBBUTTONINFOW),
            .dwMask = TBIF_STATE,
            .fsState = 0,
          };

          ::SendMessageW(exe_actions, TB_SETBUTTONINFO, extract_selected_id, (LPARAM)&button_info);
        }
      });

      ::ShowWindow(launch.launch_table, SW_SHOW);
      ::ShowWindow(resource.resource_table, SW_HIDE);

      siege::init_active_input_state();

      TBBUTTONINFOW button_info{
        .cbSize = sizeof(TBBUTTONINFOW),
        .dwMask = TBIF_STATE,
        .fsState = TBSTATE_ENABLED,
      };

      ::SendMessageW(exe_actions, TB_SETBUTTONINFO, launch.launch_selected_id, (LPARAM)&button_info);
      ::SendMessageW(exe_actions, TB_SETBUTTONINFO, input.controllers_selected_id, (LPARAM)&button_info);
      ::SendMessageW(exe_actions, TB_SETBUTTONINFO, add_to_firewall_selected_id, (LPARAM)&button_info);
    }

    auto count = load_executable(state, stream, std::move(path));

    if (count > 0)
    {
      auto values = get_resource_names(state);

      auto populate_launch_table = [this]() {
        int setting_index = 1;

        auto launch_settings = init_launch_settings(state);

        for (auto& setting : launch_settings)
        {
          std::shared_ptr<void> deferred(nullptr, [&](...) { setting_index++; });

          if (!setting.visible)
          {
            continue;
          }
          win32::list_view_item column(setting.display_name);
          column.mask = column.mask | LVIF_PARAM | LVIF_GROUPID;
          column.sub_items = { setting.get_computed_display_value() };
          column.lParam = (LPARAM)setting_index;
          column.iGroupId = setting.group_id;

          setting.persist = [setting_index, launch_table = launch.launch_table.get(), &setting, persist = std::move(setting.persist)]() {
            if (persist)
            {
              persist();
            }

            auto find_info = LVFINDINFOW{
              .flags = LVFI_PARAM,
              .lParam = (LPARAM)setting_index
            };

            auto item = ListView_FindItem(launch_table, -1, &find_info);

            if (item == -1)
            {
              return;
            }

            auto value = setting.get_computed_display_value();
            ListView_SetItemText(launch_table, item, 1, value.data());
          };
          launch.launch_table.InsertRow(std::move(column));
        }
      };

      if (has_extension_module(state))
      {
        auto& extension = get_extension(state);

        if (extension.caps)
        {
          populate_launch_table();
        }

        if (!extension.game_actions.empty())
        {
          populate_controller_table(extension.game_actions, extension.controller_input_backends);
          populate_keyboard_table(extension.game_actions, extension.controller_input_backends);
        }
        else
        {
          std::array<siege::platform::game_action, 0> actions{};
          std::array<const wchar_t*, 0> backends{};
          populate_controller_table(actions, backends);
          populate_keyboard_table(extension.game_actions, extension.controller_input_backends);
        }
      }
      else// generically support controller to keyboard/mouse mapping and zero tier networking
      {
        siege::platform::game_command_line_caps empty_caps{};
        populate_launch_table();
        std::array<siege::platform::game_action, 0> actions{};
        std::array<const wchar_t*, 0> backends{};
        populate_controller_table(actions, backends);
      }

      std::vector<win32::list_view_group> groups;

      groups.reserve(values.size());

      for (auto& value : values)
      {
        std::vector<win32::list_view_item> items;
        items.reserve(value.second.size());

        for (auto& child : value.second)
        {
          win32::list_view_item item(child);
          auto temp = value.first + L":" + child;
          item.mask = item.mask | LVIF_PARAM;
          item.lParam = ::AddAtomW(temp.c_str());

          if (value.first == L"#4")
          {
            auto data = get_resource_menu_items(state, value.first, child);

            std::wstring final_result;

            if (data && !data->menu_items.empty())
            {
              final_result.reserve(data->menu_items.size() * data->menu_items.size());

              for (auto& str : data->menu_items)
              {
                final_result.append(std::move(str.text));
                final_result.append(L" \n");
              }
            }

            item.sub_items.emplace_back(std::move(final_result));
          }
          else if (value.first == L"#6")
          {
            auto data = get_resource_strings(state, value.first, child);

            std::wstring final_result;

            if (!data.empty())
            {
              final_result.reserve(data.size() * data[0].size());

              for (auto& str : data)
              {
                final_result.append(std::move(str));
                final_result.append(L" \n");
              }
            }

            item.sub_items.emplace_back(std::move(final_result));
          }

          items.emplace_back(std::move(item));
        }

        const static std::map<std::wstring_view, std::wstring_view> group_names = {
          { L"#1"sv, L"Cursor"sv },
          { L"#2"sv, L"Bitmap"sv },
          { L"#3"sv, L"Icon"sv },
          { L"#4"sv, L"Menu"sv },
          { L"#5"sv, L"Dialog"sv },
          { L"#6"sv, L"String Table"sv },
          { L"#8"sv, L"Font"sv },
          { L"#9"sv, L"Accelerator"sv },
          { L"#10"sv, L"Raw Data"sv },
          { L"#12"sv, L"Cursor Group"sv },
          { L"#14"sv, L"Icon Group"sv },
          { L"#16"sv, L"Version"sv },
          { L"#22"sv, L"Animated Icon"sv },
          { L"#23"sv, L"HTML"sv },
          { L"#24"sv, L"Side-by-Side Assembly Manifest"sv },
        };

        if (group_names.contains(value.first))
        {
          auto& group = groups.emplace_back(std::wstring(group_names.at(value.first)), std::move(items));
          group.state = LVGS_COLLAPSIBLE;
        }
        else
        {
          auto& group = groups.emplace_back(value.first, std::move(items));
          group.state = LVGS_COLLAPSIBLE;
        }
      }

      resource.resource_table.InsertGroups(groups);

      groups.clear();
      groups.reserve(2);

      auto functions = get_function_names(state);
      std::vector<win32::list_view_item> items;
      items.reserve(functions.size());

      for (auto& child : functions)
      {
        items.emplace_back(win32::list_view_item(win32::widen(child)));
      }

      auto& group1 = groups.emplace_back(L"Script/Console Functions", std::move(items));
      group1.state = LVGS_COLLAPSIBLE;

      auto variables = get_variable_names(state);
      items.clear();
      items.reserve(variables.size());

      for (auto& child : variables)
      {
        items.emplace_back(win32::list_view_item(win32::widen(child)));
      }

      auto& group2 = groups.emplace_back(L"Script/Console Variables", std::move(items));
      group2.state = LVGS_COLLAPSIBLE;

      resource.string_table.InsertGroups(groups);

      return TRUE;
    }

    return FALSE;
  }

  std::optional<win32::lresult_t> exe_view::wm_setting_change(win32::setting_change_message message)
  {
    if (message.setting == L"ImmersiveColorSet")
    {
      recreate_image_lists(std::nullopt);
      SendMessageW(exe_actions, TB_SETIMAGELIST, 0, (LPARAM)exe_actions_icons.get());

      return 0;
    }

    return std::nullopt;
  }

  std::optional<LRESULT> exe_view::window_proc(UINT message, WPARAM wparam, LPARAM lparam)
  {
    switch (message)
    {
    case WM_CREATE:
      return wm_create();
    case WM_SETTINGCHANGE:
      return wm_setting_change(win32::setting_change_message(wparam, lparam));
    case WM_SIZE:
      return wm_size((std::size_t)wparam, SIZE(LOWORD(lparam), HIWORD(lparam)));
    case WM_COPYDATA:
      return (LRESULT)wm_copy_data(win32::copy_data_message<char>(wparam, lparam));
    default:
      return std::nullopt;
    }
  }

  ATOM register_exe_view(win32::window_module_ref module)
  {
    WNDCLASSEXW info{
      .cbSize = sizeof(info),
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = win32::basic_window<exe_view>::window_proc,
      .cbWndExtra = sizeof(void*),
      .hInstance = module,
      .lpszClassName = win32::type_name<exe_view>().c_str(),
    };
    return ::RegisterClassExW(&info);
  }

}// namespace siege::views