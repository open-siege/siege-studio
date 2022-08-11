#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <memory>

int main()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    std::cerr << "could not initialize sdl2: " << SDL_GetError() << '\n';
    return 1;
  }

  {
    auto window = std::unique_ptr<SDL_Window, void(*)(SDL_Window*)>(SDL_CreateWindow(
                                                                       "Siege Input",
                                                                       SDL_WINDOWPOS_UNDEFINED,
                                                                       SDL_WINDOWPOS_UNDEFINED,
                                                                       640,
                                                                       480,
                                                                       SDL_WINDOW_SHOWN), SDL_DestroyWindow);

    if (!window)
    {
      std::cerr << "could not create window: " << SDL_GetError() << '\n';
      return 1;
    }

    auto* screenSurface = SDL_GetWindowSurface(window.get());
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xaa, 0xbb, 0xcc));
    SDL_UpdateWindowSurface(window.get());
    SDL_Delay(2000);
  }

  SDL_Quit();
  return 0;
}