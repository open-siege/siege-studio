#ifndef OPEN_SIEGE_PLATFORM_HPP
#define OPEN_SIEGE_PLATFORM_HPP

#include <SDL.h>

SDL_JoystickType Siege_JoystickGetType(SDL_Joystick* joystick);

SDL_bool Siege_IsMouse(SDL_Joystick* joystick);

Uint16 Siege_JoystickGetVendor(SDL_Joystick *joystick);
Uint16 Siege_JoystickGetProduct(SDL_Joystick *joystick);
Uint16 Siege_JoystickGetProductVersion(SDL_Joystick *joystick);


void Siege_InitVirtualJoysticks();

#endif// OPEN_SIEGE_PLATFORM_HPP
