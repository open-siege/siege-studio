#ifndef OPEN_SIEGE_PLATFORM_HPP
#define OPEN_SIEGE_PLATFORM_HPP

#include <SDL.h>

SDL_JoystickType Siege_JoystickGetType(SDL_Joystick* joystick);

void Siege_InitVirtualJoysticks();
SDL_bool Siege_IsMouse(SDL_Joystick* joystick);
Uint16 Siege_JoystickGetVendor(SDL_Joystick* joystick);
Uint16 Siege_JoystickGetProduct(SDL_Joystick* joystick);
Uint16 Siege_JoystickGetProductVersion(SDL_Joystick* joystick);

void Siege_SaveDeviceToFile(SDL_Joystick* joystick, int device_index);
void Siege_SaveAllDevicesToFile();

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

inline const char* Siege_GameControllerGetStringForButton(SDL_GameController* controller, SDL_GameControllerButton button)
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

#endif// OPEN_SIEGE_PLATFORM_HPP
