export module siege.platform;

import std;

export namespace siege::platform
{
  // TODO make this more accurate when more platforms are supported.
  enum struct vkey : std::uint16_t
  {
    up,
    down,
    left,
    right,

    gamepad_a,
    gamepad_b,
    gamepad_x,
    gamepad_y,
    gamepad_left_shoulder,
    gamepad_right_shoulder,
    gamepad_left_trigger,
    gamepad_right_trigger,
    gamepad_dpad_down,
    gamepad_dpad_left,
    gamepad_dpad_right,
    gamepad_menu,
    gamepad_view,
    gamepad_left_thumbstick_button,
    gamepad_right_thumbstick_button,
    gamepad_left_thumbstick_up,
    gamepad_left_thumbstick_down,
    gamepad_left_thumbstick_left,
    gamepad_left_thumbstick_right,
    gamepad_right_thumbstick_up,
    gamepad_right_thumbstick_down,
    gamepad_right_thumbstick_left,
    gamepad_right_thumbstick_right,
  };
}
