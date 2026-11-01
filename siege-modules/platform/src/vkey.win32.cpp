export module siege.platform;

#include <siege/platform/windows.hpp>

import std;

export namespace siege::platform
{
  enum struct vkey : std::uint16_t
  {
    gamepad_right_trigger = VK_GAMEPAD_RIGHT_TRIGGER,
  };
}
