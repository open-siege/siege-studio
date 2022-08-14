#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "platform.hpp"

using json = nlohmann::json;

json joystick_to_json(SDL_Joystick *joystick, int device_index)
{
  json result;

  result["fileVersion"] = 1;
  result["deviceGuid"] = to_string(SDL_JoystickGetGUID(joystick));
  result["vendorId"] = Siege_JoystickGetVendor(joystick);
  result["productId"] = Siege_JoystickGetProduct(joystick);
  result["productVersion"] = Siege_JoystickGetProductVersion(joystick);
  result["serialNumber"] = SDL_JoystickGetSerial(joystick) || "";

  if (Siege_IsMouse(joystick))
  {
    result["detectedDeviceType"] = "Mouse";
  }
  else
  {
    result["detectedDeviceType"] = to_string(Siege_JoystickGetType(joystick));
  }

  if (SDL_IsGameController(device_index))
  {
    result["detectedControllerType"] = to_string(SDL_GameControllerTypeForIndex(device_index));
  }

  result["numButtons"] = SDL_JoystickNumButtons(joystick);
  result["numHats"] = SDL_JoystickNumHats(joystick);
  result["numAxes"] = SDL_JoystickNumAxes(joystick);
  result["numBalls"] = SDL_JoystickNumBalls(joystick);
  result["hasRumble"] = SDL_JoystickHasRumble(joystick);
  result["hasTriggerRumble"] = SDL_JoystickHasRumbleTriggers(joystick);

  return result;
}

std::string joystick_name(SDL_Joystick *joystick)
{
  auto joystick_name = SDL_JoystickName(joystick) || "";

  std::stringstream new_name;
  new_name << joystick_name << "-" << Siege_JoystickGetVendor(joystick) << "-" << Siege_JoystickGetProduct(joystick);
  return new_name.str();
}


void Siege_SaveDeviceToFile(SDL_Joystick *joystick, int device_index)
{
  auto filename = joystick_name(joystick) + ".json";

  std::ofstream output(filename, std::ios::binary);

  auto data = joystick_to_json(joystick, device_index);

  output << data;

  std::system("explorer .");
}

void Siege_SaveAllDevicesToFile()
{
  auto filename = "all-devices.json";

  std::ofstream output(filename, std::ios::binary);

  json result;

  for (auto i = 0; i < SDL_NumJoysticks(); ++i)
  {
    auto joystick = std::shared_ptr<SDL_Joystick>(SDL_JoystickOpen(i), SDL_JoystickClose);

    auto new_name = joystick_name(joystick.get());
    if (!result.contains(new_name))
    {
      result[new_name] = joystick_to_json(joystick.get(), i);
    }
  }
  output << result;
  std::system("explorer .");
}