#ifndef OPEN_SIEGE_PLATFORM_HPP
#define OPEN_SIEGE_PLATFORM_HPP

#include <SDL.h>
#include <string>
#include <array>
#include <iomanip>
#include <unordered_map>
#include <sstream>
#include <nlohmann/json.hpp>

namespace siege
{
  using JoystickGUID = SDL_JoystickGUID;
  using Joystick = SDL_Joystick;
  using JoystickType = SDL_JoystickType;

  constexpr static auto& InitSubSystem = SDL_InitSubSystem;
  constexpr static auto& QuitSubSystem = SDL_QuitSubSystem;
  constexpr static auto& WasInit = SDL_WasInit;
  constexpr static auto& SetHint = SDL_SetHint;

  constexpr static auto& NumJoysticks = SDL_NumJoysticks;
  constexpr static auto& JoystickFromInstanceID = SDL_JoystickFromInstanceID;
  constexpr static auto& JoystickOpen = SDL_JoystickOpen;
  constexpr static auto& JoystickClose = SDL_JoystickClose;

  constexpr static auto& JoystickGetDeviceVendor = SDL_JoystickGetDeviceVendor;
  constexpr static auto& JoystickGetDeviceProduct = SDL_JoystickGetDeviceProduct;
  constexpr static auto& JoystickGetDeviceProductVersion = SDL_JoystickGetDeviceProductVersion;
  constexpr static auto& JoystickGetSerial = SDL_JoystickGetSerial;
  constexpr static auto& JoystickGetDeviceGUID = SDL_JoystickGetDeviceGUID;
  constexpr static auto& JoystickGetPlayerIndex = SDL_JoystickGetPlayerIndex;
  constexpr static auto& JoystickPath = SDL_JoystickPath;
  constexpr static auto& JoystickGetDeviceInstanceID = SDL_JoystickGetDeviceInstanceID;
  constexpr static auto& JoystickGetDeviceType = SDL_JoystickGetDeviceType;
  constexpr static auto& JoystickNameForIndex = SDL_JoystickNameForIndex;
  constexpr static auto& JoystickName = SDL_JoystickName;
  constexpr static auto& JoystickInstanceID = SDL_JoystickInstanceID;

  constexpr static auto& JoystickIsVirtual = SDL_JoystickIsVirtual;
  constexpr static auto& JoystickIsHaptic = SDL_JoystickIsHaptic;
  constexpr static auto& JoystickHasLED = SDL_JoystickHasLED;
  constexpr static auto& JoystickHasRumble = SDL_JoystickHasRumble;
  constexpr static auto& JoystickHasRumbleTriggers = SDL_JoystickHasRumbleTriggers;

  constexpr static auto& JoystickNumAxes = SDL_JoystickNumAxes;
  constexpr static auto& JoystickGetAxis = SDL_JoystickGetAxis;
  constexpr static auto& JoystickNumHats = SDL_JoystickNumHats;
  constexpr static auto& JoystickGetHat = SDL_JoystickGetHat;
  constexpr static auto& JoystickNumButtons = SDL_JoystickNumButtons;
  constexpr static auto& JoystickGetButton = SDL_JoystickGetButton;
  constexpr static auto& JoystickNumBalls = SDL_JoystickNumBalls;
  constexpr static auto& JoystickGetBall = SDL_JoystickGetBall;
  constexpr static auto& JoystickRumble = SDL_JoystickRumble;
  constexpr static auto& JoystickRumbleTriggers = SDL_JoystickRumbleTriggers;
  constexpr static auto& JoystickSetLED = SDL_JoystickSetLED;
  constexpr static auto& JoystickUpdate = SDL_JoystickUpdate;

  SDL_JoystickType JoystickGetType(SDL_Joystick* joystick);
  void InitVirtualJoysticksFromKeyboardsAndMice();
  SDL_bool IsMouse(SDL_Joystick* joystick);
  SDL_bool IsKeyboard(SDL_Joystick* joystick);
  Uint16 JoystickGetVendor(SDL_Joystick* joystick);
  Uint16 JoystickGetProduct(SDL_Joystick* joystick);
  Uint16 JoystickGetProductVersion(SDL_Joystick* joystick);

  void SaveDeviceToFile(SDL_Joystick* joystick, int device_index);
  nlohmann::ordered_json joystick_to_json(SDL_Joystick* joystick, int device_index);
  void SaveAllDevicesToFile();

  inline auto to_string(const SDL_JoystickGUID& guid)
  {
    std::string result(33, '\0');

    SDL_JoystickGetGUIDString(guid, result.data(), int(result.size()));

    result.pop_back();

    return result;
  }

  inline auto to_string(SDL_JoystickType type)
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

  inline auto to_string(SDL_GameControllerType type)
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

  inline const char* GameControllerGetStringForButton(SDL_GameController* controller, SDL_GameControllerButton button)
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

  template<typename Integral>
  inline const char* to_hex(Integral value)
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
}

#endif// OPEN_SIEGE_PLATFORM_HPP
