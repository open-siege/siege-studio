#include <vector>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <hidusage.h>
#include <xinput.h>
#include <joystickapi.h>
#include <hidsdi.h>
#include "exe_shared.hpp"

namespace siege::views
{
  constexpr USAGE to_index(USAGE value)
  {
    return value - HID_USAGE_GENERIC_X;
  }

  std::optional<hardware_index> hardware_index_for_xbox_vkey(SHORT vkey, controller_info::button_preference)
  {
    switch (vkey)
    {
    case VK_GAMEPAD_DPAD_UP:
    case VK_GAMEPAD_DPAD_DOWN:
    case VK_GAMEPAD_DPAD_LEFT:
    case VK_GAMEPAD_DPAD_RIGHT:
      return hardware_index{ input_type::hat, 0 };
    case VK_GAMEPAD_A:
      return hardware_index{ input_type::button, 0 };
    case VK_GAMEPAD_B:
      return hardware_index{ input_type::button, 1 };
    case VK_GAMEPAD_X:
      return hardware_index{ input_type::button, 2 };
    case VK_GAMEPAD_Y:
      return hardware_index{ input_type::button, 3 };
    case VK_GAMEPAD_LEFT_SHOULDER:
      return hardware_index{ input_type::button, 4 };
    case VK_GAMEPAD_RIGHT_SHOULDER:
      return hardware_index{ input_type::button, 5 };
    case VK_GAMEPAD_VIEW:
      return hardware_index{ input_type::button, 6 };
    case VK_GAMEPAD_MENU:
      return hardware_index{ input_type::button, 7 };
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
      return hardware_index{ input_type::button, 8 };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
      return hardware_index{ input_type::button, 9 };
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_X) };
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_Y) };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_RX) };
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_RY) };
    case VK_GAMEPAD_LEFT_TRIGGER:
    case VK_GAMEPAD_RIGHT_TRIGGER:
      return hardware_index{ input_type::axis, to_index(HID_USAGE_GENERIC_Z) };
    default:
      return std::nullopt;
    }
  }
}// namespace siege::views