#include <spanstream>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/window_factory.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include "input-filter.hpp"
#include "input_injector.hpp"
#include "views/exe_views.hpp"

namespace siege::views
{
  using namespace std::literals;


  std::optional<win32::lresult_t> exe_view::wm_create()
  {
    auto control_factory = win32::window_factory(ref());

    options = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
      .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS | LBS_OWNERDRAWFIXED });

    options_unbind = options.bind_lbn_sel_change(std::bind_front(&exe_view::options_lbn_sel_change, this));
    options.InsertString(-1, L"Resources");
    options.SetCurrentSelection(0);

    exe_actions = *control_factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });

    exe_actions.InsertButton(-1, { .iBitmap = 0, .idCommand = add_to_firewall_selected_id, .fsState = TBSTATE_ENABLED, .iString = (INT_PTR)L"Add to Firewall" }, false);
    exe_actions.InsertButton(-1, { .iBitmap = 1, .idCommand = launch_selected_id, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Launch" }, false);
    exe_actions.InsertButton(-1, { .iBitmap = 2, .idCommand = extract_selected_id, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Extract" }, false);
    exe_actions.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);
    exe_actions.bind_tbn_dropdown(std::bind_front(&exe_view::exe_actions_tbn_dropdown, this));
    exe_actions.bind_nm_click(std::bind_front(&exe_view::exe_actions_nm_click, this));

    resource_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER });

    resource_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Name"),
                                    });

    resource_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Action"),
                                    });

    resource_table.EnableGroupView(true);

    string_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER });
    string_table.InsertColumn(-1, LVCOLUMNW{
                                    .pszText = const_cast<wchar_t*>(L"Text"),
                                  });

    string_table.EnableGroupView(true);

    launch_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

    launch_table.bind_nm_click(std::bind_front(&exe_view::launch_table_nm_click, this));
    launch_table.InsertColumn(-1, LVCOLUMNW{
                                    .pszText = const_cast<wchar_t*>(L"Name"),
                                  });

    launch_table.InsertColumn(-1, LVCOLUMNW{
                                    .pszText = const_cast<wchar_t*>(L"Value"),
                                  });

    launch_table_edit = *control_factory.CreateWindowExW<win32::edit>({ .style = WS_CHILD });
    launch_table_combo = *control_factory.CreateWindowExW<win32::combo_box_ex>({ .cy = 300, .cx = 300, .style = WS_CHILD | CBS_DROPDOWNLIST });
    //::SendMessageW(launch_table_combo, CBEM_SETUNICODEFORMAT, 1, 0);

    launch_table_ip_address = *control_factory.CreateWindowExW<win32::ip_address_edit>({ .cy = 100, .cx = 300, .style = WS_CHILD });

    keyboard_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS });

    keyboard_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L""),
                                    });
 
    controller_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS });
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

    std::array<std::reference_wrapper<win32::list_view>, 4> tables = { { std::ref(resource_table),
      std::ref(string_table),
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
    };
    controller_table_icons = win32::create_icon_list(controller_icons, icon_size);
  }

  void exe_view::options_lbn_sel_change(win32::list_box, const NMHDR&)
  {
    OutputDebugStringW(L"options_lbn_sel_change\n");
    auto selected = options.GetCurrentSelection();

    std::array tables{ launch_table.ref(), keyboard_table.ref(), controller_table.ref(), string_table.ref(), resource_table.ref() };

    for (auto i = 0; i < tables.size(); ++i)
    {
      ::ShowWindow(tables[i], i == selected ? SW_SHOW : SW_HIDE);
    }
  }

  std::optional<win32::lresult_t> exe_view::wm_copy_data(win32::copy_data_message<char> message)
  {
    std::spanstream stream(message.data);

    std::optional<std::filesystem::path> path;

    if (wchar_t* filename = this->GetPropW<wchar_t*>(L"FilePath"); filename)
    {
      path = filename;

      if (options.GetCount() < 3 && (path->extension() == ".exe" || path->extension() == ".EXE"))
      {
        options.InsertString(0, L"Launch Options");
        options.InsertString(1, L"Keyboard/Mouse Settings");
        options.InsertString(2, L"Controller");
        options.InsertString(3, L"Scripting");

        ::ShowWindow(launch_table, SW_SHOW);
        ::ShowWindow(resource_table, SW_HIDE);

        siege::init_active_input_state();
      }
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
        }
        else
        {
          std::array<siege::platform::game_action, 0> actions{};
          std::array<const wchar_t*, 0> backends{};
          populate_controller_table(actions, backends);
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


  LRESULT exe_view::exe_actions_tbn_dropdown(win32::tool_bar, const NMTOOLBARW& message)
  {
    POINT point{ .x = message.rcButton.left, .y = message.rcButton.top };

    if (ClientToScreen(exe_actions, &point))
    {
      /*auto result = table_settings_menu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, point, ref());

      if (result == 1)
      {
        extract_all_files();
      }*/

      return TBDDRET_DEFAULT;
    }

    return TBDDRET_NODEFAULT;
  }

  std::optional<win32::lresult_t> exe_view::wm_setting_change(win32::setting_change_message message)
  {
    if (message.setting == L"ImmersiveColorSet")
    {
      win32::apply_theme(options);
      win32::apply_theme(exe_actions);
      win32::apply_theme(*this);
      options_unbind(); 
      options_unbind = options.bind_lbn_sel_change(std::bind_front(&exe_view::options_lbn_sel_change, this));

      recreate_image_lists(std::nullopt);
      SendMessageW(exe_actions, TB_SETIMAGELIST, 0, (LPARAM)exe_actions_icons.get());


      return 0;
    }

    return std::nullopt;
  }

  std::wstring category_for_vkey(SHORT vkey)
  {
    if (vkey >= VK_LBUTTON && vkey <= VK_XBUTTON2)
    {
      return L"Mouse";
    }

    // not real vkeys
    if (vkey >= WM_MOUSEMOVE && vkey <= WM_MOUSEMOVE + 4)
    {
      return L"Mouse";
    }

    // not real vkeys
    if (vkey >= WM_MOUSEWHEEL && vkey <= WM_MOUSEWHEEL + 1)
    {
      return L"Mouse";
    }

    // not real vkeys
    if (vkey >= WM_MOUSEHWHEEL && vkey <= WM_MOUSEHWHEEL + 1)
    {
      return L"Mouse";
    }

    if (vkey >= VK_GAMEPAD_A && vkey <= VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
    {
      return L"Controller";
    }

    return L"Keyboard";
  }

  std::wstring string_for_vkey(SHORT vkey)
  {
    constexpr static auto directions = std::array<std::wstring_view, 4>{
      {
        std::wstring_view(L"Up"),
        std::wstring_view(L"Down"),
        std::wstring_view(L"Left"),
        std::wstring_view(L"Right"),
      }
    };

    // not real vkeys
    if (vkey >= WM_MOUSEMOVE && vkey <= WM_MOUSEMOVE + 4)
    {
      std::wstring result(L"Move ");
      result.append(directions[vkey - WM_MOUSEMOVE]);
      return result;
    }

    // not real vkeys
    if (vkey >= WM_MOUSEWHEEL && vkey <= WM_MOUSEWHEEL + 1)
    {
      std::wstring result(L"Scroll ");

      result.append(directions[vkey - WM_MOUSEWHEEL]);
      return result;
    }

    // not real vkeys
    if (vkey >= WM_MOUSEHWHEEL && vkey <= WM_MOUSEHWHEEL + 1)
    {
      std::wstring result(L"Scroll ");

      result.append(directions[vkey - WM_MOUSEHWHEEL + 2]);
      return result;
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