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
      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"D-Pad"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });
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

      std::map<WORD, WORD> default_mappings = {
        { VK_GAMEPAD_DPAD_UP, VK_UP },
        { VK_GAMEPAD_DPAD_DOWN, VK_DOWN },
        { VK_GAMEPAD_DPAD_LEFT, VK_LEFT },
        { VK_GAMEPAD_DPAD_RIGHT, VK_RIGHT },
        { VK_GAMEPAD_A, VK_SPACE },
        { VK_GAMEPAD_B, VK_LCONTROL },
        { VK_GAMEPAD_X, 'F' },
        { VK_GAMEPAD_Y, VK_OEM_4 },
        { VK_GAMEPAD_LEFT_SHOULDER, 'G' },
        { VK_GAMEPAD_RIGHT_SHOULDER, 'T' },
        { VK_GAMEPAD_LEFT_TRIGGER, VK_RBUTTON },
        { VK_GAMEPAD_RIGHT_TRIGGER, VK_LBUTTON },
        { VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, VK_LSHIFT },
        { VK_GAMEPAD_LEFT_THUMBSTICK_UP, 'W' },
        { VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, 'S' },
        { VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, 'A' },
        { VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, 'D' },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, 'E' },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_UP, WM_MOUSEMOVE },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, WM_MOUSEMOVE + 1 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, WM_MOUSEMOVE + 2 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, WM_MOUSEMOVE + 3 },
      };

      std::map<WORD, int> images = {
        { VK_GAMEPAD_DPAD_UP, 3 },
        { VK_GAMEPAD_DPAD_DOWN, 4 },
        { VK_GAMEPAD_DPAD_LEFT, 5 },
        { VK_GAMEPAD_DPAD_RIGHT, 6 },
        { VK_GAMEPAD_A, 13 },
        { VK_GAMEPAD_B, 14 },
        { VK_GAMEPAD_X, 15 },
        { VK_GAMEPAD_Y, 16 },
        { VK_GAMEPAD_LEFT_SHOULDER, 17 },
        { VK_GAMEPAD_RIGHT_SHOULDER, 18 },
        { VK_GAMEPAD_LEFT_TRIGGER, 19 },
        { VK_GAMEPAD_RIGHT_TRIGGER, 20 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, 7 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, 8 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_UP, 9 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, 10 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, 11 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, 12 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_UP, 9 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, 10 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, 11 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, 12 },
      };

      std::map<WORD, int> grouping = {
        { VK_GAMEPAD_DPAD_UP, 1 },
        { VK_GAMEPAD_DPAD_DOWN, 1 },
        { VK_GAMEPAD_DPAD_LEFT, 1 },
        { VK_GAMEPAD_DPAD_RIGHT, 1 },
        { VK_GAMEPAD_A, 2 },
        { VK_GAMEPAD_B, 2 },
        { VK_GAMEPAD_X, 2 },
        { VK_GAMEPAD_Y, 2 },
        { VK_GAMEPAD_LEFT_SHOULDER, 3 },
        { VK_GAMEPAD_RIGHT_SHOULDER, 3 },
        { VK_GAMEPAD_LEFT_TRIGGER, 3 },
        { VK_GAMEPAD_RIGHT_TRIGGER, 3 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, 4 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, 5 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_UP, 4 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, 4 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, 4 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, 4 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_UP, 5 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, 5 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, 5 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, 5 },
      };

      auto selected_image = images.begin();

      for (auto mapping : default_mappings)
      {
        win32::list_view_item up(string_for_vkey(mapping.first), images[mapping.first]);
        up.iGroupId = grouping[mapping.first];
        up.lParam = MAKELPARAM(mapping.first, mapping.second);

        up.sub_items.emplace_back(category_for_vkey(mapping.second));
        up.sub_items.emplace_back(string_for_vkey(mapping.second));
        controller_table.InsertRow(up);
      }

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
            SetTimer(*this, 1, USER_TIMER_MINIMUM, nullptr);
            return 0;
          }

          auto wm_timer(std::size_t id, TIMERPROC)
          {
            constexpr static std::array<SHORT, 8> states = { { VK_RETURN, VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_TAB, VK_SNAPSHOT } };

            for (auto state : states)
            {
              if (::GetKeyState(state) & 0x80)
              {
                KillTimer(*this, 1);
                EndDialog(*this, state);
                break;
              }
            }

            return 0;
          }

          auto wm_mouse_button_down(std::size_t state, POINTS)
          {
            if (state & MK_LBUTTON)
            {
              KillTimer(*this, 1);
              EndDialog(*this, VK_LBUTTON);
            }

            if (state & MK_RBUTTON)
            {
              KillTimer(*this, 1);
              EndDialog(*this, VK_RBUTTON);
            }

            if (state & MK_MBUTTON)
            {
              KillTimer(*this, 1);
              EndDialog(*this, VK_MBUTTON);
            }

            if (state & MK_XBUTTON1)
            {
              KillTimer(*this, 1);
              EndDialog(*this, VK_XBUTTON1);
            }

            if (state & MK_XBUTTON2)
            {
              KillTimer(*this, 1);
              EndDialog(*this, MK_XBUTTON2);
            }

            return 0;
          }

          auto wm_sys_key_down(KEYBDINPUT input)
          {
            KillTimer(*this, 1);

            if (input.wVk == VK_MENU)
            {
              if (::GetKeyState(VK_LMENU) & 0x80)
              {
                EndDialog(*this, VK_LMENU);
              }
              else if (::GetKeyState(VK_RMENU) & 0x80)
              {
                EndDialog(*this, VK_RMENU);
              }
            }
            else
            {
              EndDialog(*this, input.wVk);
            }

            return 0;
          }

          auto wm_key_down(KEYBDINPUT input)
          {
            KillTimer(*this, 1);

            if (input.wVk == VK_SHIFT)
            {
              if (::GetKeyState(VK_LSHIFT) & 0x80)
              {
                EndDialog(*this, VK_LSHIFT);
              }
              else if (::GetKeyState(VK_RSHIFT) & 0x80)
              {
                EndDialog(*this, VK_RSHIFT);
              }
            }
            else if (input.wVk == VK_CONTROL)
            {
              if (::GetKeyState(VK_LCONTROL) & 0x80)
              {
                EndDialog(*this, VK_LCONTROL);
              }
              else if (::GetKeyState(VK_RCONTROL) & 0x80)
              {
                EndDialog(*this, VK_RCONTROL);
              }
            }
            else
            {
              EndDialog(*this, input.wVk);
            }
            return 0;
          }
        };

        auto result = win32::DialogBoxIndirectParamW<handler>(win32::module_ref::current_application(),
          win32::default_dialog({ .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 200, .cy = 100 }),
          ref());

        auto item = controller_table.GetItem(LVITEMW{
          .mask = LVIF_PARAM,
          .iItem = message.iItem });

        if (item && item->lParam)
        {
          auto game_pad_code = LOWORD(item->lParam);

          std::wstring temp = category_for_vkey(result);
          ListView_SetItemText(controller_table, message.iItem, 1, temp.data());
          temp = string_for_vkey(result);
          ListView_SetItemText(controller_table, message.iItem, 2, temp.data());

          LVITEMW item{
            .mask = LVIF_PARAM,
            .iItem = message.iItem,
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
              .controller_key_mappings = std::move(input_mapping),
              .extension = &controller.get_extension()
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