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
#include <sstream>
#include <iomanip>

#include <SDL.h>
#include <imgui.h>
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

#include "platform.hpp"

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

auto to_string(SDL_GameControllerType type)
{
  static_assert(SDL_CONTROLLER_TYPE_UNKNOWN == 0);
  static_assert(SDL_CONTROLLER_TYPE_GOOGLE_STADIA == 9);
  constexpr static std::array<const char*, 10> names = {
    "Unknown",
    "Xbox 360",
    "Xbox One",
    "PS3",
    "PS4",
    "Nintendo Switch Pro",
    "Virtual Controller",
    "PS5",
    "Amazon Luna",
    "Google Stadia"
  };

  auto type_index = std::size_t(type);

  return type_index < names.size() ? names[type_index] : "";
}

inline auto to_array(const SDL_JoystickGUID& guid)
{
  std::array<std::byte, 16> result;
  std::memcpy(result.data(), guid.data, sizeof(guid.data));

  return result;
}

auto to_byte_view(const SDL_JoystickGUID& guid)
{
  return std::basic_string_view<std::byte>(reinterpret_cast<const std::byte*>(guid.data), sizeof(guid.data));
}

template<typename Integral>
const char* to_hex(Integral value)
{
  static std::unordered_map<Integral, std::string> converted_values;

  auto result = converted_values.find(value);

  if (result == converted_values.end())
  {
    std::stringstream stream;
    stream << "0x"
           << std::setfill('0') << std::setw(sizeof(Integral) * 2)
           << std::hex << value;
    result = converted_values.emplace(value, stream.str()).first;
  }

  return result->second.c_str();
}

Uint8 Siege_GameControllerGetDPadAsHat(SDL_GameController* controller)
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

const char* Siege_GameControllerGetStringForButton(SDL_GameController* controller, SDL_GameControllerButton button)
{
  auto value = SDL_GameControllerGetStringForButton(button);
  auto type = SDL_GameControllerGetType(controller);

  if (type == SDL_GameControllerType::SDL_CONTROLLER_TYPE_PS3 || type == SDL_GameControllerType::SDL_CONTROLLER_TYPE_PS4 || type == SDL_GameControllerType::SDL_CONTROLLER_TYPE_PS5)
  {
    constexpr static std::array<std::pair<const char*, const char*>, 9> ps_button_mapping = {
      std::make_pair("a", "cross"),
      std::make_pair("b", "circle"),
      std::make_pair("x", "square"),
      std::make_pair("y", "triangle"),
      std::make_pair("back", "select"),
      std::make_pair("leftstick", "l3"),
      std::make_pair("rightstick", "r3"),
      std::make_pair("leftshoulder", "l1"),
      std::make_pair("rightshoulder", "r1")
    };

    auto value_str = value ? std::string_view(value) : std::string_view();

    auto mapping = std::find_if(ps_button_mapping.begin(), ps_button_mapping.end(), [&](const auto& pair) {
      return value_str == pair.first;
    });

    if (mapping != ps_button_mapping.end())
    {
      return mapping->second;
    }
  }

  return value;
}

