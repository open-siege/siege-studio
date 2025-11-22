// WIP - this file is still work in progress and messy

#include <siege/platform/win/basic_window.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <hidusage.h>
#include "exe_shared.hpp"

namespace siege::views
{
  using controller_state = siege::views::controller_state;
  using controller_context = siege::platform::controller_context;


  struct connected_devices_dialog : win32::basic_window<connected_devices_dialog>
  {
    struct controller_item
    {
      win32::wparam_t item;
      controller_state state;
    };

    struct controller_mapping
    {
      win32::wparam_t item;
      bool visible = false;
      hardware_index index;
      WORD vkey;
    };

    win32::list_view device_selection;
    win32::tool_bar device_context;
    win32::list_view custom_mapping;
    std::map<HANDLE, win32::wparam_t> registered_keyboards;
    std::map<HANDLE, win32::wparam_t> registered_mice;
    std::map<HANDLE, controller_item> registered_controllers;
    std::vector<controller_mapping> mappings;
    std::span<controller_mapping> button_mappings;
    std::span<controller_mapping> axis_mappings;

    struct
    {
      win32::button a_button;
      win32::button b_button;
      win32::button x_button;
      win32::button y_button;

      win32::button dpad_up_button;
      win32::button dpad_down_button;
      win32::button dpad_left_button;
      win32::button dpad_right_button;

      win32::button view_button;
      win32::button menu_button;
      win32::button left_bumper;
      win32::button right_bumper;

      win32::track_bar left_trigger;
      win32::track_bar right_trigger;

      win32::track_bar left_x_axis_left;
      win32::track_bar left_x_axis_right;
      win32::track_bar left_y_axis_up;
      win32::track_bar left_y_axis_down;
      win32::button left_stick_button;
      win32::track_bar right_x_axis_left;
      win32::track_bar right_x_axis_right;
      win32::track_bar right_y_axis_up;
      win32::track_bar right_y_axis_down;
      win32::button right_stick_button;

      std::array<win32::window_ref, 24> to_array()
      {
        return { { a_button.ref(),
          b_button.ref(),
          x_button.ref(),
          y_button.ref(),
          dpad_up_button.ref(),
          dpad_down_button.ref(),
          dpad_left_button.ref(),
          dpad_right_button.ref(),
          view_button.ref(),
          menu_button.ref(),
          left_bumper.ref(),
          right_bumper.ref(),
          left_trigger.ref(),
          right_trigger.ref(),
          left_y_axis_up.ref(),
          left_y_axis_down.ref(),
          left_x_axis_left.ref(),
          left_x_axis_right.ref(),
          left_stick_button.ref(),
          right_y_axis_up.ref(),
          right_y_axis_down.ref(),
          right_x_axis_left.ref(),
          right_x_axis_right.ref(),
          right_stick_button.ref() } };
      }

      std::array<win32::window_ref, 10> track_bars()
      {
        return { { left_trigger.ref(),
          right_trigger.ref(),
          left_y_axis_up.ref(),
          left_y_axis_down.ref(),
          left_x_axis_left.ref(),
          left_x_axis_right.ref(),
          right_y_axis_up.ref(),
          right_y_axis_down.ref(),
          right_x_axis_left.ref(),
          right_x_axis_right.ref() } };
      }


      void show_all()
      {
        for (auto& item : to_array())
        {
          item.SetWindowStyle(item.GetWindowStyle() | WS_VISIBLE);
        }
      }

      void hide_all()
      {
        for (auto& item : to_array())
        {
          item.SetWindowStyle(item.GetWindowStyle() & ~WS_VISIBLE);
        }
      }

      void set_size_for_all(SIZE size)
      {
        for (auto& item : to_array())
        {
          item.SetWindowPos(size);
        }
      }

    } controller_controls;

    connected_devices_dialog(win32::hwnd_t self, CREATESTRUCTW& params) : basic_window(self, params)
    {
      mappings.reserve(32);
    }

