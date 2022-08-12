#define SDL_MAIN_HANDLED

#include <imgui.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <array>
#include <list>
#include <unordered_map>
#include <algorithm>

#include <SDL.h>

auto to_string(const SDL_JoystickGUID& guid)
{
  std::string result(36, '\0');

  SDL_JoystickGetGUIDString(guid, result.data(), int(result.size()));

  return result;
}

auto to_array(const SDL_JoystickGUID& guid)
{
  std::array<std::byte, 16> result;
  std::memcpy(result.data(), guid.data, sizeof(guid.data));

  return result;
}

auto to_byte_view(const SDL_JoystickGUID& guid)
{
  return std::basic_string_view<std::byte> (reinterpret_cast<const std::byte*>(guid.data), sizeof(guid.data));
}

int main(int, char**)
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) != 0)
  {
    std::cerr << "Error: " << SDL_GetError() << '\n';
    return -1;
  }

  // SDL scope
  {
    SDL_WindowFlags window_flags = SDL_WindowFlags(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    auto window = std::unique_ptr<SDL_Window, void (*)(SDL_Window*)>(
      SDL_CreateWindow("Siege Input", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags), SDL_DestroyWindow);

    auto renderer = std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer*)>(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED), SDL_DestroyRenderer);
    if (!renderer)
    {
      SDL_Log("Error creating SDL_Renderer!");
      return -1;
    }

    // The Rapoo V600S DirectInput driver causes an access violation in winmm when this is called.
    // Will find out if I can trigger vibration through the Joystick API alone, or if this needs some work.
    // SDL_InitSubSystem(SDL_INIT_HAPTIC);

    // ImGui scope
    {
      IMGUI_CHECKVERSION();
      auto context = std::unique_ptr<ImGuiContext, void (*)(ImGuiContext*)>(ImGui::CreateContext(), [](auto* context) {
        ImGui_ImplSDLRenderer_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext(context);
      });

      ImGuiIO& io = ImGui::GetIO();

      ImGui::StyleColorsDark();

      ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get());
      ImGui_ImplSDLRenderer_Init(renderer.get());

      bool show_demo_window = true;
      ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

      bool running = true;

      std::list<std::basic_string<std::byte>> joysticks_guids;
      std::unordered_map<std::basic_string_view<std::byte>, std::unique_ptr<SDL_Joystick, void(*)(SDL_Joystick *)>> joysticks;

      auto new_frame = []() {
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
      };

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
        }

        new_frame();

        if (show_demo_window)
        {
          ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::Begin("Input Info");
        ImGui::Text("Number of controllers: %d", SDL_NumJoysticks());

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("Controllers", tab_bar_flags))
        {
          std::string temp;
          temp.reserve(32);
          for (auto i = 0; i < SDL_NumJoysticks(); ++i)
          {
            temp.assign("#" + std::to_string(i + 1) + " " + SDL_JoystickNameForIndex(i));
            if (ImGui::BeginTabItem(temp.c_str()))
            {
              auto device_guid = SDL_JoystickGetDeviceGUID(i);
              auto joystick = joysticks.find(to_byte_view(device_guid));

              if (joystick == joysticks.end())
              {
                joystick = joysticks.emplace(joysticks_guids.emplace_back(to_byte_view(device_guid)),
                                      std::unique_ptr<SDL_Joystick, void(*)(SDL_Joystick *)>(SDL_JoystickOpen(i), SDL_JoystickClose)
                                      ).first;
              }

              ImGui::Text("Device GUID %s", to_string(device_guid).c_str());
              ImGui::Text("Vendor ID %d", SDL_JoystickGetVendor(joystick->second.get()));
              ImGui::Text("Product ID %d", SDL_JoystickGetProduct(joystick->second.get()));
              ImGui::Text("Product Version %d", SDL_JoystickGetProductVersion(joystick->second.get()));
              ImGui::Text("Serial Number %s", SDL_JoystickGetSerial(joystick->second.get()));
              ImGui::EndTabItem();
            }
          }
          ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer.get(), (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer.get());
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer.get());
      }
    }
    // /ImGui scope
  }

  SDL_Quit();

  return 0;
}