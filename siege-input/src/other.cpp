#include <SDL.h>

SDL_JoystickType Siege_JoystickGetType(SDL_Joystick *joystick)
{
  return SDL_JoystickGetType(joystick);
}

void Siege_InitVirtualJoysticks()
{
}

SDL_bool Siege_IsMouse(SDL_Joystick*)
{
  return SDL_bool::SDL_FALSE;
}