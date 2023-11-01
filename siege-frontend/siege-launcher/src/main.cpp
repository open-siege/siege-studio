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

#include "platform/platform.hpp"
#include "joystick_info.hpp"
#include "virtual_joystick.hpp"
#include "interface/main.hpp"


constexpr static auto devices_which_crash = std::array<std::pair<Uint16, Uint16>, 3> {{
  { 4607, 2103 },
  { 4607, 2106 },
  { 121, 6287 }
}};


struct GameLauncherOnFrameBeginCallback
{
  std::vector<std::string> games;
  std::vector<siege::joystick_info> joysticks;


  GameLauncherOnFrameBeginCallback()
    : games({
      "Quake",
      "HeXen II",
      "Laser Arena",
      "CIA Operative: Solo Misisons",
      "battleMETAL",
      "Quake 2", 
      "Heretic 2",
      "SiN",
      "Kingpin: Life of Crime",
      "Soldier of Fortune",
      "Daikatana",
      "Anachronox",
      "Half-Life",
      "007 Nightfire",
      "Cry of Fear",
      "Quake III Arena",
      "Heavy Metal FAKK 2",
      "Star Trek: Voyager - Elite Force",
      "American McGee's Alice",
      "007 Agent Under Fire",
      "Return to Castle Wolfenstein",
      "Medal of Honor: Allied Assault",
      "Star Wars Jedi Knight II: Jedi Outcast",
      "Soldier of Fortune II: Double Helix",
      "Wolfenstein: Enemy Territory",
      "Star Wars Jedi Knight: Jedi Academy",
      "Call of Duty",
      "007 Everything or Nothing",
      "Iron Grip: Warlord",
      "Dark Salvation",
      "Quake Live"
    })
  {
  }


  void operator()() {
        ImGui::Begin("Controllers");

        if (joysticks.empty())
        {
          joysticks = siege::get_all_joysticks();
        }

          for (auto& controller : joysticks)
          {
            ImGui::Text("Controller name: %s", controller.name.c_str());
            ImGui::Text("Num buttons: %lu", controller.buttons.size());
            ImGui::Text("Num axes: %lu", controller.axes.size());
            ImGui::Text("Num hats: %lu", controller.hats.size());
          }
        ImGui::End();


        ImGui::Begin("Games");

        for (auto& game : games)
        {
            if (ImGui::Button(game.c_str()))
            {
               
            }
        }
       ImGui::End();

      ImGui::Begin("Settings");
              ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        if (ImGui::BeginTabBar("Game Settings", tab_bar_flags))
        {
                if (ImGui::BeginTabItem("Controls"))
                {
                   ImGui::BeginGroup();
                  ImGui::Text("Move Forward: Left Stick Y-Axis +");
                  ImGui::Text("Move Backward: Left Stick Y-Axis -");

                  ImGui::Text("Move Left: Left Stick X-Axis -");
                  ImGui::Text("Move Right: Left Stick X-Axis +");

                  ImGui::Text("Look Up: Right Stick Y-Axis +");
                  ImGui::Text("Look Down: Right Stick Y-Axis -");

                  ImGui::Text("Turn Left: Right Stick X-Axis -");
                  ImGui::Text("Turn Right: Left Stick X-Axis +");


                  ImGui::Text("Jump: X");
                  ImGui::Text("Crouch: O");
                  ImGui::Text("Reload: []");
                  ImGui::Text("Change Weapon: ^");

                  ImGui::Text("Change Item: L1");
                  ImGui::Text("Use Item: R1");
                  ImGui::Text("Secondary Attack: L2");
                  ImGui::Text("Primary Attack: R2");
                  ImGui::Text("Run: L3");
                  ImGui::Text("Melee: R3");

                  ImGui::EndGroup();
                  ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Graphics"))
                {
                  ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Audio"))
                {
                  ImGui::EndTabItem();
                }
  }
        ImGui::End();
  }
};


int main(int argc, char** argv)
{
  siege::SetHint(SDL_HINT_JOYSTICK_ROG_CHAKRAM, "1");
  siege::InitSubSystem(SDL_INIT_JOYSTICK);

  for (auto i = 0; i < siege::NumJoysticks(); ++i)
  {
    auto bad_device = std::find(devices_which_crash.begin(), devices_which_crash.end(),
                        std::make_pair(siege::JoystickGetDeviceVendor(i), siege::JoystickGetDeviceProduct(i)));

    if (bad_device != devices_which_crash.end())
    {
      siege::SetHint(SDL_HINT_DIRECTINPUT_ENABLED, "0");
    }
  }
  siege::QuitSubSystem(SDL_INIT_JOYSTICK);

  siege::AppCallbacks callbacks;
  callbacks.onWindowCreated = [](SDL_Window* window) {
    SDL_SetWindowTitle(window, "Siege Launcher");
    siege::InitSubSystem(SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
    siege::InitVirtualJoysticksFromKeyboardsAndMice();
    siege::InitVirtualJoysticksFromJoysticks();
  };
  callbacks.onNewFrame = GameLauncherOnFrameBeginCallback{};

  return siege::AppMain(std::vector<std::string>(argv, argv + argc), std::move(callbacks));
}