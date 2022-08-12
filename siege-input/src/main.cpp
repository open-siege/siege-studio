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

auto to_string(SDL_JoystickType type)
{
  static_assert(SDL_JOYSTICK_TYPE_UNKNOWN == 0);
  static_assert(SDL_JOYSTICK_TYPE_THROTTLE == 9);
  constexpr static std::array<const char*, 10> names = {
    "Unknown",
    "Game Controller",
    "Wheel",
    "Arcade Stick",
    "Flight Stick",
    "Dance Pad",
    "Guitar",
    "Drum Kit",
    "Arcade Pad",
    "Throttle"
  };

  auto type_index = std::size_t(type);

  return type_index < names.size() ? names[type_index] : "";
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

      std::vector<std::shared_ptr<SDL_Joystick>> joysticks(SDL_NumJoysticks());

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

        if (joysticks.size() != SDL_NumJoysticks())
        {
          joysticks.resize(SDL_NumJoysticks());
        }

        if (ImGui::BeginTabBar("Controllers", tab_bar_flags))
        {
          std::string temp;
          temp.reserve(32);
          for (auto i = 0; i < SDL_NumJoysticks(); ++i)
          {
            temp.assign("#" + std::to_string(i + 1) + " " + SDL_JoystickNameForIndex(i));
            if (ImGui::BeginTabItem(temp.c_str()))
            {
              auto joystick = joysticks[i];

              if (!joystick || (joystick && SDL_JoystickGetDeviceInstanceID(i) != SDL_JoystickInstanceID(joystick.get())))
              {
                joystick = joysticks[i] = std::shared_ptr<SDL_Joystick>(SDL_JoystickOpen(i), SDL_JoystickClose);
              }

              ImGui::Text("Device GUID %s", to_string(SDL_JoystickGetDeviceGUID(i)).c_str());
              ImGui::Text("Vendor ID %d", SDL_JoystickGetVendor(joystick.get()));
              ImGui::Text("Product ID %d", SDL_JoystickGetProduct(joystick.get()));
              ImGui::Text("Product Version %d", SDL_JoystickGetProductVersion(joystick.get()));
              ImGui::Text("Serial Number %s", SDL_JoystickGetSerial(joystick.get()));
              ImGui::Text("Detected Type %s", to_string(SDL_JoystickGetType(joystick.get())));
              ImGui::Text("Num Buttons %d", SDL_JoystickNumButtons(joystick.get()));
              ImGui::Text("Num Hats %d", SDL_JoystickNumHats(joystick.get()));
              ImGui::Text("Num Axes %d", SDL_JoystickNumAxes(joystick.get()));
              ImGui::Text("Num Balls %d", SDL_JoystickNumBalls(joystick.get()));
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