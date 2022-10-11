#include <SDL.h>

SDL_JoystickType Siege_JoystickGetType(SDL_Joystick *joystick)
{
  return SDL_JoystickGetType(joystick);
}

void Siege_InitVirtualJoysticksFromMice()
{
}

SDL_bool Siege_IsMouse(SDL_Joystick*)
{
  return SDL_bool::SDL_FALSE;
}

Uint16 Siege_JoystickGetVendor(SDL_Joystick *joystick)
{
  return SDL_JoystickGetVendor(joystick);
}

Uint16 Siege_JoystickGetProduct(SDL_Joystick *joystick)
{
  return SDL_JoystickGetProduct(joystick);
}

Uint16 Siege_JoystickGetProductVersion(SDL_Joystick *joystick)
{
  return SDL_JoystickGetProductVersion(joystick);
}
