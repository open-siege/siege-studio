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
#include "virtual_joystick.hpp"
#include "interface/main.hpp"

inline auto to_array(const siege::JoystickGUID& guid)
{
  std::array<std::byte, 16> result;
  std::memcpy(result.data(), guid.data, sizeof(guid.data));

  return result;
}

auto to_byte_view(const siege::JoystickGUID& guid)
{
  return std::basic_string_view<std::byte>(reinterpret_cast<const std::byte*>(guid.data), sizeof(guid.data));
}

Uint8 GameControllerGetDPadAsHat(SDL_GameController* controller)
{
  if (SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP) && SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT))
  {
    return SDL_HAT_LEFTUP;
  }
  else if (SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP) && SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
  {
    return SDL_HAT_RIGHTUP;
  }

  if (SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN) && SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT))
  {
    return SDL_HAT_LEFTDOWN;
  }
  else if (SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN) && SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
  {
    return SDL_HAT_RIGHTDOWN;
  }

  if (SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP))
  {
    return SDL_HAT_UP;
  }
  else if (SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN))
  {
    return SDL_HAT_DOWN;
  }
  else if (SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT))
  {
    return SDL_HAT_LEFT;
  }
  else if (SDL_GameControllerGetButton(controller, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
  {
    return SDL_HAT_RIGHT;
  }

  return SDL_HAT_CENTERED;
}

constexpr static auto devices_which_crash = std::array<std::pair<Uint16, Uint16>, 3> {{
  { 4607, 2103 },
  { 4607, 2106 },
  { 121, 6287 }
}};


struct LauncherOnFrameBeginCallback
{
  std::vector<std::shared_ptr<siege::Joystick>> joysticks;
  
  // Same as the number of joysticks to make it easier to index.
  std::vector<std::shared_ptr<SDL_Haptic>> haptic_devices;
  std::vector<std::shared_ptr<SDL_GameController>> controllers;
  bool controller_rumble;
  bool trigger_rumble;
  int low_frequency;
  int high_frequency;
  int duration;
  int left_trigger;
  int right_trigger;
  int trigger_duration;

  bool led_enabled;
  ImVec4 led_colour;

  std::vector<std::pair<SDL_GameControllerButton, SDL_GameControllerButtonBind>> current_bindings;

  LauncherOnFrameBeginCallback()
   : controller_rumble(false), 
    trigger_rumble(false),
      low_frequency(0xAAAA),
      high_frequency(0xAAAA),
      duration(1000),
      left_trigger(0xAAAA),
      right_trigger(0xAAAA),
      trigger_duration(1000),
      led_enabled(false),
      led_colour(1.0f, 0.0f, 1.0f, 0.5f)
  {
      joysticks.assign(SDL_NumJoysticks(), nullptr);
      haptic_devices.assign(SDL_NumJoysticks(), nullptr);
      controllers.assign(SDL_NumJoysticks(), nullptr);
      current_bindings.reserve(std::size_t(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MAX));
  }


  void operator()() {
        ImGui::Begin("Input Info");
        ImGui::Text("Number of controllers: %d", SDL_NumJoysticks());

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        if (joysticks.size() != SDL_NumJoysticks())
        {
          joysticks.resize(SDL_NumJoysticks());
          haptic_devices.resize(SDL_NumJoysticks());
          controllers.resize(SDL_NumJoysticks());
        }

        if (ImGui::BeginTabBar("Controllers", tab_bar_flags))
        {
          std::string temp;
          temp.reserve(32);
          for (auto i = 0; i < SDL_NumJoysticks(); ++i)
          {
            temp.assign("#" + std::to_string(i + 1) + " " + siege::JoystickNameForIndex(i));
            if (ImGui::BeginTabItem(temp.c_str()))
            {
              auto joystick = joysticks[i];

              if (!joystick || (joystick && siege::JoystickGetDeviceInstanceID(i) != siege::JoystickInstanceID(joystick.get())))
              {
                joystick = joysticks[i] = std::shared_ptr<siege::Joystick>(siege::JoystickOpen(i), [i, this](auto* joystick) {
                  if (haptic_devices.size() > i && haptic_devices[i])
                  {
                    haptic_devices[i] = std::shared_ptr<SDL_Haptic>();
                  }

                  if (controllers.size() > i && controllers[i])
                  {
                    controllers[i] = std::shared_ptr<SDL_GameController>();
                  }

                  siege::JoystickClose(joystick);
                });

                if (siege::JoystickIsHaptic(joystick.get()))
                {
                  haptic_devices[i] = std::shared_ptr<SDL_Haptic>(SDL_HapticOpenFromJoystick(joystick.get()), SDL_HapticClose);
                }

                if (SDL_IsGameController(i) == SDL_TRUE)
                {
                  controllers[i] = std::shared_ptr<SDL_GameController>(SDL_GameControllerOpen(i), SDL_GameControllerClose);
                }
              }

              const auto vendor_id = siege::JoystickGetVendor(joystick.get());
              const auto product_id = siege::JoystickGetProduct(joystick.get());

              ImGui::BeginGroup();

              if (ImGui::Button("Export Device Info"))
              {
                siege::SaveDeviceToFile(joystick.get(), i);
              }

              ImGui::SameLine();

              if (ImGui::Button("Export Info for All Devices"))
              {
                siege::SaveAllDevicesToFile();
              }

              ImGui::EndGroup();

              ImGui::Text("Device GUID: %s", siege::to_string(siege::JoystickGetDeviceGUID(i)).c_str());
              ImGui::Text("Vendor ID: %s (%d)", siege::to_hex(vendor_id), vendor_id);
              ImGui::Text("Product ID: %s (%d)", siege::to_hex(product_id), product_id);
              ImGui::Text("Product Version: %d", siege::JoystickGetProductVersion(joystick.get()));
              ImGui::Text("Serial Number: %s", siege::JoystickGetSerial(joystick.get()));

              if (siege::JoystickIsVirtual(i) && siege::IsMouse(joystick.get()))
              {
                ImGui::Text("Detected Type: Mouse");
              }
              else
              {
                ImGui::Text("Detected Type: %s", siege::to_string(siege::JoystickGetType(joystick.get())));
              }

              if (controllers[i])
              {
                ImGui::Text("Detected Controller Type: %s", siege::to_string(SDL_GameControllerGetType(controllers[i].get())));
              }

              ImGui::Text("Num Buttons: %d", siege::JoystickNumButtons(joystick.get()));
              ImGui::Text("Num Hats: %d", siege::JoystickNumHats(joystick.get()));
              ImGui::Text("Num Axes: %d", siege::JoystickNumAxes(joystick.get()));
              ImGui::Text("Num Balls: %d", siege::JoystickNumBalls(joystick.get()));

              if (controllers[i])
              {
                ImGui::Text("Num Touchpads: %d", SDL_GameControllerGetNumTouchpads(controllers[i].get()));
              }

              ImGui::Text("Has LED: %s", siege::JoystickHasLED(joystick.get()) == SDL_TRUE ? "True" : "False");
              ImGui::Text("Has Rumble: %s", siege::JoystickHasRumble(joystick.get()) == SDL_TRUE ? "True" : "False");
              ImGui::Text("Has Triggers with Rumble: %s", siege::JoystickHasRumbleTriggers(joystick.get()) == SDL_TRUE ? "True" : "False");


              if (haptic_devices[i])
              {
                ImGui::Text("Num Supported Haptic Effects: %d", SDL_HapticNumEffects(haptic_devices[i].get()));
              }

              constexpr static auto hat_states = std::array<std::tuple<int, const char*, ImVec2>, 9>{
                std::make_tuple(SDL_HAT_LEFTUP, "\\", ImVec2(0.0f, 0.5f)),
                std::make_tuple(SDL_HAT_UP, "^\n|", ImVec2(0.5f, 0.5f)),
                std::make_tuple(SDL_HAT_RIGHTUP, "/", ImVec2(1.0f, 0.5f)),
                std::make_tuple(SDL_HAT_LEFT, "<-", ImVec2(0.0f, 0.5f)),
                std::make_tuple(SDL_HAT_CENTERED, "o", ImVec2(0.5f, 0.5f)),
                std::make_tuple(SDL_HAT_RIGHT, "->", ImVec2(1.0f, 0.5f)),
                std::make_tuple(SDL_HAT_LEFTDOWN, "/", ImVec2(0.0f, 0.5f)),
                std::make_tuple(SDL_HAT_DOWN, "|\nv", ImVec2(0.5f, 0.5f)),
                std::make_tuple(SDL_HAT_RIGHTDOWN, "\\", ImVec2(1.0f, 0.5f)),
              };

              constexpr static auto d_pad_buttons = std::array<SDL_GameControllerButton, 4>{
                SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP,
                SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN,
                SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT,
                SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT
              };

              ImGui::BeginGroup();
              if (siege::JoystickNumHats(joystick.get()) == 0 && controllers[i] && std::any_of(d_pad_buttons.begin(), d_pad_buttons.end(), [&](auto button) {
                    return SDL_GameControllerHasButton(controllers[i].get(), button);
                  }))
              {
                auto value = GameControllerGetDPadAsHat(controllers[i].get());

                auto x = 0;
                for (auto& [state, label, alignment] : hat_states)
                {
                  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
                  ImGui::Selectable(label, state == value, 0, ImVec2(25, 25));
                  ImGui::PopStyleVar();
                  ImGui::SameLine();

                  if ((x + 1) % 3 == 0)
                  {
                    ImGui::NewLine();
                  }
                  x++;
                }
              }
              else
              {
                for (auto h = 0; h < siege::JoystickNumHats(joystick.get()); ++h)
                {
                  auto value = siege::JoystickGetHat(joystick.get(), h);

                  auto x = 0;
                  for (auto& [state, label, alignment] : hat_states)
                  {
                    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
                    ImGui::Selectable(label, state == value, 0, ImVec2(25, 25));
                    ImGui::PopStyleVar();
                    ImGui::SameLine();

                    if ((x + 1) % 3 == 0)
                    {
                      ImGui::NewLine();
                    }
                    x++;
                  }
                }
              }
              ImGui::EndGroup();

              ImGui::SameLine();

              if (controllers[i])
              {
                auto left_x_binding = SDL_GameControllerGetBindForAxis(controllers[i].get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX);
                auto left_y_binding = SDL_GameControllerGetBindForAxis(controllers[i].get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY);
                auto right_x_binding = SDL_GameControllerGetBindForAxis(controllers[i].get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX);
                auto right_y_binding = SDL_GameControllerGetBindForAxis(controllers[i].get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY);

                std::vector<std::pair<SDL_GameControllerButtonBind, SDL_GameControllerButtonBind>> valid_bindings;
                valid_bindings.reserve(2);

                if (left_x_binding.bindType == SDL_CONTROLLER_BINDTYPE_AXIS && left_y_binding.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
                {
                  valid_bindings.emplace_back(std::make_pair(left_x_binding, left_y_binding));
                }

                if (right_x_binding.bindType == SDL_CONTROLLER_BINDTYPE_AXIS && right_x_binding.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
                {
                  valid_bindings.emplace_back(std::make_pair(right_x_binding, right_y_binding));
                }

                for (auto& [x_axis, y_axis] : valid_bindings)
                {
                  auto x_value = float(siege::JoystickGetAxis(joystick.get(), x_axis.value.axis)) / std::numeric_limits<Sint16>::max() / 2;
                  auto y_value = float(siege::JoystickGetAxis(joystick.get(), y_axis.value.axis)) / std::numeric_limits<Sint16>::max() / 2;
                  ImVec2 alignment(x_value + 0.5f, y_value + 0.5f);
                  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
                  ImGui::Selectable("+", true, 0, ImVec2(200, 200));
                  ImGui::PopStyleVar();
                  ImGui::SameLine();
                }

                auto left_trigger_axis = SDL_GameControllerGetBindForAxis(controllers[i].get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT);
                auto right_trigger_axis = SDL_GameControllerGetBindForAxis(controllers[i].get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

                if (left_trigger_axis.bindType != SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_NONE)
                {
                  auto value = int(SDL_GameControllerGetAxis(controllers[i].get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT));
                  ImGui::VSliderInt("##int", ImVec2(18, 160), &value, std::numeric_limits<Sint16>::min(), std::numeric_limits<Sint16>::max());
                  ImGui::SameLine();
                }

                if (right_trigger_axis.bindType != SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_NONE)
                {
                  auto value = int(SDL_GameControllerGetAxis(controllers[i].get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
                  ImGui::VSliderInt("##int", ImVec2(18, 160), &value, std::numeric_limits<Sint16>::min(), std::numeric_limits<Sint16>::max());
                  ImGui::SameLine();
                }
              }
              else if (siege::JoystickGetType(joystick.get()) == siege::JoystickType::SDL_JOYSTICK_TYPE_FLIGHT_STICK || siege::JoystickGetType(joystick.get()) == siege::JoystickType::SDL_JOYSTICK_TYPE_GAMECONTROLLER)
              {
                if (siege::JoystickNumAxes(joystick.get()) >= 2)
                {
                  auto x_value = float(siege::JoystickGetAxis(joystick.get(), 0)) / std::numeric_limits<Sint16>::max() / 2;
                  auto y_value = float(siege::JoystickGetAxis(joystick.get(), 1)) / std::numeric_limits<Sint16>::max() / 2;
                  ImVec2 alignment(x_value + 0.5f, y_value + 0.5f);
                  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
                  ImGui::Selectable("+", true, 0, ImVec2(200, 200));
                  ImGui::PopStyleVar();
                  ImGui::SameLine();
                }

                for (auto x = 2; x < siege::JoystickNumAxes(joystick.get()); ++x)
                {
                  auto value = int(siege::JoystickGetAxis(joystick.get(), x));
                  ImGui::VSliderInt("##int", ImVec2(18, 160), &value, std::numeric_limits<Sint16>::min(), std::numeric_limits<Sint16>::max());
                  ImGui::SameLine();
                }
              }
              else
              {
                for (auto x = 0; x < siege::JoystickNumAxes(joystick.get()); ++x)
                {
                  auto value = int(siege::JoystickGetAxis(joystick.get(), x));
                  ImGui::VSliderInt("##int", ImVec2(18, 160), &value, std::numeric_limits<Sint16>::min(), std::numeric_limits<Sint16>::max());
                  ImGui::SameLine();
                }
              }


              ImGui::NewLine();

              ImGui::BeginGroup();
              if (controllers[i])
              {
                constexpr static auto known_buttons = std::array<SDL_GameControllerButton, 17>{
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_GUIDE,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MISC1,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE1,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE2,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE3,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_PADDLE4,
                  SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_TOUCHPAD
                };

                current_bindings.clear();

                for (auto button : known_buttons)
                {
                  if (SDL_GameControllerHasButton(controllers[i].get(), button) == SDL_TRUE)
                  {
                    current_bindings.emplace_back(std::make_pair(button, SDL_GameControllerGetBindForButton(controllers[i].get(), button)));
                  }
                }

                auto x = 0;
                for (auto& [button, binding] : current_bindings)
                {
                  auto value = siege::JoystickGetButton(joystick.get(), binding.value.button) == 1;

                  ImGui::Selectable(siege::GameControllerGetStringForButton(controllers[i].get(), button), value, 0, ImVec2(50, 50));
                  ImGui::SameLine();

                  if ((x + 1) % 4 == 0)
                  {
                    ImGui::NewLine();
                  }
                  x++;
                }
              }
              else
              {
                for (auto x = 0; x < siege::JoystickNumButtons(joystick.get()); ++x)
                {
                  auto value = siege::JoystickGetButton(joystick.get(), x) == 1;
                  ImGui::Selectable(std::to_string(x + 1).c_str(), value, 0, ImVec2(50, 50));
                  ImGui::SameLine();

                  if ((x + 1) % 4 == 0)
                  {
                    ImGui::NewLine();
                  }
                }
              }

              ImGui::EndGroup();

              if (siege::JoystickHasRumble(joystick.get()) == SDL_TRUE)
              {
                ImGui::SameLine();
                ImGui::BeginGroup();

                if (ImGui::Checkbox("Controller Rumble", &controller_rumble))
                {
                  static std::optional<SDL_TimerID> pending_timer;
                  if (controller_rumble)
                  {
                    pending_timer.emplace(SDL_AddTimer(
                      duration, [](Uint32 interval, void* param) -> Uint32 {
                        pending_timer.reset();
                        bool* should_rumble = reinterpret_cast<bool*>(param);
                        *should_rumble = false;
                        return 0;
                      },
                      &controller_rumble));
                    siege::JoystickRumble(joystick.get(), Uint16(low_frequency), Uint16(high_frequency), Uint32(duration));
                  }
                  else
                  {
                    if (pending_timer.has_value())
                    {
                      SDL_RemoveTimer(pending_timer.value());
                    }
                    siege::JoystickRumble(joystick.get(), 0, 0, 0);
                  }
                }

                ImGui::PushItemWidth(320.0f);
                ImGui::SliderInt("Low Frequency", &low_frequency, std::numeric_limits<Uint16>::min(), std::numeric_limits<Uint16>::max());
                ImGui::SliderInt("High Frequency", &high_frequency, std::numeric_limits<Uint16>::min(), std::numeric_limits<Uint16>::max());
                ImGui::SliderInt("Duration", &duration, std::numeric_limits<Uint16>::min(), 20000);
                ImGui::PopItemWidth();

                ImGui::EndGroup();
              }

              if (siege::JoystickHasRumbleTriggers(joystick.get()) == SDL_TRUE)
              {
                static std::optional<SDL_TimerID> pending_timer;
                ImGui::SameLine();
                ImGui::BeginGroup();

                if (ImGui::Checkbox("Trigger Rumble", &trigger_rumble))
                {
                  if (trigger_rumble)
                  {
                    pending_timer.emplace(SDL_AddTimer(
                      trigger_duration, [](Uint32 interval, void* param) -> Uint32 {
                        pending_timer.reset();
                        bool* should_rumble = reinterpret_cast<bool*>(param);
                        *should_rumble = false;
                        return 0;
                      },
                      &trigger_rumble));
                    siege::JoystickRumbleTriggers(joystick.get(), Uint16(left_trigger), Uint16(right_trigger), Uint32(trigger_duration));
                  }
                  else
                  {
                    if (pending_timer.has_value())
                    {
                      SDL_RemoveTimer(pending_timer.value());
                    }
                    siege::JoystickRumble(joystick.get(), 0, 0, 0);
                  }
                }

                ImGui::PushItemWidth(320.0f);
                ImGui::SliderInt("Left Trigger", &left_trigger, std::numeric_limits<Uint16>::min(), std::numeric_limits<Uint16>::max());
                ImGui::SliderInt("Right Trigger", &right_trigger, std::numeric_limits<Uint16>::min(), std::numeric_limits<Uint16>::max());
                ImGui::SliderInt("Trigger Duration", &trigger_duration, std::numeric_limits<Uint16>::min(), 20000);
                ImGui::PopItemWidth();

                ImGui::EndGroup();
              }

              ImGui::NewLine();

              if (controllers[i])
              {
                ImGui::BeginGroup();
                for (auto x = 0; x < SDL_GameControllerGetNumTouchpads(controllers[i].get()); ++x)
                {
                  Uint8 state;
                  float x_coord = 0;
                  float y_coord = 0;
                  float pressure = 0;

                  SDL_GameControllerGetTouchpadFinger(controllers[i].get(), x, 0, &state, &x_coord, &y_coord, &pressure);

                  ImGui::Text("Touchpad %d", x + 1);
                  ImGui::Text("Num Simultaneous Fingers: %d", SDL_GameControllerGetNumTouchpadFingers(controllers[i].get(), x));
                  ImVec2 alignment(x_coord, y_coord);
                  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
                  ImGui::Selectable("+", true, 0, ImVec2(200, 200));
                  ImGui::PopStyleVar();
                  ImGui::SameLine();
                }
                ImGui::EndGroup();
              }

              if (siege::JoystickHasLED(joystick.get()) == SDL_TRUE)
              {
                ImGui::SameLine();
                ImGui::BeginGroup();
                if (ImGui::Checkbox("LED On/Off", &led_enabled))
                {
                }
                ImGui::PushItemWidth(320.0f);
                ImGui::ColorPicker3("LED Colour", reinterpret_cast<float*>(&led_colour));
                ImGui::PopItemWidth();

                if (led_enabled)
                {
                  siege::JoystickSetLED(joystick.get(), Uint8(led_colour.x * 255), Uint8(led_colour.y * 255), Uint8(led_colour.z * 255));
                }
                else
                {
                  siege::JoystickSetLED(joystick.get(), 0, 0, 0);
                }
                ImGui::EndGroup();
              }

              ImGui::EndTabItem();
            }
          }
          ImGui::EndTabBar();
        }

        ImGui::End();
  }
};


struct GameLauncherOnFrameBeginCallback
{
  
  GameLauncherOnFrameBeginCallback()
  {
  }


  void operator()() {
        ImGui::Begin("Games");


        ImGui::BeginGroup();
            ImGui::Text("Detected Games: ");

            if (ImGui::Button("Quake 2"))
            {
               
            }

            if (ImGui::Button("Heretic 2"))
            {
       
            }

            if (ImGui::Button("SiN"))
            {
       
            }

            if (ImGui::Button("Kingpin: Life of Crime"))
            {
        
            }

            if (ImGui::Button("Soldier of Fortune"))
            {
          
            }

            if (ImGui::Button("Anachronox"))
            {
             
            }
        ImGui::EndGroup();

        //ImGui::SameLine();

        if (ImGui::BeginTabBar("Settings"))
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
                ImGui::EndTabBar();
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