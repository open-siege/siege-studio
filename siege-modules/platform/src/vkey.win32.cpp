module;
#include <siege/platform/windows.hpp>

export module siege.platform;

import std;

export namespace siege::platform
{
  enum struct vkey : std::uint16_t
  {
    lbutton = VK_LBUTTON,
    rbutton = VK_RBUTTON,
    mbutton = VK_MBUTTON,
    xbutton1 = VK_XBUTTON1,
    xbutton2 = VK_XBUTTON2,

    backspace = VK_BACK,
    tab = VK_TAB,
    enter = VK_RETURN,
    shift = VK_SHIFT,
    control = VK_CONTROL,
    alt = VK_MENU,
    pause = VK_PAUSE,
    caps_lock = VK_CAPITAL,
    space = VK_SPACE,
    page_up = VK_PRIOR,
    page_down = VK_NEXT,
    end = VK_END,
    home = VK_HOME,
    left = VK_LEFT,
    up = VK_UP,
    right = VK_RIGHT,
    down = VK_DOWN,
    insert = VK_INSERT,
    del = VK_DELETE,

    f1 = VK_F1,
    f2 = VK_F2,
    f3 = VK_F3,
    f4 = VK_F4,
    f5 = VK_F5,
    f6 = VK_F6,
    f7 = VK_F7,
    f8 = VK_F8,
    f9 = VK_F9,
    f10 = VK_F10,
    f11 = VK_F11,
    f12 = VK_F12,

    num_lock = VK_NUMLOCK,
    numpad_2 = VK_NUMPAD2,

    left_shift = VK_LSHIFT,
    right_shift = VK_RSHIFT,
    left_control = VK_LCONTROL,
    right_control = VK_RCONTROL,
    left_alt = VK_LMENU,
    right_alt = VK_RMENU,

    oem_1 = VK_OEM_1,
    oem_plus = VK_OEM_PLUS,
    oem_comma = VK_OEM_COMMA,
    oem_minus = VK_OEM_MINUS,
    oem_period = VK_OEM_PERIOD,
    oem_2 = VK_OEM_2,
    oem_3 = VK_OEM_3,
    oem_4 = VK_OEM_4,
    oem_5 = VK_OEM_5,
    oem_6 = VK_OEM_6,

    gamepad_a = VK_GAMEPAD_A,
    gamepad_b = VK_GAMEPAD_B,
    gamepad_x = VK_GAMEPAD_X,
    gamepad_y = VK_GAMEPAD_Y,
    gamepad_right_shoulder = VK_GAMEPAD_RIGHT_SHOULDER,
    gamepad_left_shoulder = VK_GAMEPAD_LEFT_SHOULDER,
    gamepad_left_trigger = VK_GAMEPAD_LEFT_TRIGGER,
    gamepad_right_trigger = VK_GAMEPAD_RIGHT_TRIGGER,
    gamepad_dpad_up = VK_GAMEPAD_DPAD_UP,
    gamepad_dpad_down = VK_GAMEPAD_DPAD_DOWN,
    gamepad_dpad_left = VK_GAMEPAD_DPAD_LEFT,
    gamepad_dpad_right = VK_GAMEPAD_DPAD_RIGHT,
    gamepad_menu = VK_GAMEPAD_MENU,
    gamepad_view = VK_GAMEPAD_VIEW,
    gamepad_left_thumbstick_button = VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON,
    gamepad_right_thumbstick_button = VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON,
    gamepad_left_thumbstick_up = VK_GAMEPAD_LEFT_THUMBSTICK_UP,
    gamepad_left_thumbstick_down = VK_GAMEPAD_LEFT_THUMBSTICK_DOWN,
    gamepad_left_thumbstick_right = VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT,
    gamepad_left_thumbstick_left = VK_GAMEPAD_LEFT_THUMBSTICK_LEFT,
    gamepad_right_thumbstick_up = VK_GAMEPAD_RIGHT_THUMBSTICK_UP,
    gamepad_right_thumbstick_down = VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN,
    gamepad_right_thumbstick_right = VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT,
    gamepad_right_thumbstick_left = VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT,
  };
}
