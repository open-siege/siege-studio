#ifndef OPEN_SIEGE_PLATFORM_HPP
#define OPEN_SIEGE_PLATFORM_HPP

#include <SDL.h>

SDL_JoystickType Siege_JoystickGetType(SDL_Joystick* joystick);

SDL_bool Siege_IsMouse(SDL_Joystick* joystick);

void Siege_InitVirtualJoysticks();

#endif// OPEN_SIEGE_PLATFORM_HPP
