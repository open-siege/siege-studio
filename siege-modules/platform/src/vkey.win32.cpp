export module siege.platform;

#include <siege/platform/windows.hpp>

import std;

export namespace siege::platform
{
  enum struct vkey : std::uint16_t
  {
    up = VK_UP,
    down = VK_DOWN,
    left = VK_LEFT,
    right = VK_RIGHT,

    gamepad_a = VK_GAMEPAD_A,
    gamepad_b = VK_GAMEPAD_B,
    gamepad_x = VK_GAMEPAD_X,
    gamepad_y = VK_GAMEPAD_Y,
    gamepad_left_shoulder = VK_GAMEPAD_LEFT_SHOULDER,
    gamepad_right_shoulder = VK_GAMEPAD_RIGHT_SHOULDER,
    gamepad_left_trigger = VK_GAMEPAD_LEFT_TRIGGER,
    gamepad_right_trigger = VK_GAMEPAD_RIGHT_TRIGGER,
    gamepad_dpad_down = VK_GAMEPAD_DPAD_DOWN,
    gamepad_dpad_left = VK_GAMEPAD_DPAD_LEFT,
    gamepad_dpad_right = VK_GAMEPAD_DPAD_RIGHT,
    gamepad_menu = VK_GAMEPAD_MENU,
    gamepad_view = VK_GAMEPAD_VIEW,
    gamepad_left_thumbstick_button = VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON,
    gamepad_right_thumbstick_button = VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON,
    gamepad_left_thumbstick_up = VK_GAMEPAD_LEFT_THUMBSTICK_UP,
    gamepad_left_thumbstick_down = VK_GAMEPAD_LEFT_THUMBSTICK_DOWN,
    gamepad_left_thumbstick_left = VK_GAMEPAD_LEFT_THUMBSTICK_LEFT,
    gamepad_left_thumbstick_right = VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT,
    gamepad_right_thumbstick_up = VK_GAMEPAD_RIGHT_THUMBSTICK_UP,
    gamepad_right_thumbstick_down = VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN,
    gamepad_right_thumbstick_left = VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT,
    gamepad_right_thumbstick_right = VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT,
  };
}