    LRESULT wm_create()
    {
      ::SetWindowTextW(*this, L"Connected Inputs");
      win32::apply_window_theme(*this);

      DWORD flags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
      std::array<RAWINPUTDEVICE, 4> descriptors{ {
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_GAMEPAD,
          .dwFlags = flags,
          .hwndTarget = *this },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_JOYSTICK,
          .dwFlags = flags,
          .hwndTarget = *this },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_MOUSE,
          .dwFlags = flags,
          .hwndTarget = *this },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_KEYBOARD,
          .dwFlags = flags,
          .hwndTarget = *this },
      } };

      if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
      {
        return -1;
      }

      device_selection = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
        .style = WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

      device_selection.InsertColumn(-1, LVCOLUMNW{
                                          .pszText = const_cast<wchar_t*>(L"Name"),
                                        });

      device_selection.InsertColumn(-1, LVCOLUMNW{
                                          .pszText = const_cast<wchar_t*>(L"Device Type"),
                                        });

      device_selection.EnableGroupView(true);

      device_selection.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Keyboards"),
                                         .iGroupId = 1,
                                         .state = LVGS_COLLAPSIBLE,
                                       });
      device_selection.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Mice"),
                                         .iGroupId = 2,
                                         .state = LVGS_COLLAPSIBLE,
                                       });

      device_selection.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Controllers"),
                                         .iGroupId = 3,
                                         .state = LVGS_COLLAPSIBLE,
                                       });


      device_selection.bind_nm_click([&](auto, const NMITEMACTIVATE& event) {
        auto item = std::find_if(registered_controllers.begin(), registered_controllers.end(), [&](auto& controller) {
          return controller.second.item == (win32::wparam_t)event.iItem;
        });

        if (item == registered_controllers.end())
        {
          controller_controls.hide_all();
          device_context.SetWindowStyle(device_context.GetWindowStyle() & ~WS_VISIBLE);
          custom_mapping.SetWindowStyle(custom_mapping.GetWindowStyle() & ~WS_VISIBLE);
          return;
        }

        controller_controls.show_all();
        device_context.SetWindowStyle(device_context.GetWindowStyle() | WS_VISIBLE);
        custom_mapping.SetWindowStyle(custom_mapping.GetWindowStyle() | WS_VISIBLE);

        auto context = item->second.state.info.detected_context;
        ::SendMessageW(device_context, TB_SETSTATE, (WPARAM)context, MAKELPARAM(TBSTATE_CHECKED, 0));
        update_mappings_for_context(item->second.state.info);
      });

      device_context = *win32::CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });

      device_context.InsertButton(-1, {
                                        .idCommand = (int)controller_context::controller_xbox,
                                        .fsState = TBSTATE_ENABLED,
                                        .fsStyle = BTNS_CHECKGROUP,
                                        .iString = (INT_PTR)L"Xbox",
                                      },
        false);
      device_context.InsertButton(-1, { .idCommand = (int)controller_context::controller_playstation_3, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Playstation 3" }, false);
      device_context.InsertButton(-1, { .idCommand = (int)controller_context::controller_playstation_4, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Playstation 4" }, false);
      device_context.InsertButton(-1, { .idCommand = (int)controller_context::joystick, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Joystick" }, false);
      device_context.InsertButton(-1, { .idCommand = (int)controller_context::throttle, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Throttle" }, false);
      device_context.InsertButton(-1, { .idCommand = (int)controller_context::steering_wheel, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Steering Wheel" }, false);
      device_context.InsertButton(-1, { .idCommand = (int)controller_context::pedal, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Pedals" }, false);
      device_context.InsertButton(-1, { .idCommand = (int)controller_context::custom, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECKGROUP, .iString = (INT_PTR)L"Custom" }, false);

      custom_mapping = *win32::CreateWindowExW<win32::list_view>({ .hwndParent = *this,
        .style = WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER });

      custom_mapping.InsertColumn(-1, LVCOLUMNW{
                                        .pszText = const_cast<wchar_t*>(L"Input Name"),
                                      });

      custom_mapping.InsertColumn(-1, LVCOLUMNW{
                                        .pszText = const_cast<wchar_t*>(L"Controller Mapping"),
                                      });

      custom_mapping.EnableGroupView(true);

      custom_mapping.InsertGroup(-1, LVGROUP{
                                       .pszHeader = const_cast<wchar_t*>(L"Hidden"),
                                       .iGroupId = 3,
                                       .state = LVGS_HIDDEN | LVGS_NOHEADER | LVGS_COLLAPSED,
                                     });

      custom_mapping.InsertGroup(-1, LVGROUP{
                                       .pszHeader = const_cast<wchar_t*>(L"Buttons"),
                                       .iGroupId = 1,
                                       .state = LVGS_COLLAPSIBLE,
                                     });

      for (auto i = 1; i <= 32; ++i)
      {
        win32::list_view_item button_item(L"Button " + std::to_wstring(i));
        button_item.sub_items.emplace_back(L"N/A");
        button_item.iGroupId = 1;
        button_item.mask = button_item.mask | LVIF_GROUPID;
        auto index = custom_mapping.InsertRow(button_item);

        mappings.emplace_back().item = index;
      }
      button_mappings = std::span<controller_mapping>(mappings.begin(), 32);

      custom_mapping.InsertGroup(-1, LVGROUP{
                                       .pszHeader = const_cast<wchar_t*>(L"Axes"),
                                       .iGroupId = 2,
                                       .state = LVGS_COLLAPSIBLE,
                                     });

      for (auto i = 1; i <= 16; ++i)
      {
        win32::list_view_item axis_item(L"Axis " + std::to_wstring(i));
        axis_item.sub_items.emplace_back(L"N/A");
        axis_item.iGroupId = 2;
        axis_item.mask = axis_item.mask | LVIF_GROUPID;
        auto index = custom_mapping.InsertRow(axis_item);
        mappings.emplace_back().item = index;
      }
      axis_mappings = std::span<controller_mapping>(mappings.begin() + button_mappings.size(), 16);

      // TODO add an icon for each button
      // TODO have different labels for certain contexts
      controller_controls.a_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"A" });

      controller_controls.b_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"B" });

      controller_controls.x_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"X" });

      controller_controls.y_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"Y" });

      controller_controls.dpad_up_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"D-pad Up" });

      controller_controls.dpad_down_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"D-Pad Down" });

      controller_controls.dpad_left_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"D-Pad Left" });

      controller_controls.dpad_right_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"D-Pad Right" });

      controller_controls.left_bumper = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"LB" });

      controller_controls.right_bumper = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"RB" });

      controller_controls.view_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"View" });

      controller_controls.menu_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"Menu" });

      auto lparam = MAKELPARAM(0, 32767);
      controller_controls.left_trigger = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_VERT | TBS_NOTICKS | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"LT" });
      ::SendMessageW(controller_controls.left_trigger, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));

      controller_controls.right_trigger = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_VERT | TBS_NOTICKS | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"RT" });
      ::SendMessageW(controller_controls.right_trigger, TBM_SETRANGE, FALSE, MAKELPARAM(0, 255));

      controller_controls.left_x_axis_left = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_HORZ | TBS_NOTICKS | TBS_REVERSED | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"LS Left" });
      ::SendMessageW(controller_controls.left_x_axis_left, TBM_SETRANGE, FALSE, lparam);

      controller_controls.left_x_axis_right = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_HORZ | TBS_NOTICKS | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"LS Right" });
      ::SendMessageW(controller_controls.left_x_axis_right, TBM_SETRANGE, FALSE, lparam);

      controller_controls.left_y_axis_up = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_VERT | TBS_NOTICKS | TBS_REVERSED | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"LS Up" });
      ::SendMessageW(controller_controls.left_y_axis_up, TBM_SETRANGE, FALSE, lparam);

      controller_controls.left_y_axis_down = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_VERT | TBS_NOTICKS | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"LS Down" });
      ::SendMessageW(controller_controls.left_y_axis_down, TBM_SETRANGE, FALSE, lparam);

      controller_controls.right_x_axis_left = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_HORZ | TBS_NOTICKS | TBS_REVERSED | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"RS Left" });
      ::SendMessageW(controller_controls.right_x_axis_left, TBM_SETRANGE, FALSE, lparam);

      controller_controls.right_x_axis_right = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_HORZ | TBS_NOTICKS | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"RS Right" });
      ::SendMessageW(controller_controls.right_x_axis_right, TBM_SETRANGE, FALSE, lparam);

      controller_controls.right_y_axis_up = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_VERT | TBS_NOTICKS | TBS_REVERSED | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"RS Up" });
      ::SendMessageW(controller_controls.right_y_axis_up, TBM_SETRANGE, FALSE, lparam);

      controller_controls.right_y_axis_down = *win32::CreateWindowExW<win32::track_bar>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | TBS_VERT | TBS_NOTICKS | TBS_ENABLESELRANGE | TBS_NOTHUMB,
        .lpszName = L"RS Down" });
      ::SendMessageW(controller_controls.right_y_axis_down, TBM_SETRANGE, FALSE, lparam);

      controller_controls.left_stick_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"LS" });

      controller_controls.right_stick_button = *win32::CreateWindowExW<win32::button>({ .hwndParent = *this,
        .style = WS_CHILD | WS_DISABLED | BS_PUSHLIKE,
        .lpszName = L"RS" });

      return 0;
    }

    LRESULT wm_destroy()
    {
      DWORD flags = RIDEV_REMOVE;
      std::array<RAWINPUTDEVICE, 4> descriptors{ {
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_GAMEPAD,
          .dwFlags = flags },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_JOYSTICK,
          .dwFlags = flags },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_MOUSE,
          .dwFlags = flags },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_KEYBOARD,
          .dwFlags = flags },
      } };

      ::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE));
      return 0;
    }

    LRESULT wm_close()
    {
      ::EndDialog(*this, 0);
      return 0;
    }

    std::optional<win32::lresult_t> wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MINIMIZED || type == SIZE_MAXHIDE || type == SIZE_MAXSHOW)
      {
        return std::nullopt;
      }

      auto one_quarter = SIZE{ .cx = client_size.cx / 4, .cy = client_size.cy };

      device_selection.SetWindowPos(POINT{});
      device_selection.SetWindowPos(one_quarter);

      auto column_count = device_selection.GetColumnCount();

      if (!column_count)
      {
        return 0;
      }

      auto column_width = one_quarter.cx / column_count;

      for (auto i = 0u; i < column_count; ++i)
      {
        device_selection.SetColumnWidth(i, column_width);
      }

      auto current_item = ListView_GetNextItem(device_selection, -1, LVNI_SELECTED);

      auto remaining = SIZE{ .cx = (client_size.cx - one_quarter.cx) / 4, .cy = client_size.cy };
      auto button_size = SIZE{ .cx = remaining.cx / 4, .cy = remaining.cx / 4 };

      auto starting = POINT{ .x = one_quarter.cx };

      controller_controls.set_size_for_all(button_size);
      controller_controls.left_trigger.SetWindowPos(POINT{ .x = starting.x + button_size.cx });
      controller_controls.right_trigger.SetWindowPos(POINT{ .x = starting.x + button_size.cx * 13 });
      controller_controls.left_bumper.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = button_size.cy });
      controller_controls.view_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx * 5, .y = button_size.cy });
      controller_controls.menu_button.SetWindowPos(button_size);
      controller_controls.menu_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx * 9, .y = button_size.cy });
      controller_controls.right_bumper.SetWindowPos(button_size);
      controller_controls.right_bumper.SetWindowPos(POINT{ .x = starting.x + button_size.cx * 13, .y = button_size.cy });

      starting = POINT{ .x = one_quarter.cx, .y = button_size.cy * 3 };

      controller_controls.dpad_up_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y });
      controller_controls.dpad_left_button.SetWindowPos(POINT{ .x = starting.x, .y = starting.y + button_size.cy });
      controller_controls.dpad_down_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy + button_size.cy });
      controller_controls.dpad_right_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx + button_size.cx, .y = starting.y + button_size.cy });

      starting = POINT{ .x = starting.x + button_size.cx * 4, .y = starting.y };

      controller_controls.left_y_axis_up.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y });
      controller_controls.left_y_axis_down.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy + button_size.cy });
      controller_controls.left_stick_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy });
      controller_controls.left_x_axis_left.SetWindowPos(POINT{ .x = starting.x, .y = starting.y + button_size.cy });
      controller_controls.left_x_axis_right.SetWindowPos(POINT{ .x = starting.x + button_size.cx + button_size.cx, .y = starting.y + button_size.cy });

      starting = POINT{ .x = starting.x + button_size.cx * 4, .y = starting.y };

      controller_controls.right_y_axis_up.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y });
      controller_controls.right_y_axis_down.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy + button_size.cy });
      controller_controls.right_stick_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy });
      controller_controls.right_x_axis_left.SetWindowPos(POINT{ .x = starting.x, .y = starting.y + button_size.cy });
      controller_controls.right_x_axis_right.SetWindowPos(POINT{ .x = starting.x + button_size.cx + button_size.cx, .y = starting.y + button_size.cy });

      starting = POINT{ .x = starting.x + button_size.cx * 4, .y = starting.y };

      controller_controls.x_button.SetWindowPos(POINT{ .x = starting.x, .y = starting.y + button_size.cy });
      controller_controls.y_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y });
      controller_controls.a_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy + button_size.cy });
      controller_controls.b_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx + button_size.cx, .y = starting.y + button_size.cy });

      starting = POINT{ .x = one_quarter.cx, .y = starting.y + button_size.cy * 3 };

      device_context.SetWindowPos(starting);

      device_context.SetWindowPos(SIZE{ .cx = client_size.cx - one_quarter.cx, .cy = button_size.cy });

      starting = POINT{ .x = one_quarter.cx, .y = starting.y + button_size.cy };
      custom_mapping.SetWindowPos(starting);
      custom_mapping.SetWindowPos(SIZE{ .cx = client_size.cx - one_quarter.cx, .cy = remaining.cy - starting.y });

      auto mapping_count = custom_mapping.GetColumnCount();

      auto mapping_width = (client_size.cx - one_quarter.cx) / mapping_count;

      for (auto i = 0u; i < mapping_count; ++i)
      {
        custom_mapping.SetColumnWidth(i, mapping_width);
      }

      return 0;
    }

    std::optional<win32::lresult_t> wm_input(HRAWINPUT raw_input)
    {
      RAWINPUTHEADER header{};

      UINT size = sizeof(header);

      if (::GetRawInputData(raw_input, RID_HEADER, &header, &size, sizeof(header)) < 0)
      {
        return 0;
      }

      if (header.dwType == RIM_TYPEKEYBOARD || header.dwType == RIM_TYPEMOUSE)
      {
        return 0;
      }

      auto info = registered_controllers.find(header.hDevice);

      if (info == registered_controllers.end())
      {
        return 0;
      }

      auto new_state = get_current_state_for_handle(info->second.state, raw_input);

      auto diff = get_changes(info->second.state.last_state, new_state, info->second.state.buffer);

      if (diff.empty())
      {
        return 0;
      }

      info->second.state.last_state = std::move(new_state);

      Button_SetState(controller_controls.a_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_A ? TRUE : FALSE);
      Button_SetState(controller_controls.b_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_B ? TRUE : FALSE);
      Button_SetState(controller_controls.x_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_X ? TRUE : FALSE);
      Button_SetState(controller_controls.y_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_Y ? TRUE : FALSE);

      Button_SetState(controller_controls.view_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK ? TRUE : FALSE);
      Button_SetState(controller_controls.menu_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_START ? TRUE : FALSE);
      Button_SetState(controller_controls.left_bumper, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ? TRUE : FALSE);
      Button_SetState(controller_controls.right_bumper, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ? TRUE : FALSE);

      Button_SetState(controller_controls.dpad_up_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP ? TRUE : FALSE);
      Button_SetState(controller_controls.dpad_down_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ? TRUE : FALSE);
      Button_SetState(controller_controls.dpad_left_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT ? TRUE : FALSE);
      Button_SetState(controller_controls.dpad_right_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT ? TRUE : FALSE);

      Button_SetState(controller_controls.left_stick_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ? TRUE : FALSE);
      Button_SetState(controller_controls.right_stick_button, new_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ? TRUE : FALSE);

      ::SendMessageW(controller_controls.left_trigger, TBM_SETPOS, FALSE, new_state.Gamepad.bLeftTrigger);
      ::SendMessageW(controller_controls.left_trigger, TBM_SETSEL, TRUE, MAKELPARAM(0, new_state.Gamepad.bLeftTrigger));
      ::SendMessageW(controller_controls.right_trigger, TBM_SETPOS, FALSE, new_state.Gamepad.bRightTrigger);
      ::SendMessageW(controller_controls.right_trigger, TBM_SETSEL, TRUE, MAKELPARAM(0, new_state.Gamepad.bRightTrigger));

      auto convert = [](SHORT value) -> SHORT {
        if (value >= 0)
        {
          return value;
        }
        return -(value + 1);
      };

      if (new_state.Gamepad.sThumbLX >= 0)
      {
        ::SendMessageW(controller_controls.left_x_axis_right, TBM_SETPOS, FALSE, convert(new_state.Gamepad.sThumbLX));
        ::SendMessageW(controller_controls.left_x_axis_right, TBM_SETSEL, TRUE, MAKELPARAM(0, convert(new_state.Gamepad.sThumbLX)));
        ::SendMessageW(controller_controls.left_x_axis_left, TBM_SETPOS, FALSE, 0);
        ::SendMessageW(controller_controls.left_x_axis_left, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
      }
      else
      {

        ::SendMessageW(controller_controls.left_x_axis_right, TBM_SETPOS, FALSE, 0);
        ::SendMessageW(controller_controls.left_x_axis_right, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
        ::SendMessageW(controller_controls.left_x_axis_left, TBM_SETPOS, FALSE, 32767 - convert(new_state.Gamepad.sThumbLX));
        ::SendMessageW(controller_controls.left_x_axis_left, TBM_SETSEL, TRUE, MAKELPARAM(32767 - convert(new_state.Gamepad.sThumbLX), 32767));
      }

      if (new_state.Gamepad.sThumbLY >= 0)
      {
        ::SendMessageW(controller_controls.left_y_axis_up, TBM_SETPOS, FALSE, 32767 - convert(new_state.Gamepad.sThumbLY));
        ::SendMessageW(controller_controls.left_y_axis_up, TBM_SETSEL, TRUE, MAKELPARAM(32767 - convert(new_state.Gamepad.sThumbLY), 32767));
        ::SendMessageW(controller_controls.left_y_axis_down, TBM_SETPOS, FALSE, 0);
        ::SendMessageW(controller_controls.left_y_axis_down, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
      }
      else
      {
        ::SendMessageW(controller_controls.left_y_axis_up, TBM_SETPOS, FALSE, 0);
        ::SendMessageW(controller_controls.left_y_axis_up, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
        ::SendMessageW(controller_controls.left_y_axis_down, TBM_SETPOS, FALSE, convert(new_state.Gamepad.sThumbLY));
        ::SendMessageW(controller_controls.left_y_axis_down, TBM_SETSEL, TRUE, MAKELPARAM(0, convert(new_state.Gamepad.sThumbLY)));
      }

      if (new_state.Gamepad.sThumbRX >= 0)
      {
        ::SendMessageW(controller_controls.right_x_axis_right, TBM_SETPOS, FALSE, convert(new_state.Gamepad.sThumbRX));
        ::SendMessageW(controller_controls.right_x_axis_right, TBM_SETSEL, TRUE, MAKELPARAM(0, convert(new_state.Gamepad.sThumbRX)));
        ::SendMessageW(controller_controls.right_x_axis_left, TBM_SETPOS, FALSE, 0);
        ::SendMessageW(controller_controls.right_x_axis_left, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
      }
      else
      {
        ::SendMessageW(controller_controls.right_x_axis_right, TBM_SETPOS, FALSE, MAKELPARAM(0, 0));
        ::SendMessageW(controller_controls.right_x_axis_right, TBM_SETSEL, TRUE, 0);
        ::SendMessageW(controller_controls.right_x_axis_left, TBM_SETPOS, FALSE, 32767 - convert(new_state.Gamepad.sThumbRX));
        ::SendMessageW(controller_controls.right_x_axis_left, TBM_SETSEL, TRUE, MAKELPARAM(32767 - convert(new_state.Gamepad.sThumbRX), 32767));
      }

      if (new_state.Gamepad.sThumbRY >= 0)
      {
        ::SendMessageW(controller_controls.right_y_axis_up, TBM_SETPOS, FALSE, 32767 - convert(new_state.Gamepad.sThumbRY));
        ::SendMessageW(controller_controls.right_y_axis_up, TBM_SETSEL, TRUE, MAKELPARAM(32767 - convert(new_state.Gamepad.sThumbRY), 32767));
        ::SendMessageW(controller_controls.right_y_axis_down, TBM_SETPOS, FALSE, 0);
        ::SendMessageW(controller_controls.right_y_axis_down, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
      }
      else
      {
        ::SendMessageW(controller_controls.right_y_axis_up, TBM_SETPOS, FALSE, 0);
        ::SendMessageW(controller_controls.right_y_axis_up, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));

        ::SendMessageW(controller_controls.right_y_axis_down, TBM_SETPOS, FALSE, convert(new_state.Gamepad.sThumbRY));
        ::SendMessageW(controller_controls.right_y_axis_down, TBM_SETSEL, TRUE, MAKELPARAM(0, convert(new_state.Gamepad.sThumbRY)));
      }

      return std::nullopt;
    }


    win32::lresult_t nm_custom_draw(NMCUSTOMDRAW& info)
    {
      if (auto bars = controller_controls.track_bars(); info.dwDrawStage == CDDS_PREPAINT && std::any_of(bars.begin(), bars.end(), [&](auto& bar) {
            return bar == info.hdr.hwndFrom;
          }))
      {
        auto control = win32::window_ref(info.hdr.hwndFrom);

        auto rect = control.GetClientRect();

        if (!rect)
        {
          return CDRF_DODEFAULT;
        }

        auto stock_brush = GetStockBrush(DC_BRUSH);

        ::SetDCBrushColor(info.hdc, RGB(0, 0, 0));
        ::FillRect(info.hdc, &*rect, stock_brush);

        auto style = control.GetWindowStyle();

        if (style & TBS_VERT)
        {
          auto pos = ::SendMessageW(control, TBM_GETPOS, 0, 0);
          auto max = ::SendMessageW(control, TBM_GETRANGEMAX, 0, 0);

          auto max_height = rect->bottom - rect->top;

          auto interval = (double)max_height / max;
          auto actual_height = pos * interval;

          if (style & TBS_REVERSED)
          {
            rect->top = (long)actual_height;
            if (::SendMessageW(control, TBM_GETSELEND, 0, 0) == 0)
            {
              rect->bottom = 0;
            }
          }
          else
          {
            rect->bottom = ((long)actual_height) - rect->top;
          }

          ::SetDCBrushColor(info.hdc, RGB(255, 0, 0));
          ::FillRect(info.hdc, &*rect, stock_brush);
        }
        else
        {
          auto pos = ::SendMessageW(control, TBM_GETPOS, 0, 0);
          auto max = ::SendMessageW(control, TBM_GETRANGEMAX, 0, 0);

          auto max_width = rect->right - rect->left;

          auto interval = (double)max_width / max;
          auto actual_width = pos * interval;

          if (style & TBS_REVERSED)
          {
            rect->left = (long)actual_width;
            if (::SendMessageW(control, TBM_GETSELEND, 0, 0) == 0)
            {
              rect->right = 0;
            }
          }
          else
          {
            rect->right = ((long)actual_width) - rect->left;
          }
          ::SetDCBrushColor(info.hdc, RGB(255, 0, 0));
          ::FillRect(info.hdc, &*rect, stock_brush);
        }

        return CDRF_SKIPDEFAULT;
      }

      return CDRF_DODEFAULT;
    }

    void add_device(HANDLE handle)
    {
      if (registered_keyboards.contains(handle) || registered_mice.contains(handle) || registered_controllers.contains(handle))
      {
        return;
      }

      RID_DEVICE_INFO device_info{};
      UINT size = sizeof(device_info);

      if (::GetRawInputDeviceInfoW(handle, RIDI_DEVICEINFO, &device_info, &size) <= 0)
      {
        return;
      }

      if (device_info.dwType == RIM_TYPEKEYBOARD)
      {
        win32::list_view_item item(L"Keyboard X");
        item.iGroupId = 1;
        item.mask = item.mask | LVIF_GROUPID;
        item.sub_items.emplace_back(L"Keyboard");
        registered_keyboards.emplace(handle, device_selection.InsertRow(std::move(item)));
      }
      else if (device_info.dwType == RIM_TYPEMOUSE)
      {
        win32::list_view_item item(L"Mouse X");
        item.iGroupId = 2;
        item.mask = item.mask | LVIF_GROUPID;
        item.sub_items.emplace_back(L"Mouse");
        registered_mice.emplace(handle, device_selection.InsertRow(std::move(item)));
      }
      else
      {
        auto info = siege::views::controller_info_for_raw_input_device_handle(handle);

        if (!info)
        {
          return;
        }

        win32::list_view_item item(std::wstring(info->device_name));
        item.iGroupId = 3;
        item.mask = item.mask | LVIF_GROUPID;
        item.sub_items.emplace_back(L"Controller");
        registered_controllers.emplace(handle, controller_item{ device_selection.InsertRow(std::move(item)), std::move(*info) });
      }
    }

    void remove_device(HANDLE handle)
    {
      auto iter = registered_keyboards.find(handle);

      if (iter != registered_keyboards.end())
      {
        ListView_DeleteItem(device_selection, iter->second);
        registered_keyboards.erase(handle);
        return;
      }

      iter = registered_mice.find(handle);

      if (iter != registered_mice.end())
      {
        ListView_DeleteItem(device_selection, iter->second);
        registered_mice.erase(handle);
        return;
      }

      auto controller_iter = registered_controllers.find(handle);

      if (controller_iter == registered_controllers.end())
      {
        return;
      }

      ListView_DeleteItem(device_selection, controller_iter->second.item);
      registered_controllers.erase(handle);
    }


    std::optional<win32::lresult_t> wm_input_device_change(WPARAM type, HANDLE device)
    {
      if (type == GIDC_ARRIVAL)
      {
        add_device(device);
        return 0;
      }
      else if (type == GIDC_REMOVAL)
      {
        remove_device(device);
        return 0;
      }
      return std::nullopt;
    }

    void update_mappings_for_context(controller_info& info)
    {
      for (auto& mapping : mappings)
      {
        mapping.visible = false;
      }

      for (auto button : {
             VK_GAMEPAD_A,
             VK_GAMEPAD_B,
             VK_GAMEPAD_X,
             VK_GAMEPAD_Y,
             VK_GAMEPAD_LEFT_SHOULDER,
             VK_GAMEPAD_RIGHT_SHOULDER,
             VK_GAMEPAD_LEFT_TRIGGER,
             VK_GAMEPAD_RIGHT_TRIGGER,
             VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON,
             VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON,
             VK_GAMEPAD_VIEW,
             VK_GAMEPAD_MENU,
           })
      {
        auto index = info.get_hardware_index(button, controller_info::prefer_button);

        if (index.type != hardware_index::button)
        {
          continue;
        }
        button_mappings[index.index].visible = true;
        button_mappings[index.index].index = index;
        button_mappings[index.index].vkey = button;
      }

      for (auto axis : {
             VK_GAMEPAD_LEFT_THUMBSTICK_LEFT,
             VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT,
             VK_GAMEPAD_LEFT_THUMBSTICK_UP,
             VK_GAMEPAD_LEFT_THUMBSTICK_DOWN,
             VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT,
             VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT,
             VK_GAMEPAD_RIGHT_THUMBSTICK_UP,
             VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN,
             VK_GAMEPAD_LEFT_TRIGGER,
             VK_GAMEPAD_RIGHT_TRIGGER,
           })
      {
        auto index = info.get_hardware_index(axis, controller_info::prefer_value);

        if (index.type != hardware_index::value)
        {
          continue;
        }
        axis_mappings[index.index].visible = true;
        axis_mappings[index.index].index = index;
        axis_mappings[index.index].vkey = axis;
      }

      std::wstring temp;

      LVITEMW invisible_item{
        .mask = LVIF_GROUPID,
        .iGroupId = 3
      };
      LVITEMW button_item{
        .mask = LVIF_GROUPID,
        .iGroupId = 1
      };

      LVITEMW axis_item{
        .mask = LVIF_GROUPID,
        .iGroupId = 2
      };

      for (auto& mapping : button_mappings)
      {
        if (mapping.visible)
        {
          auto name = string_for_vkey(mapping.vkey, info.detected_context);
          temp = L"Button " + std::to_wstring(mapping.index.index + 1);
          ListView_SetItemText(custom_mapping, mapping.item, 0, temp.data());
          ListView_SetItemText(custom_mapping, mapping.item, 1, name.data());
          button_item.iItem = mapping.item;
          ListView_SetItem(custom_mapping, &button_item);
        }
        else
        {
          temp = L"";
          ListView_SetItemText(custom_mapping, mapping.item, 0, temp.data());
          ListView_SetItemText(custom_mapping, mapping.item, 1, temp.data());
          invisible_item.iItem = mapping.item;
          ListView_SetItem(custom_mapping, &invisible_item);
        }
      }

      for (auto& mapping : axis_mappings)
      {
        if (mapping.visible)
        {
          auto name = string_for_vkey(mapping.vkey, info.detected_context);
          temp = L"Axis " + std::to_wstring(mapping.index.index + 1);
          ListView_SetItemText(custom_mapping, mapping.item, 0, temp.data());
          ListView_SetItemText(custom_mapping, mapping.item, 1, name.data());
          button_item.iItem = mapping.item;
          ListView_SetItem(custom_mapping, &axis_item);
        }
        else
        {
          temp = L"";
          ListView_SetItemText(custom_mapping, mapping.item, 0, temp.data());
          ListView_SetItemText(custom_mapping, mapping.item, 1, temp.data());
          invisible_item.iItem = mapping.item;
          ListView_SetItem(custom_mapping, &invisible_item);
        }
      }
    }

    std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override
    {
      switch (message)
      {
      case WM_CREATE: {
        return wm_create();
      }
      case WM_DESTROY: {
        return wm_destroy();
      }
      case WM_SIZE: {
        return wm_size((std::size_t)wparam, SIZE(LOWORD(lparam), HIWORD(lparam)));
      }
      case WM_NOTIFY: {
        NMHDR* hdr = (NMHDR*)lparam;
        if (hdr && hdr->code == NM_CUSTOMDRAW)
        {
          return nm_custom_draw(*(NMCUSTOMDRAW*)lparam);
        }
        return std::nullopt;
      }
      case WM_CLOSE:
        return wm_close();
      case WM_INPUT: {
        return wm_input((HRAWINPUT)lparam);
      }
      case WM_INPUT_DEVICE_CHANGE: {
        return wm_input_device_change(wparam, (HANDLE)lparam);
      }
      default:
        return std::nullopt;
      }
    }
  };

  void show_connected_devices_dialog(win32::window_ref parent)
  {
    win32::DialogBoxIndirectParamW<connected_devices_dialog>(::GetModuleHandleW(nullptr),
      win32::default_dialog{ { .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_SYSMENU, .cx = 640, .cy = 480 } },
      std::move(parent));
  }
}// namespace siege::views