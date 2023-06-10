#define SDL_MAIN_HANDLED
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <optional>
#include <utility>
#include <cstring>

#include <SDL.h>
#include <imgui.h>
#include "renderer/renderer.hpp"
#include "imgui_impl_sdl.h"
#include "interface/main.hpp"

namespace siege
{
  int AppMain(std::vector<std::string> args, AppCallbacks callbacks)
  {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
      std::cerr << "Error: " << SDL_GetError() << '\n';
      return -1;
    }

    // SDL scope
    {
      SDL_WindowFlags window_flags = SDL_WindowFlags(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
      auto window = std::unique_ptr<SDL_Window, void (*)(SDL_Window*)>(
        SDL_CreateWindow("Siege Input", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags), SDL_DestroyWindow);

      if (callbacks.onWindowCreated)
      {
        callbacks.onWindowCreated(window.get());
      }

      // ImGui scope
      {
        IMGUI_CHECKVERSION();
        auto context = std::unique_ptr<ImGuiContext, void (*)(ImGuiContext*)>(ImGui::CreateContext(), siege::RenderShutdown);

        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        auto render_context = siege::RenderInit(window.get());

        if (!render_context)
        {
          return -1;
        }

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        bool running = true;

        while (running)
        {
          SDL_Event event;
          while (SDL_PollEvent(&event))
          {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
            {
              running = false;
            }

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window.get()))
            {
              running = false;
            }

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(window.get()))
            {
              // Release all outstanding references to the swap chain's buffers before resizing.
              siege::Resize(*render_context);
            }
          }

          siege::NewFrame();

          callbacks.onNewFrame();

          ImGui::Render();
          siege::RenderReset(*render_context, clear_color);
          siege::RenderDrawData(ImGui::GetDrawData());

          if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
          {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
          }

          siege::RenderPresent(*render_context);
        } // main loop
      }
      // /ImGui scope
    } // /SDL scope

    SDL_Quit();

    return 0;
  }
}
