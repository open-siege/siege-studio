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

  struct connected_devices_dialog : win32::basic_window<connected_devices_dialog>
  {
    struct controller_item
    {
      win32::wparam_t item;
      controller_state state;
    };

    win32::list_view device_selection;
    std::map<HANDLE, win32::wparam_t> registered_keyboards;
    std::map<HANDLE, win32::wparam_t> registered_mice;
    std::map<HANDLE, controller_item> registered_controllers;

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
    } controller_controls;

    connected_devices_dialog(win32::hwnd_t self, CREATESTRUCTW& params) : basic_window(self, params)
    {
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

      // TODO create list view here,
      // at least 3 groups - keyboards, mice, controllers
      // TODO create controller input table,
      // TODO create controller settings section
      // register for raw input here
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

      // TODO reduce the duplication somehow
      auto remaining = SIZE{ .cx = (client_size.cx - one_quarter.cx) / 4, .cy = client_size.cy };
      auto button_size = SIZE{ .cx = remaining.cx / 4, .cy = remaining.cx / 4 };

      auto slider_x_size = button_size;
      auto slider_y_size = button_size;

      auto starting = POINT{ .x = one_quarter.cx };

      controller_controls.left_trigger.SetWindowStyle(controller_controls.left_trigger.GetWindowStyle() | WS_VISIBLE);
      controller_controls.left_trigger.SetWindowPos(button_size);
      controller_controls.left_trigger.SetWindowPos(POINT{ .x = starting.x + button_size.cx });

      controller_controls.right_trigger.SetWindowStyle(controller_controls.right_trigger.GetWindowStyle() | WS_VISIBLE);
      controller_controls.right_trigger.SetWindowPos(button_size);
      controller_controls.right_trigger.SetWindowPos(POINT{ .x = starting.x + button_size.cx * 13 });

      controller_controls.left_bumper.SetWindowStyle(controller_controls.left_bumper.GetWindowStyle() | WS_VISIBLE);
      controller_controls.left_bumper.SetWindowPos(button_size);
      controller_controls.left_bumper.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = button_size.cy });

      controller_controls.view_button.SetWindowStyle(controller_controls.left_bumper.GetWindowStyle() | WS_VISIBLE);
      controller_controls.view_button.SetWindowPos(button_size);
      controller_controls.view_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx * 5, .y = button_size.cy });

      controller_controls.menu_button.SetWindowStyle(controller_controls.left_bumper.GetWindowStyle() | WS_VISIBLE);
      controller_controls.menu_button.SetWindowPos(button_size);
      controller_controls.menu_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx * 9, .y = button_size.cy });

      controller_controls.right_bumper.SetWindowStyle(controller_controls.right_bumper.GetWindowStyle() | WS_VISIBLE);
      controller_controls.right_bumper.SetWindowPos(button_size);
      controller_controls.right_bumper.SetWindowPos(POINT{ .x = starting.x + button_size.cx * 13, .y = button_size.cy });

      starting = POINT{ .x = one_quarter.cx, .y = button_size.cy * 3 };

      controller_controls.dpad_up_button.SetWindowStyle(controller_controls.dpad_up_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.dpad_up_button.SetWindowPos(button_size);
      controller_controls.dpad_up_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y });

      controller_controls.dpad_left_button.SetWindowStyle(controller_controls.dpad_left_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.dpad_left_button.SetWindowPos(button_size);
      controller_controls.dpad_left_button.SetWindowPos(POINT{ .x = starting.x, .y = starting.y + button_size.cy });

      controller_controls.dpad_down_button.SetWindowStyle(controller_controls.dpad_down_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.dpad_down_button.SetWindowPos(button_size);
      controller_controls.dpad_down_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy + button_size.cy });

      controller_controls.dpad_right_button.SetWindowStyle(controller_controls.dpad_right_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.dpad_right_button.SetWindowPos(button_size);
      controller_controls.dpad_right_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx + button_size.cx, .y = starting.y + button_size.cy });

      starting = POINT{ .x = starting.x + button_size.cx * 4, .y = starting.y };

      controller_controls.left_x_axis_left.SetWindowStyle(controller_controls.left_x_axis_left.GetWindowStyle() | WS_VISIBLE);
      controller_controls.left_x_axis_right.SetWindowStyle(controller_controls.left_x_axis_right.GetWindowStyle() | WS_VISIBLE);
      controller_controls.left_y_axis_up.SetWindowStyle(controller_controls.left_y_axis_up.GetWindowStyle() | WS_VISIBLE);
      controller_controls.left_y_axis_down.SetWindowStyle(controller_controls.left_y_axis_down.GetWindowStyle() | WS_VISIBLE);
      controller_controls.left_stick_button.SetWindowStyle(controller_controls.left_stick_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.left_x_axis_left.SetWindowPos(slider_x_size);
      controller_controls.left_x_axis_right.SetWindowPos(slider_x_size);
      controller_controls.left_y_axis_up.SetWindowPos(slider_y_size);
      controller_controls.left_y_axis_down.SetWindowPos(slider_y_size);
      controller_controls.left_stick_button.SetWindowPos(button_size);

      controller_controls.left_y_axis_up.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y });
      controller_controls.left_y_axis_down.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy + button_size.cy });
      controller_controls.left_stick_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy });
      controller_controls.left_x_axis_left.SetWindowPos(POINT{ .x = starting.x, .y = starting.y + button_size.cy });
      controller_controls.left_x_axis_right.SetWindowPos(POINT{ .x = starting.x + button_size.cx + button_size.cx, .y = starting.y + button_size.cy });

      starting = POINT{ .x = starting.x + button_size.cx * 4, .y = starting.y };

      controller_controls.right_x_axis_left.SetWindowStyle(controller_controls.right_x_axis_left.GetWindowStyle() | WS_VISIBLE);
      controller_controls.right_x_axis_right.SetWindowStyle(controller_controls.right_x_axis_right.GetWindowStyle() | WS_VISIBLE);
      controller_controls.right_y_axis_up.SetWindowStyle(controller_controls.right_y_axis_up.GetWindowStyle() | WS_VISIBLE);
      controller_controls.right_y_axis_down.SetWindowStyle(controller_controls.right_y_axis_down.GetWindowStyle() | WS_VISIBLE);
      controller_controls.right_stick_button.SetWindowStyle(controller_controls.right_stick_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.right_x_axis_left.SetWindowPos(slider_x_size);
      controller_controls.right_x_axis_right.SetWindowPos(slider_x_size);
      controller_controls.right_y_axis_up.SetWindowPos(slider_y_size);
      controller_controls.right_y_axis_down.SetWindowPos(slider_y_size);
      controller_controls.right_stick_button.SetWindowPos(button_size);

      controller_controls.right_y_axis_up.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y });
      controller_controls.right_y_axis_down.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy + button_size.cy });
      controller_controls.right_stick_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy });
      controller_controls.right_x_axis_left.SetWindowPos(POINT{ .x = starting.x, .y = starting.y + button_size.cy });
      controller_controls.right_x_axis_right.SetWindowPos(POINT{ .x = starting.x + button_size.cx + button_size.cx, .y = starting.y + button_size.cy });

      starting = POINT{ .x = starting.x + button_size.cx * 4, .y = starting.y };

      controller_controls.x_button.SetWindowStyle(controller_controls.a_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.x_button.SetWindowPos(button_size);
      controller_controls.x_button.SetWindowPos(POINT{ .x = starting.x, .y = starting.y + button_size.cy });

      controller_controls.y_button.SetWindowStyle(controller_controls.b_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.y_button.SetWindowPos(button_size);
      controller_controls.y_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y });

      controller_controls.a_button.SetWindowStyle(controller_controls.x_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.a_button.SetWindowPos(button_size);
      controller_controls.a_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx, .y = starting.y + button_size.cy + button_size.cy });

      controller_controls.b_button.SetWindowStyle(controller_controls.y_button.GetWindowStyle() | WS_VISIBLE);
      controller_controls.b_button.SetWindowPos(button_size);
      controller_controls.b_button.SetWindowPos(POINT{ .x = starting.x + button_size.cx + button_size.cx, .y = starting.y + button_size.cy });

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

      ::SendMessageW(controller_controls.left_trigger, TBM_SETSEL, TRUE, MAKELPARAM(0, new_state.Gamepad.bLeftTrigger));
      ::SendMessageW(controller_controls.left_trigger, TBM_SETPOS, TRUE, new_state.Gamepad.bLeftTrigger);
      ::SendMessageW(controller_controls.right_trigger, TBM_SETSEL, TRUE, MAKELPARAM(0, new_state.Gamepad.bRightTrigger));
      ::SendMessageW(controller_controls.right_trigger, TBM_SETPOS, TRUE, new_state.Gamepad.bLeftTrigger);

      auto convert = [](SHORT value) -> SHORT {
        if (value >= 0)
        {
          return value;
        }
        return -(value + 1);
      };

      if (new_state.Gamepad.sThumbLX >= 0)
      {
        ::SendMessageW(controller_controls.left_x_axis_right, TBM_SETSEL, TRUE, MAKELPARAM(0, convert(new_state.Gamepad.sThumbLX)));
        ::SendMessageW(controller_controls.left_x_axis_right, TBM_SETPOS, TRUE, convert(new_state.Gamepad.sThumbLX));
        ::SendMessageW(controller_controls.left_x_axis_left, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
      }
      else
      {
        ::SendMessageW(controller_controls.left_x_axis_right, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
        ::SendMessageW(controller_controls.left_x_axis_left, TBM_SETSEL, TRUE, MAKELPARAM(32767 - convert(new_state.Gamepad.sThumbLX), 32767));
        ::SendMessageW(controller_controls.left_x_axis_left, TBM_SETPOS, TRUE, 32767 - convert(new_state.Gamepad.sThumbLX));
      }

      if (new_state.Gamepad.sThumbLY >= 0)
      {
        ::SendMessageW(controller_controls.left_y_axis_up, TBM_SETSEL, TRUE, MAKELPARAM(32767 - convert(new_state.Gamepad.sThumbLY), 32767));
        ::SendMessageW(controller_controls.left_y_axis_down, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
      }
      else
      {
        ::SendMessageW(controller_controls.left_y_axis_up, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
        ::SendMessageW(controller_controls.left_y_axis_down, TBM_SETSEL, TRUE, MAKELPARAM(0, convert(new_state.Gamepad.sThumbLY)));
      }

      if (new_state.Gamepad.sThumbRX >= 0)
      {
        ::SendMessageW(controller_controls.right_x_axis_right, TBM_SETSEL, TRUE, MAKELPARAM(0, convert(new_state.Gamepad.sThumbRX)));
        ::SendMessageW(controller_controls.right_x_axis_right, TBM_SETPOS, TRUE, convert(new_state.Gamepad.sThumbRX));
        ::SendMessageW(controller_controls.right_x_axis_left, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
      }
      else
      {
        ::SendMessageW(controller_controls.right_x_axis_right, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
        ::SendMessageW(controller_controls.right_x_axis_left, TBM_SETSEL, TRUE, MAKELPARAM(32767 - convert(new_state.Gamepad.sThumbRX), 32767));
        ::SendMessageW(controller_controls.right_x_axis_left, TBM_SETPOS, TRUE, 32767 - convert(new_state.Gamepad.sThumbRX));
      }

      if (new_state.Gamepad.sThumbRY >= 0)
      {
        ::SendMessageW(controller_controls.right_y_axis_up, TBM_SETSEL, TRUE, MAKELPARAM(32767 - convert(new_state.Gamepad.sThumbRY), 32767));
        ::SendMessageW(controller_controls.right_y_axis_down, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
      }
      else
      {
        ::SendMessageW(controller_controls.right_y_axis_up, TBM_SETSEL, TRUE, MAKELPARAM(0, 0));
        ::SendMessageW(controller_controls.right_y_axis_down, TBM_SETSEL, TRUE, MAKELPARAM(0, convert(new_state.Gamepad.sThumbRY)));
      }


      return std::nullopt;
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
      win32::default_dialog{ { .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 640, .cy = 480 } },
      std::move(parent));
  }
}// namespace siege::views