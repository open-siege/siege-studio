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
  using input_type = siege::platform::controller_input_type;

  constexpr USAGE to_index(USAGE value)
  {
    return value - HID_USAGE_GENERIC_X;
  }

  std::optional<hardware_index> hardware_index_for_ps3_vkey(vkey key, controller_info::button_preference)
  {
    switch (key)
    {
    case vkey::gamepad_dpad_up:
    case vkey::gamepad_dpad_down:
    case vkey::gamepad_dpad_left:
    case vkey::gamepad_dpad_right:
      return hardware_index{ input_type::hat, 0 };
    case vkey::gamepad_x:// ps3/ps4 square
      return hardware_index{ input_type::button, 0 };
    case vkey::gamepad_a:// ps3/ps4 cross
      return hardware_index{ input_type::button, 1 };
    case vkey::gamepad_b:// ps3/ps4 circle
      return hardware_index{ input_type::button, 2 };
    case vkey::gamepad_y:// ps3/ps4 triangle
      return hardware_index{ input_type::button, 3 };
    case vkey::gamepad_left_shoulder:
      return hardware_index{ input_type::button, 4 };
    case vkey::gamepad_right_shoulder:
      return hardware_index{ input_type::button, 5 };
    case vkey::gamepad_left_trigger:
      return hardware_index{ input_type::button, 6 };
    case vkey::gamepad_right_trigger:
      return hardware_index{ input_type::button, 7 };
    case vkey::gamepad_view:
      return hardware_index{ input_type::button, 8 };
    case vkey::gamepad_menu:
      return hardware_index{ input_type::button, 9 };
    case vkey::gamepad_left_thumbstick_button:
      return hardware_index{ input_type::button, 10 };
    case vkey::gamepad_right_thumbstick_button:
      return hardware_index{ input_type::button, 11 };
    case vkey::gamepad_left_thumbstick_left:
    case vkey::gamepad_left_thumbstick_right:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_X) };
    case vkey::gamepad_left_thumbstick_up:
    case vkey::gamepad_left_thumbstick_down:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_Y) };
    case vkey::gamepad_right_thumbstick_left:
    case vkey::gamepad_right_thumbstick_right:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_Z) };
    case vkey::gamepad_right_thumbstick_up:
    case vkey::gamepad_right_thumbstick_down:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_RZ) };
    default:
      return std::nullopt;
    }
  }

  std::optional<hardware_index> hardware_index_for_ps4_vkey(vkey key, controller_info::button_preference prefer_button)
  {
    switch (key)
    {
    case vkey::gamepad_dpad_up:
    case vkey::gamepad_dpad_down:
    case vkey::gamepad_dpad_left:
    case vkey::gamepad_dpad_right:
      return hardware_index{ input_type::hat, 0 };
    case vkey::gamepad_x:// ps3/ps4 square
      return hardware_index{ input_type::button, 0 };
    case vkey::gamepad_a:// ps3/ps4 cross
      return hardware_index{ input_type::button, 1 };
    case vkey::gamepad_b:// ps3/ps4 circle
      return hardware_index{ input_type::button, 2 };
    case vkey::gamepad_y:// ps3/ps4 triangle
      return hardware_index{ input_type::button, 3 };
    case vkey::gamepad_left_shoulder:
      return hardware_index{ input_type::button, 4 };
    case vkey::gamepad_right_shoulder:
      return hardware_index{ input_type::button, 5 };
    case vkey::gamepad_view:
      return hardware_index{ input_type::button, 8 };
    case vkey::gamepad_menu:
      return hardware_index{ input_type::button, 9 };
    case vkey::gamepad_left_thumbstick_button:
      return hardware_index{ input_type::button, 10 };
    case vkey::gamepad_right_thumbstick_button:
      return hardware_index{ input_type::button, 11 };
    case vkey::gamepad_left_thumbstick_left:
    case vkey::gamepad_left_thumbstick_right:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_X) };
    case vkey::gamepad_left_thumbstick_up:
    case vkey::gamepad_left_thumbstick_down:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_Y) };
    case vkey::gamepad_left_trigger: {
      if (prefer_button)
      {
        return hardware_index{ input_type::button, 6 };
      }

      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_RX) };
    }
    case vkey::gamepad_right_trigger: {

      if (prefer_button)
      {
        return hardware_index{ input_type::button, 7 };
      }

      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_RY) };
    }
    case vkey::gamepad_right_thumbstick_left:
    case vkey::gamepad_right_thumbstick_right:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_Z) };
    case vkey::gamepad_right_thumbstick_up:
    case vkey::gamepad_right_thumbstick_down:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_RZ) };
    default:
      return std::nullopt;
    }
  }
}// namespace siege::views