int main(int, char**)
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0)
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

    Siege_InitVirtualJoysticks();

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

      // Same as the number of joysticks to make it easier to index.
      std::vector<std::shared_ptr<SDL_Haptic>> haptic_devices(SDL_NumJoysticks());
      std::vector<std::shared_ptr<SDL_GameController>> controllers(SDL_NumJoysticks());

      auto new_frame = []() {
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
      };

      bool controller_rumble = false;
      bool trigger_rumble = false;
      int low_frequency = 0xAAAA;
      int high_frequency = 0xAAAA;
      int duration = 1000;
      int left_trigger = 0xAAAA;
      int right_trigger = 0xAAAA;
      int trigger_duration = 1000;

      bool led_enabled = false;
      ImVec4 led_colour(1.0f, 0.0f, 1.0f, 0.5f);

      std::vector<std::pair<SDL_GameControllerButton, SDL_GameControllerButtonBind>> current_bindings;
      current_bindings.reserve(std::size_t(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MAX));

      while (running)
      {
        SDL_JoystickUpdate();
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
          haptic_devices.resize(SDL_NumJoysticks());
          controllers.resize(SDL_NumJoysticks());
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
                joystick = joysticks[i] = std::shared_ptr<SDL_Joystick>(SDL_JoystickOpen(i), [i, &haptic_devices, &controllers](auto* joystick) {
                  if (haptic_devices.size() > i && haptic_devices[i])
                  {
                    haptic_devices[i] = std::shared_ptr<SDL_Haptic>();
                  }

                  if (controllers.size() > i && controllers[i])
                  {
                    controllers[i] = std::shared_ptr<SDL_GameController>();
                  }

                  SDL_JoystickClose(joystick);
                });

                if (SDL_JoystickIsHaptic(joystick.get()))
                {
                  haptic_devices[i] = std::shared_ptr<SDL_Haptic>(SDL_HapticOpenFromJoystick(joystick.get()), SDL_HapticClose);
                }

                if (SDL_IsGameController(i) == SDL_TRUE)
                {
                  controllers[i] = std::shared_ptr<SDL_GameController>(SDL_GameControllerOpen(i), SDL_GameControllerClose);
                }
              }

              const auto vendor_id = Siege_JoystickGetVendor(joystick.get());
              const auto product_id = Siege_JoystickGetProduct(joystick.get());
              ImGui::Text("Device GUID: %s", to_string(SDL_JoystickGetDeviceGUID(i)).c_str());
              ImGui::Text("Vendor ID: %s (%d)",to_hex(vendor_id), vendor_id);
              ImGui::Text("Product ID: %s (%d)", to_hex(product_id), vendor_id);
              ImGui::Text("Product Version: %d", Siege_JoystickGetProductVersion(joystick.get()));
              ImGui::Text("Serial Number: %s", SDL_JoystickGetSerial(joystick.get()));

              if (SDL_JoystickIsVirtual(i) && Siege_IsMouse(joystick.get()))
              {
                ImGui::Text("Detected Type: Mouse");
              }
              else
              {
                ImGui::Text("Detected Type: %s", to_string(Siege_JoystickGetType(joystick.get())));
              }

              if (controllers[i])
              {
                ImGui::Text("Detected Controller Type: %s", to_string(SDL_GameControllerGetType(controllers[i].get())));
              }

              ImGui::Text("Num Buttons: %d", SDL_JoystickNumButtons(joystick.get()));
              ImGui::Text("Num Hats: %d", SDL_JoystickNumHats(joystick.get()));
              ImGui::Text("Num Axes: %d", SDL_JoystickNumAxes(joystick.get()));
              ImGui::Text("Num Balls: %d", SDL_JoystickNumBalls(joystick.get()));

              if (controllers[i])
              {
                ImGui::Text("Num Touchpads: %d", SDL_GameControllerGetNumTouchpads(controllers[i].get()));
              }

              ImGui::Text("Has LED: %s", SDL_JoystickHasLED(joystick.get()) == SDL_TRUE ? "True" : "False");
              ImGui::Text("Has Rumble: %s", SDL_JoystickHasRumble(joystick.get()) == SDL_TRUE ? "True" : "False");
              ImGui::Text("Has Triggers with Rumble: %s", SDL_JoystickHasRumbleTriggers(joystick.get()) == SDL_TRUE ? "True" : "False");


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
              if (SDL_JoystickNumHats(joystick.get()) == 0 && controllers[i] && std::any_of(d_pad_buttons.begin(), d_pad_buttons.end(), [&](auto button) {
                    return SDL_GameControllerHasButton(controllers[i].get(), button);
                  }))
              {
                auto value = Siege_GameControllerGetDPadAsHat(controllers[i].get());

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
                for (auto h = 0; h < SDL_JoystickNumHats(joystick.get()); ++h)
                {
                  auto value = SDL_JoystickGetHat(joystick.get(), h);

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
                  auto x_value = float(SDL_JoystickGetAxis(joystick.get(), x_axis.value.axis)) / std::numeric_limits<Sint16>::max() / 2;
                  auto y_value = float(SDL_JoystickGetAxis(joystick.get(), y_axis.value.axis)) / std::numeric_limits<Sint16>::max() / 2;
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
              else if (Siege_JoystickGetType(joystick.get()) == SDL_JoystickType::SDL_JOYSTICK_TYPE_FLIGHT_STICK || Siege_JoystickGetType(joystick.get()) == SDL_JoystickType::SDL_JOYSTICK_TYPE_GAMECONTROLLER)
              {
                if (SDL_JoystickNumAxes(joystick.get()) >= 2)
                {
                  auto x_value = float(SDL_JoystickGetAxis(joystick.get(), 0)) / std::numeric_limits<Sint16>::max() / 2;
                  auto y_value = float(SDL_JoystickGetAxis(joystick.get(), 1)) / std::numeric_limits<Sint16>::max() / 2;
                  ImVec2 alignment(x_value + 0.5f, y_value + 0.5f);
                  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
                  ImGui::Selectable("+", true, 0, ImVec2(200, 200));
                  ImGui::PopStyleVar();
                  ImGui::SameLine();
                }

                for (auto x = 2; x < SDL_JoystickNumAxes(joystick.get()); ++x)
                {
                  auto value = int(SDL_JoystickGetAxis(joystick.get(), x));
                  ImGui::VSliderInt("##int", ImVec2(18, 160), &value, std::numeric_limits<Sint16>::min(), std::numeric_limits<Sint16>::max());
                  ImGui::SameLine();
                }
              }
              else
              {
                for (auto x = 0; x < SDL_JoystickNumAxes(joystick.get()); ++x)
                {
                  auto value = int(SDL_JoystickGetAxis(joystick.get(), x));
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
                  auto value = SDL_JoystickGetButton(joystick.get(), binding.value.button) == 1;

                  ImGui::Selectable(Siege_GameControllerGetStringForButton(controllers[i].get(), button), value, 0, ImVec2(50, 50));
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
                for (auto x = 0; x < SDL_JoystickNumButtons(joystick.get()); ++x)
                {
                  auto value = SDL_JoystickGetButton(joystick.get(), x) == 1;
                  ImGui::Selectable(std::to_string(x + 1).c_str(), value, 0, ImVec2(50, 50));
                  ImGui::SameLine();

                  if ((x + 1) % 4 == 0)
                  {
                    ImGui::NewLine();
                  }
                }
              }

              ImGui::EndGroup();

              if (SDL_JoystickHasRumble(joystick.get()) == SDL_TRUE)
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
                    SDL_JoystickRumble(joystick.get(), Uint16(low_frequency), Uint16(high_frequency), Uint32(duration));
                  }
                  else
                  {
                    if (pending_timer.has_value())
                    {
                      SDL_RemoveTimer(pending_timer.value());
                    }
                    SDL_JoystickRumble(joystick.get(), 0, 0, 0);
                  }
                }

                ImGui::PushItemWidth(320.0f);
                ImGui::SliderInt("Low Frequency", &low_frequency, std::numeric_limits<Uint16>::min(), std::numeric_limits<Uint16>::max());
                ImGui::SliderInt("High Frequency", &high_frequency, std::numeric_limits<Uint16>::min(), std::numeric_limits<Uint16>::max());
                ImGui::SliderInt("Duration", &duration, std::numeric_limits<Uint16>::min(), 20000);
                ImGui::PopItemWidth();

                ImGui::EndGroup();
              }

              if (SDL_JoystickHasRumbleTriggers(joystick.get()) == SDL_TRUE)
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
                    SDL_JoystickRumbleTriggers(joystick.get(), Uint16(left_trigger), Uint16(right_trigger), Uint32(trigger_duration));
                  }
                  else
                  {
                    if (pending_timer.has_value())
                    {
                      SDL_RemoveTimer(pending_timer.value());
                    }
                    SDL_JoystickRumble(joystick.get(), 0, 0, 0);
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

              if (SDL_JoystickHasLED(joystick.get()) == SDL_TRUE)
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
                  SDL_JoystickSetLED(joystick.get(), Uint8(led_colour.x * 255), Uint8(led_colour.y * 255), Uint8(led_colour.z * 255));
                }
                else
                {
                  SDL_JoystickSetLED(joystick.get(), 0, 0, 0);
                }
                ImGui::EndGroup();
              }

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