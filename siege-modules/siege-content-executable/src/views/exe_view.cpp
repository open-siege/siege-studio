#include <spanstream>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/threading.hpp>
#include "input-filter.hpp"
#include "input_injector.hpp"
#include "views/exe_views.hpp"

namespace siege::views
{
  using namespace std::literals;


  std::optional<win32::lresult_t> exe_view::wm_create()
  {
    options = *win32::CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
      .hwndParent = *this,
      .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

    extract_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 1, L"Open in New Tab");
    extract_menu.AppendMenuW(MF_STRING | MF_OWNERDRAW, 2, L"Extract");

    options_unbind = options.bind_lbn_sel_change(std::bind_front(&exe_view::options_lbn_sel_change, this));
    options.InsertString(-1, L"Resources");
    options.SetCurrentSelection(0);

    exe_actions = *win32::CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{
      .hwndParent = *this,
      .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });

    exe_actions.InsertButton(-1, { .iBitmap = 0, .idCommand = add_to_firewall_selected_id, .fsStyle = BTNS_BUTTON, .iString = (INT_PTR)L"Add to Firewall" }, false);
    exe_actions.InsertButton(-1, { .iBitmap = 1, .idCommand = launch_selected_id, .fsStyle = BTNS_BUTTON, .iString = (INT_PTR)L"Launch" }, false);
    exe_actions.InsertButton(-1, { .iBitmap = 2, .idCommand = extract_selected_id, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Extract" }, false);
    exe_actions.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);
    exe_actions.bind_nm_click(std::bind_front(&exe_view::exe_actions_nm_click, this));

    resource_table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
      .style = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER });

    resource_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Name"),
                                    });

    resource_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Action"),
                                    });

    resource_table.EnableGroupView(true);

    resource_table.bind_nm_rclick(std::bind_front(&exe_view::resource_table_nm_rclick, this));
    resource_table.bind_lvn_item_changed(std::bind_front(&exe_view::resource_table_lvn_item_changed, this));
    resource_table.SetExtendedListViewStyle(LVS_EX_HEADERINALLVIEWS | LVS_EX_FULLROWSELECT, LVS_EX_HEADERINALLVIEWS | LVS_EX_FULLROWSELECT);

    string_table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
      .style = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER });
    string_table.InsertColumn(-1, LVCOLUMNW{
                                    .pszText = const_cast<wchar_t*>(L"Text"),
                                  });

    string_table.EnableGroupView(true);

    launch_table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
      .style = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

    launch_table.bind_nm_click(std::bind_front(&exe_view::launch_table_nm_click, this));
    launch_table.InsertColumn(-1, LVCOLUMNW{
                                    .pszText = const_cast<wchar_t*>(L"Name"),
                                  });

    launch_table.InsertColumn(-1, LVCOLUMNW{
                                    .pszText = const_cast<wchar_t*>(L"Value"),
                                  });

    launch_table_edit = *win32::CreateWindowExW<win32::edit>({ .hwndParent = *this, .style = WS_CHILD });
    launch_table_combo = *win32::CreateWindowExW<win32::combo_box_ex>({ .hwndParent = *this, .cy = 300, .cx = 300, .style = WS_CHILD | CBS_DROPDOWNLIST });
    //::SendMessageW(launch_table_combo, CBEM_SETUNICODEFORMAT, 1, 0);

    launch_table_ip_address = *win32::CreateWindowExW<win32::ip_address_edit>({ .hwndParent = *this, .cy = 100, .cx = 300, .style = WS_CHILD });

    keyboard_table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this, .style = WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS });

    keyboard_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Action"),
                                    });

    keyboard_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Key"),
                                    });

    keyboard_table.SetExtendedListViewStyle(0, LVS_EX_FULLROWSELECT);
    keyboard_table.EnableGroupView(true);
    keyboard_table.bind_nm_click(std::bind_front(&exe_view::keyboard_table_nm_click, this));


    controller_table = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this, .style = WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS });
    controller_table.bind_nm_click(std::bind_front(&exe_view::controller_table_nm_click, this));

    controller_table.InsertColumn(-1, LVCOLUMNW{
                                        .pszText = const_cast<wchar_t*>(L""),
                                      });

    controller_table.InsertColumn(-1, LVCOLUMNW{
                                        .fmt = LVCFMT_RIGHT,
                                        .pszText = const_cast<wchar_t*>(L""),

                                      });

    controller_table.InsertColumn(-1, LVCOLUMNW{
                                        .fmt = LVCFMT_RIGHT,
                                        .pszText = const_cast<wchar_t*>(L""),
                                      });

    controller_table.EnableGroupView(true);

    ListView_SetView(controller_table, LV_VIEW_TILE);

    LVTILEVIEWINFO tileViewInfo{
      .cbSize = sizeof(tileViewInfo),
      .dwMask = LVTVIM_COLUMNS,
      .dwFlags = LVTVIF_AUTOSIZE,
      .cLines = 2
    };

    ListView_SetTileViewInfo(controller_table, &tileViewInfo);

    for (auto i = 0; i < controller_table.GetItemCount(); ++i)
    {
      UINT columns[2] = { 1, 2 };
      int formats[2] = { LVCFMT_LEFT, LVCFMT_RIGHT };
      LVTILEINFO item_info{ .cbSize = sizeof(LVTILEINFO), .iItem = (int)i, .cColumns = 2, .puColumns = columns, .piColFmt = formats };

      ListView_SetTileInfo(controller_table, &item_info);
    }

    auto header = launch_table.GetHeader();

    auto style = header.GetWindowStyle();

    header.SetWindowStyle(style | HDS_NOSIZING | HDS_FLAT);

    wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

    return 0;
  }

  std::optional<win32::lresult_t> exe_view::wm_size(std::size_t type, SIZE client_size)
  {
    auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 12 };

    auto one_quarter = SIZE{ .cx = client_size.cx / 4, .cy = client_size.cy - top_size.cy };
    auto three_quarters = SIZE{ .cx = client_size.cx - one_quarter.cx, .cy = client_size.cy - top_size.cy };

    recreate_image_lists(exe_actions.GetIdealIconSize(SIZE{ .cx = client_size.cx / exe_actions.ButtonCount(), .cy = top_size.cy }));
    SendMessageW(exe_actions, TB_SETIMAGELIST, 0, (LPARAM)exe_actions_icons.get());

    exe_actions.SetWindowPos(POINT{}, SWP_DEFERERASE | SWP_NOREDRAW);
    exe_actions.SetWindowPos(top_size, SWP_DEFERERASE);
    exe_actions.SetButtonSize(SIZE{ .cx = top_size.cx / exe_actions.ButtonCount(), .cy = top_size.cy });

    resource_table.SetWindowPos(three_quarters);
    resource_table.SetWindowPos(POINT{ .y = top_size.cy });

    string_table.SetWindowPos(three_quarters);
    string_table.SetWindowPos(POINT{ .y = top_size.cy });

    launch_table.SetWindowPos(three_quarters);
    launch_table.SetWindowPos(POINT{ .y = top_size.cy });

    keyboard_table.SetWindowPos(three_quarters);
    keyboard_table.SetWindowPos(POINT{ .y = top_size.cy });

    controller_table.SetWindowPos(three_quarters);
    controller_table.SetWindowPos(POINT{ .y = top_size.cy });
    controller_table.SetImageList(LVSIL_NORMAL, controller_table_icons);

    options.SetWindowPos(one_quarter);
    options.SetWindowPos(POINT{ .x = three_quarters.cx, .y = top_size.cy });

    std::array<std::reference_wrapper<win32::list_view>, 5> tables = { { std::ref(resource_table),
      std::ref(string_table),
      std::ref(keyboard_table),
      std::ref(launch_table),
      std::ref(controller_table) } };

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

  void exe_view::extract_selected_files()
  {
    if (has_saved == false)
    {
      return;
    }

    auto path = get_destination_directory();

    if (!path)
    {
      return;
    }

    auto items = selected_resource_items;

    win32::queue_user_work_item([path = std::move(path), this, items]() {
      has_saved = false;

      std::for_each(items.begin(), items.end(), [this, path](auto& item) {
        auto extension = controller.get_extension_for_name(item.first, item.second);

        if (extension)
        {
          auto raw_data = controller.get_resource_data(item.first, item.second);
          auto filename = item.second + *extension;

          std::ofstream extracted_file(*path / filename, std::ios::trunc | std::ios::binary);

          extracted_file.write((char*)raw_data.data(), raw_data.size());
        }
      });

      win32::launch_shell_process(*path);
      has_saved = true;
    });
  }

  void exe_view::resource_table_lvn_item_changed(win32::list_view, const NMLISTVIEW& message)
  {
    std::wstring temp(255, '\0');

    if (message.lParam)
    {
      temp.resize(::GetAtomNameW((ATOM)message.lParam, temp.data(), temp.size()));

      if (temp.size() == 0)
      {
        return;
      }

      auto group_id = temp.substr(0, temp.find(':'));
      auto item_id = temp.substr(temp.find(':') + 1);

      if (message.uNewState & LVIS_SELECTED)
      {
        selected_resource_items.emplace(std::make_pair(group_id, item_id));
      }
      else if (message.uOldState & LVIS_SELECTED)
      {
        selected_resource_items.erase(std::make_pair(group_id, item_id));
      }

      TBBUTTONINFOW button_info{
        .cbSize = sizeof(TBBUTTONINFOW),
        .dwMask = TBIF_STATE,
        .fsState = selected_resource_items.empty() ? BYTE(0x00) : BYTE(TBSTATE_ENABLED),
      };

      ::SendMessageW(exe_actions, TB_SETBUTTONINFO, extract_selected_id, (LPARAM)&button_info);
    }
  }

  void exe_view::resource_table_nm_rclick(win32::list_view, const NMITEMACTIVATE& message)
  {
    auto point = message.ptAction;

    if (ClientToScreen(resource_table, &point))
    {
      auto result = extract_menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, point, ref());

      if (result == 1)
      {
        auto root = this->GetAncestor(GA_ROOT);

        if (root)
        {
          for (auto& item : selected_resource_items)
          {
            auto extension = controller.get_extension_for_name(item.first, item.second);

            if (extension)
            {
              auto raw_data = controller.get_resource_data(item.first, item.second);
              auto filename = item.second + *extension;
              root->CopyData(*this, COPYDATASTRUCT{ .dwData = ::AddAtomW(filename.data()), .cbData = DWORD(raw_data.size()), .lpData = raw_data.data() });
            }
          }
        }
      }
      else if (result == 2)
      {
        extract_selected_files();
      }
    }
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

    if (controller_table_icons)
    {
      controller_table_icons.reset();
    }


    std::array exe_icons{
      win32::segoe_fluent_icons::shield,
      win32::segoe_fluent_icons::new_window,
      win32::segoe_fluent_icons::folder_open,
    };

    exe_actions_icons = win32::create_icon_list(exe_icons, icon_size);

    std::array controller_icons{
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
    controller_table_icons = win32::create_icon_list(controller_icons, icon_size);
  }

  void exe_view::options_lbn_sel_change(win32::list_box, const NMHDR&)
  {
    auto selected = options.GetCurrentSelection();

    std::array tables{ launch_table.ref(), keyboard_table.ref(), controller_table.ref(), string_table.ref(), resource_table.ref() };

    for (auto i = 0; i < tables.size(); ++i)
    {
      ::ShowWindow(tables[i], i == selected ? SW_SHOW : SW_HIDE);
    }

    if (selected != 0)
    {
      ::ShowWindow(launch_table_edit, SW_HIDE);
      ::ShowWindow(launch_table_combo, SW_HIDE);
      ::ShowWindow(launch_table_ip_address, SW_HIDE);
    }

    if (selected != tables.size() - 1)
    {
      selected_resource_items.clear();

      TBBUTTONINFOW button_info{
        .cbSize = sizeof(TBBUTTONINFOW),
        .dwMask = TBIF_STATE,
        .fsState = 0,
      };

      ::SendMessageW(exe_actions, TB_SETBUTTONINFO, extract_selected_id, (LPARAM)&button_info);
    }
  }

  std::optional<win32::lresult_t> exe_view::wm_copy_data(win32::copy_data_message<char> message)
  {
    std::spanstream stream(message.data);

    std::optional<std::filesystem::path> path = win32::get_path_from_handle((HANDLE)message.data_type);

    if (path && options.GetCount() < 3 && (path->extension() == ".exe" || path->extension() == ".EXE"))
    {
      options.InsertString(0, L"Launch Options");
      options.InsertString(1, L"Keyboard/Mouse Settings");
      options.InsertString(2, L"Controller");
      options.InsertString(3, L"Scripting");

      ::ShowWindow(launch_table, SW_SHOW);
      ::ShowWindow(resource_table, SW_HIDE);

      siege::init_active_input_state();

      TBBUTTONINFOW button_info{
        .cbSize = sizeof(TBBUTTONINFOW),
        .dwMask = TBIF_STATE,
        .fsState = TBSTATE_ENABLED,
      };

      ::SendMessageW(exe_actions, TB_SETBUTTONINFO, launch_selected_id, (LPARAM)&button_info);
      ::SendMessageW(exe_actions, TB_SETBUTTONINFO, add_to_firewall_selected_id, (LPARAM)&button_info);
    }

    auto count = controller.load_executable(stream, std::move(path));

    if (count > 0)
    {
      auto values = controller.get_resource_names();

      if (controller.has_extension_module())
      {
        auto& extension = controller.get_extension();

        auto* caps = extension.caps;

        if (caps)
        {
          populate_launch_table(*caps);
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
            auto data = controller.get_resource_menu_items(value.first, child);

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
            auto data = controller.get_resource_strings(value.first, child);

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

        if (group_names.contains(value.first))
        {
          auto& group = groups.emplace_back(std::wstring(group_names[value.first]), std::move(items));
          group.state = LVGS_COLLAPSIBLE;
        }
        else
        {
          auto& group = groups.emplace_back(value.first, std::move(items));
          group.state = LVGS_COLLAPSIBLE;
        }
      }

      resource_table.InsertGroups(groups);

      groups.clear();
      groups.reserve(2);

      auto functions = controller.get_function_names();
      std::vector<win32::list_view_item> items;
      items.reserve(functions.size());

      for (auto& child : functions)
      {
        items.emplace_back(win32::list_view_item(win32::widen(child)));
      }

      auto& group1 = groups.emplace_back(L"Script/Console Functions", std::move(items));
      group1.state = LVGS_COLLAPSIBLE;

      auto variables = controller.get_variable_names();
      items.clear();
      items.reserve(variables.size());

      for (auto& child : variables)
      {
        items.emplace_back(win32::list_view_item(win32::widen(child)));
      }

      auto& group2 = groups.emplace_back(L"Script/Console Variables", std::move(items));
      group2.state = LVGS_COLLAPSIBLE;

      string_table.InsertGroups(groups);

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

  std::wstring category_for_vkey(SHORT vkey, siege::platform::hardware_context context)
  {
    using namespace siege::platform;
    if (vkey >= VK_LBUTTON && vkey <= VK_XBUTTON2)
    {
      return L"Mouse";
    }

    if (context == hardware_context::mouse || context == hardware_context::mouse_wheel)
    {
      return L"Mouse";
    }

    if (vkey >= VK_GAMEPAD_A && vkey <= VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
    {
      return L"Controller";
    }

    if (context == hardware_context::controller || context == hardware_context::joystick || context == hardware_context::throttle)
    {
      return L"Controller";
    }

    return L"Keyboard";
  }

  std::uint16_t hardware_index_for_controller_vkey(std::span<RAWINPUTDEVICELIST> controllers, std::uint32_t index, SHORT vkey)
  {
    std::wstring device_name(1024, 0);

    std::uint32_t current_index = 0;
    std::set<std::uint32_t> xinput_devices{};
    bool found_controller = false;
    for (auto i = 0; i < controllers.size(); ++i)
    {
      if (controllers[i].dwType != RIM_TYPEHID)
      {
        continue;
      }
      RID_DEVICE_INFO info{};
      UINT info_size = sizeof(info);

      if (::GetRawInputDeviceInfoW(controllers[i].hDevice, RIDI_DEVICEINFO, &info, &info_size) != info_size)
      {
        continue;
      }
      if (!(info.hid.usUsage == HID_USAGE_GENERIC_GAMEPAD || info.hid.usUsage == HID_USAGE_GENERIC_JOYSTICK))
      {
        continue;
      }

      info_size = device_name.size();
      if (auto real_size = ::GetRawInputDeviceInfoW(controllers[i].hDevice, RIDI_DEVICENAME, device_name.data(), &info_size); real_size <= 0)
      {
        continue;
      }
      else
      {
        device_name.resize(real_size);
      }

      if (device_name.rfind(L"IG_") != std::wstring_view::npos)
      {
        xinput_devices.insert(current_index);
      }

      if (current_index == index)
      {
        found_controller = true;
        break;
      }

      current_index++;
    }

    if (found_controller && xinput_devices.contains(current_index))
    {
      switch (vkey)
      {
      case VK_GAMEPAD_A:
        return 0;
      case VK_GAMEPAD_B:
        return 1;
      case VK_GAMEPAD_X:
        return 2;
      case VK_GAMEPAD_Y:
        return 3;
      case VK_GAMEPAD_LEFT_SHOULDER:
        return 4;
      case VK_GAMEPAD_RIGHT_SHOULDER:
        return 5;
      case VK_GAMEPAD_VIEW:
        return 6;
      case VK_GAMEPAD_MENU:
        return 7;
      case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
        return 8;
      case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
        return 9;
      case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
      case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
        return 0;
      case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
      case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
        return 1;
      case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
      case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
        return 4;
      case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
      case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
        return 3;
      case VK_GAMEPAD_LEFT_TRIGGER:
      case VK_GAMEPAD_RIGHT_TRIGGER:
        return 2;
      default:
        return 0;
      }
    }
    else if (found_controller)
    {
      switch (vkey)
      {
      case VK_GAMEPAD_X:// ps3/ps4 square
        return 0;
      case VK_GAMEPAD_A:// ps3/ps4 cross
        return 1;
      case VK_GAMEPAD_B:// ps3/ps4 circle
        return 2;
      case VK_GAMEPAD_Y:// ps3/ps4 triangle
        return 3;
      case VK_GAMEPAD_LEFT_SHOULDER:
        return 4;
      case VK_GAMEPAD_RIGHT_SHOULDER:
        return 5;
      case VK_GAMEPAD_LEFT_TRIGGER:
        return 6;
      case VK_GAMEPAD_RIGHT_TRIGGER:
        return 7;
      case VK_GAMEPAD_VIEW:
        return 8;
      case VK_GAMEPAD_MENU:
        return 9;
      case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
        return 10;
      case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
        return 11;
      case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
      case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
        return 0;
      case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
      case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
        return 1;
      case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
      case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
        return 2;
      case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
      case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
        return 3;
      default:
        return 0;
      }
    }
    return 0;
  }

  std::wstring string_for_vkey(SHORT vkey, siege::platform::hardware_context context)
  {
    constexpr static auto directions = std::array<std::wstring_view, 4>{
      {
        std::wstring_view(L"Left"),
        std::wstring_view(L"Up"),
        std::wstring_view(L"Right"),
        std::wstring_view(L"Down"),
      }
    };

    if (context == siege::platform::hardware_context::mouse)
    {
      if (vkey >= VK_LEFT && vkey <= VK_DOWN)
      {
        std::wstring result(L"Move ");
        result.append(directions[vkey - VK_LEFT]);
        return result;
      }
    }

    if (context == siege::platform::hardware_context::mouse_wheel)
    {
      if (vkey >= VK_LEFT && vkey <= VK_DOWN)
      {
        std::wstring result(L"Scroll ");
        result.append(directions[vkey - VK_LEFT]);
        return result;
      }
    }

    if (vkey >= VK_NUMPAD0 && vkey <= VK_NUMPAD9)
    {
      std::wstring result(L"Numpad ");
      result.push_back(L'0' + vkey - VK_NUMPAD0);
      return result;
    }

    if (vkey >= VK_F1 && vkey <= VK_F9)
    {
      std::wstring result(1, 'F');
      result.push_back(L'1' + vkey - VK_F1);
      return result;
    }

    if (vkey >= VK_F10 && vkey <= VK_F19)
    {
      std::wstring result(1, 'F');
      result.push_back(L'1');
      result.push_back(L'0' + vkey - VK_F10);
      return result;
    }

    if (vkey >= VK_F20 && vkey <= VK_F24)
    {
      std::wstring result(1, 'F');
      result.push_back(L'2');
      result.push_back(L'0' + vkey - VK_F20);
      return result;
    }

    switch (vkey)
    {
    case VK_LBUTTON:
      return L"Left Button";
    case VK_RBUTTON:
      return L"Right Button";
    case VK_MBUTTON:
      return L"Middle Button";
    case VK_XBUTTON1:
      return L"Extra Button 1";
    case VK_XBUTTON2:
      return L"Extra Button 2";
    case VK_TAB:
      return L"Tab";
    case VK_LCONTROL:
      return L"Left Control";
    case VK_RCONTROL:
      return L"Right Control";
    case VK_LMENU:
      return L"Left Alt";
    case VK_RMENU:
      return L"Right Alt";
    case VK_LSHIFT:
      return L"Left Shift";
    case VK_RSHIFT:
      return L"Right Shift";
    case VK_LWIN:
      return L"Left Windows Key";
    case VK_RWIN:
      return L"Right Windows Key";
    case VK_RETURN:
      return L"Enter";
    case VK_UP:
      return L"Up Arrow";
    case VK_DOWN:
      return L"Down Arrow";
    case VK_LEFT:
      return L"Left Arrow";
    case VK_RIGHT:
      return L"Right Arrow";
    case VK_SPACE:
      return L"Spacebar";
    case VK_SNAPSHOT:
      return L"Print Screen";
    case VK_CAPITAL:
      return L"Caps Lock";
    case VK_NUMLOCK:
      return L"Num Lock";
    case VK_SCROLL:
      return L"Scroll Lock";
    case VK_HOME:
      return L"Home";
    case VK_DELETE:
      return L"Insert";
    case VK_ESCAPE:
      return L"Escape";
    case VK_INSERT:
      return L"Insert";
    case VK_PRIOR:
      return L"Page Down";
    case VK_NEXT:
      return L"Page Up";
    case VK_PAUSE:
      return L"Pause";
    case VK_BACK:
      return L"Backspace";
    case VK_GAMEPAD_A:
      return L"A Button";
    case VK_GAMEPAD_B:
      return L"B Button";
    case VK_GAMEPAD_X:
      return L"X Button";
    case VK_GAMEPAD_Y:
      return L"Y Button";
    case VK_GAMEPAD_LEFT_SHOULDER:
      return L"Left Bumper";
    case VK_GAMEPAD_RIGHT_SHOULDER:
      return L"Right Bumper";
    case VK_GAMEPAD_LEFT_TRIGGER:
      return L"Left Trigger";
    case VK_GAMEPAD_RIGHT_TRIGGER:
      return L"Right Trigger";
    case VK_GAMEPAD_DPAD_UP:
      return L"D-pad Up";
    case VK_GAMEPAD_DPAD_DOWN:
      return L"D-pad Down";
    case VK_GAMEPAD_DPAD_LEFT:
      return L"D-pad Left";
    case VK_GAMEPAD_DPAD_RIGHT:
      return L"D-pad Right";
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
      return L"Left Stick Button";
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
      return L"Left Stick Up";
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
      return L"Left Stick Down";
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
      return L"Left Stick Left";
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
      return L"Left Stick Right";
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
      return L"Right Stick Button";
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
      return L"Right Stick Up";
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
      return L"Right Stick Down";
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
      return L"Right Stick Left";
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
      return L"Right Stick Right";
    default:
      return std::wstring(1, ::MapVirtualKeyW(vkey, MAPVK_VK_TO_CHAR));
    }
  }

}// namespace siege::views