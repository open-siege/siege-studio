#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <map>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/dialog.hpp>
#include "exe_controller.hpp"
#include "input-filter.hpp"
#include "input_injector.hpp"

namespace siege::views
{
  using namespace std::literals;

  struct exe_view final : win32::window_ref
    , win32::list_box::notifications
    , win32::list_view::notifications
    , win32::tool_bar::notifications
  {
    exe_controller controller;

    win32::list_box options;
    win32::list_view resource_table;
    win32::list_view string_table;
    win32::list_view launch_table;
    win32::list_view keyboard_table;
    win32::list_view controller_table;
    win32::tool_bar exe_actions;
    win32::image_list image_list;

    constexpr static int launch_selected_id = 10;
    constexpr static int extract_selected_id = 11;

    std::map<std::wstring_view, std::wstring_view> group_names = {
      { L"#1"sv, L"Hardware Dependent Cursor"sv },
      { L"#2"sv, L"Bitmap"sv },
      { L"#3"sv, L"Hardware Dependent Icon"sv },
      { L"#4"sv, L"Menu"sv },
      { L"#5"sv, L"Dialog"sv },
      { L"#6"sv, L"String Table"sv },
      { L"#8"sv, L"Font"sv },
      { L"#9"sv, L"Accelerator"sv },
      { L"#10"sv, L"Raw Data"sv },
      { L"#12"sv, L"Hardware Independent Cursor"sv },
      { L"#14"sv, L"Hardware Independent Icon"sv },
      { L"#16"sv, L"Version"sv },
      { L"#22"sv, L"Animated Icon"sv },
      { L"#23"sv, L"HTML"sv },
      { L"#24"sv, L"Side-by-Side Assembly Manifest"sv },
    };

    exe_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      options = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS });

      options.InsertString(-1, L"Resources");
      options.SetCurrentSelection(0);

      exe_actions = *control_factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });

      exe_actions.InsertButton(-1, { .iBitmap = 0, .idCommand = launch_selected_id, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Launch" }, false);
      exe_actions.InsertButton(-1, { .iBitmap = 1, .idCommand = extract_selected_id, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Extract" }, false);
      exe_actions.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);

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

      launch_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Type"),
                                    });

      launch_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Name"),
                                    });

      launch_table.InsertColumn(-1, LVCOLUMNW{
                                      .pszText = const_cast<wchar_t*>(L"Value"),
                                    });

      keyboard_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS });

      controller_table = *control_factory.CreateWindowExW<win32::list_view>({ .style = WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS });

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
      int id = 1;
      assert(controller_table.InsertGroup(-1, LVGROUP{
                                                .pszHeader = const_cast<wchar_t*>(L"D-Pad"),
                                                .iGroupId = id++,
                                                .state = LVGS_COLLAPSIBLE,
                                              })
             != -1);
      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Face Buttons"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });

      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Bumpers and Triggers"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });
      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Left Stick"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });

      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Right Stick"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });

      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"System Buttons"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });


      win32::list_view_item up(L"D-pad Up", 3);
      up.iGroupId = 1;
      up.lParam = MAKELPARAM(VK_GAMEPAD_DPAD_UP, VK_UP);
      up.sub_items.emplace_back(L"Keyboard");
      up.sub_items.emplace_back(L"Up arrow");
      controller_table.InsertRow(up);

      win32::list_view_item down(L"D-pad Down", 4);
      down.iGroupId = 1;
      down.lParam = MAKELPARAM(VK_GAMEPAD_DPAD_DOWN, VK_DOWN);
      down.sub_items.emplace_back(L"Keyboard");
      down.sub_items.emplace_back(L"Down arrow");
      controller_table.InsertRow(down);

      win32::list_view_item left(L"D-pad Left", 5);
      left.iGroupId = 1;
      left.lParam = MAKELPARAM(VK_GAMEPAD_DPAD_LEFT, VK_LEFT);
      left.sub_items.emplace_back(L"Keyboard");
      left.sub_items.emplace_back(L"Left arrow");
      controller_table.InsertRow(left);

      win32::list_view_item right(L"D-pad Right", 6);
      right.iGroupId = 1;
      right.lParam = MAKELPARAM(VK_GAMEPAD_DPAD_RIGHT, VK_RIGHT);
      right.sub_items.emplace_back(L"Keyboard");
      right.sub_items.emplace_back(L"Right arrow");
      controller_table.InsertRow(right);

      win32::list_view_item a(L"A Button", 13);
      a.iGroupId = 2;
      a.lParam = MAKELPARAM(VK_GAMEPAD_A, VK_SPACE);
      a.sub_items.emplace_back(L"Keyboard");
      a.sub_items.emplace_back(L"Spacebar");
      controller_table.InsertRow(a);

      win32::list_view_item b(L"B Button", 14);
      b.iGroupId = 2;
      b.lParam = MAKELPARAM(VK_GAMEPAD_B, VK_LCONTROL);
      b.sub_items.emplace_back(L"Keyboard");
      b.sub_items.emplace_back(L"Left control");
      controller_table.InsertRow(b);


      win32::list_view_item x(L"X Button", 15);
      x.iGroupId = 2;
      x.lParam = MAKELPARAM(VK_GAMEPAD_X, 'F');
      x.sub_items.emplace_back(L"Keyboard");
      x.sub_items.emplace_back(L"F");
      controller_table.InsertRow(x);


      win32::list_view_item y(L"Y Button", 16);
      y.iGroupId = 2;
      y.lParam = MAKELPARAM(VK_GAMEPAD_Y, '[');
      y.sub_items.emplace_back(L"Keyboard");
      y.sub_items.emplace_back(L"[");
      controller_table.InsertRow(y);

      win32::list_view_item lb(L"Left Bumper", 17);
      lb.iGroupId = 3;
      lb.lParam = MAKELPARAM(VK_GAMEPAD_LEFT_SHOULDER, 'G');
      lb.sub_items.emplace_back(L"Keyboard");
      lb.sub_items.emplace_back(L"G");
      controller_table.InsertRow(lb);


      win32::list_view_item rb(L"Right Bumper", 18);
      rb.iGroupId = 3;
      rb.lParam = MAKELPARAM(VK_GAMEPAD_RIGHT_SHOULDER, 'T');
      rb.sub_items.emplace_back(L"Keyboard");
      rb.sub_items.emplace_back(L"T");
      controller_table.InsertRow(rb);

      win32::list_view_item lt(L"Left Trigger", 19);
      lt.iGroupId = 3;
      lt.lParam = MAKELPARAM(VK_GAMEPAD_LEFT_TRIGGER, VK_RBUTTON);
      lt.sub_items.emplace_back(L"Mouse");
      lt.sub_items.emplace_back(L"Right click");
      controller_table.InsertRow(lt);

      win32::list_view_item rt(L"Right Trigger", 20);
      rt.iGroupId = 3;
      rt.lParam = MAKELPARAM(VK_GAMEPAD_RIGHT_TRIGGER, VK_LBUTTON);
      rt.sub_items.emplace_back(L"Mouse");
      rt.sub_items.emplace_back(L"Left click");
      controller_table.InsertRow(rt);

      win32::list_view_item ls(L"Left Stick Button", 7);
      ls.iGroupId = 4;
      ls.lParam = MAKELPARAM(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, VK_LSHIFT);
      ls.sub_items.emplace_back(L"Keyboard");
      ls.sub_items.emplace_back(L"Left shift");
      controller_table.InsertRow(ls);

      win32::list_view_item lsu(L"Left Stick Up", 9);
      lsu.iGroupId = 4;
      lsu.lParam = MAKELPARAM(VK_GAMEPAD_LEFT_THUMBSTICK_UP, 'W');
      lsu.sub_items.emplace_back(L"Keyboard");
      lsu.sub_items.emplace_back(L"W");
      controller_table.InsertRow(lsu);

      win32::list_view_item lsd(L"Left Stick Down", 10);
      lsd.iGroupId = 4;
      lsd.lParam = MAKELPARAM(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, 'S');
      lsd.sub_items.emplace_back(L"Keyboard");
      lsd.sub_items.emplace_back(L"S");
      controller_table.InsertRow(lsd);

      win32::list_view_item lsl(L"Left Stick Left", 10);
      lsl.iGroupId = 4;
      lsl.lParam = MAKELPARAM(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, 'A');
      lsl.sub_items.emplace_back(L"Keyboard");
      lsl.sub_items.emplace_back(L"A");
      controller_table.InsertRow(lsl);

      win32::list_view_item lsr(L"Left Stick Right", 10);
      lsr.iGroupId = 4;
      lsr.lParam = MAKELPARAM(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, 'D');
      lsr.sub_items.emplace_back(L"Keyboard");
      lsr.sub_items.emplace_back(L"D");
      controller_table.InsertRow(lsr);

      win32::list_view_item rs(L"Right Stick Button", 8);
      rs.iGroupId = 5;
      rs.lParam = MAKELPARAM(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, 'E');
      rs.sub_items.emplace_back(L"Keyboard");
      rs.sub_items.emplace_back(L"E");
      controller_table.InsertRow(rs);

      win32::list_view_item rsu(L"Right Stick Up", 9);
      rsu.iGroupId = 5;
      rsu.lParam = MAKELPARAM(VK_GAMEPAD_RIGHT_THUMBSTICK_UP, WM_MOUSEMOVE);
      rsu.sub_items.emplace_back(L"Mouse");
      rsu.sub_items.emplace_back(L"Mouse up");
      controller_table.InsertRow(rsu);

      win32::list_view_item rsd(L"Right Stick Down", 10);
      rsd.iGroupId = 5;
      rsd.lParam = MAKELPARAM(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, WM_MOUSEMOVE + 1);
      rsd.sub_items.emplace_back(L"Mouse");
      rsd.sub_items.emplace_back(L"Mouse down");
      controller_table.InsertRow(rsd);

      win32::list_view_item rsl(L"Right Stick Left", 11);
      rsl.iGroupId = 5;
      rsl.lParam = MAKELPARAM(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, WM_MOUSEMOVE + 2);
      rsl.sub_items.emplace_back(L"Mouse");
      rsl.sub_items.emplace_back(L"Mouse left");
      controller_table.InsertRow(rsl);

      win32::list_view_item rsr(L"Right Stick Right", 12);
      rsr.iGroupId = 5;
      rsr.lParam = MAKELPARAM(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, WM_MOUSEMOVE + 3);
      rsr.sub_items.emplace_back(L"Mouse");
      rsr.sub_items.emplace_back(L"Mouse right");
      controller_table.InsertRow(rsr);

      LVTILEVIEWINFO tileViewInfo = { 0 };

      tileViewInfo.cbSize = sizeof(tileViewInfo);
      tileViewInfo.dwMask = LVTVIM_COLUMNS;
      tileViewInfo.dwFlags = LVTVIF_AUTOSIZE;
      tileViewInfo.cLines = 2;

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

    auto wm_size(std::size_t type, SIZE client_size)
    {
      auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 12 };

      auto one_quarter = SIZE{ .cx = client_size.cx / 4, .cy = client_size.cy - top_size.cy };
      auto three_quarters = SIZE{ .cx = client_size.cx - one_quarter.cx, .cy = client_size.cy - top_size.cy };

      recreate_image_list(exe_actions.GetIdealIconSize(SIZE{ .cx = client_size.cx / exe_actions.ButtonCount(), .cy = top_size.cy }));
      SendMessageW(exe_actions, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());

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
      controller_table.SetImageList(LVSIL_NORMAL, image_list);

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

    void recreate_image_list(std::optional<SIZE> possible_size)
    {

      SIZE icon_size = possible_size.or_else([this] {
                                      return image_list.GetIconSize();
                                    })
                         .or_else([] {
                           return std::make_optional(SIZE{
                             .cx = ::GetSystemMetrics(SM_CXSIZE),
                             .cy = ::GetSystemMetrics(SM_CYSIZE) });
                         })
                         .value();

      if (image_list)
      {
        image_list.reset();
      }

      std::vector icons{
        win32::segoe_fluent_icons::new_window,
        win32::segoe_fluent_icons::folder_open,
        win32::segoe_fluent_icons::dpad,// 2
        win32::segoe_fluent_icons::caret_solid_up,
        win32::segoe_fluent_icons::caret_solid_down,
        win32::segoe_fluent_icons::caret_solid_left,
        win32::segoe_fluent_icons::caret_solid_right,
        win32::segoe_fluent_icons::left_stick,// 7
        win32::segoe_fluent_icons::right_stick,
        win32::segoe_fluent_icons::arrow_up,// 9
        win32::segoe_fluent_icons::arrow_down,
        win32::segoe_fluent_icons::arrow_left,
        win32::segoe_fluent_icons::arrow_right,
        win32::segoe_fluent_icons::button_a,// 13
        win32::segoe_fluent_icons::button_b,
        win32::segoe_fluent_icons::button_x,
        win32::segoe_fluent_icons::button_y,
        win32::segoe_fluent_icons::bumper_left,// 17
        win32::segoe_fluent_icons::bumper_right,
        win32::segoe_fluent_icons::trigger_left,// 19
        win32::segoe_fluent_icons::trigger_right,
      };

      image_list = win32::create_icon_list(icons, icon_size);
    }


    std::optional<win32::lresult_t> wm_command(win32::list_box hwndFrom, int code) override
    {
      if (code == LBN_SELCHANGE && hwndFrom == options)
      {
        auto selected = options.GetCurrentSelection();
        ::ShowWindow(resource_table, SW_HIDE);
        ::ShowWindow(string_table, SW_HIDE);
        ::ShowWindow(launch_table, SW_HIDE);
        ::ShowWindow(controller_table, SW_HIDE);

        std::array tables{ resource_table.ref(), string_table.ref(), launch_table.ref(), keyboard_table.ref(), controller_table.ref() };

        if (selected < tables.size())
        {
          ::ShowWindow(tables[selected], SW_SHOW);
        }

        return 0;
      }

      return std::nullopt;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      std::optional<std::filesystem::path> path;

      if (wchar_t* filename = this->GetPropW<wchar_t*>(L"FilePath"); filename)
      {
        path = filename;

        if (options.GetCount() < 3 && (path->extension() == ".exe" || path->extension() == ".EXE"))
        {
          options.InsertString(-1, L"Scripting");
          options.InsertString(-1, L"Launch Options");
          options.InsertString(-1, L"Keyboard/Mouse Settings");
          options.InsertString(-1, L"Controller");

          siege::init_active_input_state();
        }
      }

      auto count = controller.load_executable(stream, std::move(path));

      if (count > 0)
      {
        auto values = controller.get_resource_names();

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

    std::optional<win32::lresult_t> wm_notify(win32::list_view sender, const NMITEMACTIVATE& message) override
    {
      if (sender == controller_table)
      {
        struct handler : win32::window_ref
        {
          handler(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
          {
          }

          auto wm_create()
          {
            SetWindowTextW(*this, L"Press a keyboard or mouse button");
            return 0;
          }

          auto wm_key_down(KEYBDINPUT input)
          {
            EndDialog(*this, input.wVk);
            return 0;
          }
        };

        auto result = win32::DialogBoxIndirectParamW<handler>(win32::module_ref::current_application(),
          win32::default_dialog({ .cx = 100, .cy = 50 }),
          ref());

        auto game_pad_code = LOWORD(message.lParam);


        if (result >= WM_MOUSEMOVE && result <= WM_MOUSEMOVE + 3)
        {
          std::wstring temp = L"Mouse";
          ListView_SetItemText(controller_table, message.iItem, 1, temp.data());
          temp = L"Mouse Up";
          ListView_SetItemText(controller_table, message.iItem, 2, temp.data());
        }
        else
        {

          std::wstring temp = L"Keyboard";
          ListView_SetItemText(controller_table, message.iItem, 1, temp.data());
          temp.clear();
          temp.push_back(result);
          ListView_SetItemText(controller_table, message.iItem, 2, temp.data());
          
          LVITEMW item{
            .mask = LVIF_PARAM,
            .lParam = MAKELPARAM(game_pad_code, result)
          };
          ListView_SetItem(controller_table, &item);
        }

        return 0;
      }

      return std::nullopt;
    }

    std::optional<win32::lresult_t> wm_notify(win32::tool_bar, const NMTOOLBARW& message) override
    {
      switch (message.hdr.code)
      {
      case TBN_DROPDOWN: {
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
      default: {
        return FALSE;
      }
      }
    }

    std::optional<BOOL> wm_notify(win32::tool_bar, const NMMOUSE& message) override
    {
      switch (message.hdr.code)
      {
      case NM_CLICK: {
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

            input_injector_args args{
              .exe_path = controller.get_exe_path(),
              .extension_path = controller.get_extension().GetModuleFileName(),
              .controller_key_mappings = std::move(input_mapping)
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
      default: {
        return FALSE;
      }
      }
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(resource_table);
        win32::apply_theme(launch_table);
        win32::apply_theme(string_table);
        win32::apply_theme(controller_table);
        win32::apply_theme(options);
        win32::apply_theme(exe_actions);
        win32::apply_theme(*this);

        recreate_image_list(std::nullopt);
        SendMessageW(exe_actions, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());


        return 0;
      }

      return std::nullopt;
    }
  };
}// namespace siege::views

#endif