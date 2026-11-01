export module siege.platform;

import std;

export namespace siege::platform
{
  // TODO: map these to real platform constants where they exist (e.g. Linux/evdev, macOS).
  // Values are placeholders until non-Windows backends need stable numeric IDs.
  // Gamepad enumerators follow Win32 VK_GAMEPAD_* order so range checks stay valid later.
  enum struct vkey : std::uint16_t
  {
    lbutton,
    rbutton,
    mbutton,
    xbutton1,
    xbutton2,

    backspace,
    tab,
    enter,
    shift,
    control,
    alt,
    pause,
    caps_lock,
    space,
    page_up,
    page_down,
    end,
    home,
    left,
    up,
    right,
    down,
    insert,
    del,

    f1,
    f2,
    f3,
    f4,
    f5,
    f6,
    f7,
    f8,
    f9,
    f10,
    f11,
    f12,

    num_lock,
    numpad_2,

    left_shift,
    right_shift,
    left_control,
    right_control,
    left_alt,
    right_alt,

    oem_1,
    oem_plus,
    oem_comma,
    oem_minus,
    oem_period,
    oem_2,
    oem_3,
    oem_4,
    oem_5,
    oem_6,

    gamepad_a,
    gamepad_b,
    gamepad_x,
    gamepad_y,
    gamepad_right_shoulder,
    gamepad_left_shoulder,
    gamepad_left_trigger,
    gamepad_right_trigger,
    gamepad_dpad_up,
    gamepad_dpad_down,
    gamepad_dpad_left,
    gamepad_dpad_right,
    gamepad_menu,
    gamepad_view,
    gamepad_left_thumbstick_button,
    gamepad_right_thumbstick_button,
    gamepad_left_thumbstick_up,
    gamepad_left_thumbstick_down,
    gamepad_left_thumbstick_right,
    gamepad_left_thumbstick_left,
    gamepad_right_thumbstick_up,
    gamepad_right_thumbstick_down,
    gamepad_right_thumbstick_right,
    gamepad_right_thumbstick_left,
  };
}
