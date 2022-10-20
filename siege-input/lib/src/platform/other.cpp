#include <SDL.h>

namespace siege
{
  SDL_JoystickType JoystickGetType(SDL_Joystick *joystick)
  {
    return SDL_JoystickGetType(joystick);
  }

  void InitVirtualJoysticksFromKeyboardsAndMice()
  {
  }

  SDL_bool IsMouse(SDL_Joystick*)
  {
    return SDL_bool::SDL_FALSE;
  }

  SDL_bool IsKeyboard(SDL_Joystick*)
  {
    return SDL_bool::SDL_FALSE;
  }

  Uint16 JoystickGetVendor(SDL_Joystick *joystick)
  {
    return SDL_JoystickGetVendor(joystick);
  }

  Uint16 JoystickGetProduct(SDL_Joystick *joystick)
  {
    return SDL_JoystickGetProduct(joystick);
  }

  Uint16 JoystickGetProductVersion(SDL_Joystick *joystick)
  {
    return SDL_JoystickGetProductVersion(joystick);
  }
}

