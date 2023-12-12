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
#include <iostream>
#include <fstream>

#include <SDL.h>
#include <imgui.h>
#include "platform/platform.hpp"
#include "joystick_info.hpp"
#include "game_info.hpp"
#include "virtual_joystick.hpp"
#include "interface/main.hpp"


constexpr static auto devices_which_crash = std::array<std::pair<Uint16, Uint16>, 3> {{
  { 4607, 2103 },
  { 4607, 2106 },
  { 121, 6287 }
}};


const std::string& to_string(std::string_view value, std::size_t index = 0)
{
  thread_local std::array<std::string, 10> strings;

  if (index > strings.size() - 1)
  {
    index = strings.size() - 1;
  }

  strings[index].assign(value);
  return strings[index];
}

struct GameLauncherOnFrameBeginCallback
{
  std::optional<siege::game_info> selected_game;
  std::vector<siege::game_info> games;
  std::vector<siege::joystick_info> joysticks;

  GameLauncherOnFrameBeginCallback()
    : games(siege::get_supported_games())
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
            if (ImGui::Button(to_string(game.english_name).c_str()))
            {
              selected_game.emplace(game);
            }
        }
       ImGui::End();

      ImGui::Begin("Settings");
              ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        if (ImGui::BeginTabBar("Game Settings", tab_bar_flags))
        {
                if (ImGui::BeginTabItem("Controls"))
                {

                  if (selected_game.has_value())
                  {
                    decltype(joysticks) game_joysticks;
                    game_joysticks.reserve(joysticks.size());
                    std::transform(joysticks.begin(), joysticks.end(), std::back_inserter(game_joysticks), [&](const auto& joystick) {
                        auto add_actions = selected_game.value().add_default_actions;
                        auto add_metadata = selected_game.value().add_input_metadata;

                        return add_actions(add_metadata(amend_controller_info(joystick)));
                    });

                    if (ImGui::Button("Save files"))
                    {
                          auto configs = selected_game.value().create_game_configs(game_joysticks);

                          for (auto& config : configs)
                          {
                              std::visit([&](const auto& raw_config) {
                                std::ofstream config_file(config.path, std::ios::trunc | std::ios::binary);
                                raw_config.save(config_file);
                              }, config.config);
                          }
                          
                    }

                    for (auto& joystick_info : game_joysticks)
                    {
                      ImGui::BeginGroup();

                        ImGui::Text("%s", to_string(joystick_info.name).c_str());

                        for (auto& axis : joystick_info.axes)
                        {
                          if (axis.axis_type.has_value())
                          {
                            for (auto& action : axis.actions)
                            {
                              ImGui::Text("%s: %s %s", to_string(action.name).c_str(), 
                                      to_string(action.target_meta_name, 1).c_str(), 
                                      to_string(axis.axis_type.value(), 2).c_str());
                            }
                          }
                        }

                        for (auto& button : joystick_info.buttons)
                        {
                          if (button.button_type.has_value())
                          {
                            for (auto& action : button.actions)
                            {
                              ImGui::Text("%s: %s %s", to_string(action.name).c_str(), 
                                      to_string(action.target_meta_name, 1).c_str(), 
                                      to_string(button.button_type.value(), 2).c_str());
                            }

                          }
                        }

                        for (auto& hat : joystick_info.hats)
                        {
                            for (auto& action : hat.actions)
                            {
                              ImGui::Text("%s: %s", to_string(action.name).c_str(), 
                                      to_string(action.target_meta_name, 1).c_str());
                            }
                        }

                    ImGui::EndGroup();
                    }
                  }
                   
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
 //   siege::InitVirtualJoysticksFromKeyboardsAndMice();
 //   siege::InitVirtualJoysticksFromJoysticks();
  };
  callbacks.onNewFrame = GameLauncherOnFrameBeginCallback{};

  return siege::AppMain(std::vector<std::string>(argv, argv + argc), std::move(callbacks));
}