module;
#include <siege/platform/windows.hpp>
#include <hidusage.h>

module exe.shared;

import std;
import win32.dialog;
import win32.capabilities;

namespace siege::views
{
  using vkey = siege::platform::vkey;
  constexpr USAGE to_index(USAGE value)
  {
    return value - HID_USAGE_GENERIC_X;
  }

  std::optional<hardware_index> hardware_index_for_xbox_vkey(vkey key, controller_info::button_preference)
  {
    switch (key)
    {
    case vkey::gamepad_dpad_up:
    case vkey::gamepad_dpad_down:
    case vkey::gamepad_dpad_left:
    case vkey::gamepad_dpad_right:
      return hardware_index{ input_type::hat, 0 };
    case vkey::gamepad_a:
      return hardware_index{ input_type::button, 0 };
    case vkey::gamepad_b:
      return hardware_index{ input_type::button, 1 };
    case vkey::gamepad_x:
      return hardware_index{ input_type::button, 2 };
    case vkey::gamepad_y:
      return hardware_index{ input_type::button, 3 };
    case vkey::gamepad_left_shoulder:
      return hardware_index{ input_type::button, 4 };
    case vkey::gamepad_right_shoulder:
      return hardware_index{ input_type::button, 5 };
    case vkey::gamepad_view:
      return hardware_index{ input_type::button, 6 };
    case vkey::gamepad_menu:
      return hardware_index{ input_type::button, 7 };
    case vkey::gamepad_left_thumbstick_button:
      return hardware_index{ input_type::button, 8 };
    case vkey::gamepad_right_thumbstick_button:
      return hardware_index{ input_type::button, 9 };
    case vkey::gamepad_left_thumbstick_left:
    case vkey::gamepad_left_thumbstick_right:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_X) };
    case vkey::gamepad_left_thumbstick_up:
    case vkey::gamepad_left_thumbstick_down:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_Y) };
    case vkey::gamepad_right_thumbstick_left:
    case vkey::gamepad_right_thumbstick_right:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_RX) };
    case vkey::gamepad_right_thumbstick_up:
    case vkey::gamepad_right_thumbstick_down:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_RY) };
    case vkey::gamepad_left_trigger:
    case vkey::gamepad_right_trigger:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_Z) };
    default:
      return std::nullopt;
    }
  }
}// namespace siege::views
