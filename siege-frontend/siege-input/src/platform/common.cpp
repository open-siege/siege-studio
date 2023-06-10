#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>
#include "platform/platform.hpp"

using json =  nlohmann::ordered_json;

namespace siege
{
  auto to_string(SDL_GameControllerBindType bind_type)
  {
    constexpr static std::array<const char*, 4> names = {
      "none",
      "button",
      "axis",
      "hat"
    };

    auto type_index = std::size_t(bind_type);

    return type_index < names.size() ? names[type_index] : "";
  }

  auto to_json(const SDL_GameControllerButtonBind& binding)
  {
    static_assert(std::is_same_v<decltype(binding.value.button), decltype(binding.value.axis)>);
    static_assert(std::is_same_v<decltype(binding.value.button), decltype(binding.value.hat.hat)>);

    return json{
      { "type", to_string(binding.bindType) },
      { "index", binding.value.button }
    };
  }

  json joystick_to_json(SDL_Joystick* joystick, int device_index)
  {
    json result;

    result["fileVersion"] = 1;
    result["fileType"] = "deviceInfo";
    result["deviceName"] = SDL_JoystickName(joystick) ? SDL_JoystickName(joystick) : "";
    result["deviceGuid"] = to_string(SDL_JoystickGetGUID(joystick));
    result["vendorId"] = to_hex(siege::JoystickGetVendor(joystick));
    result["productId"] = to_hex(siege::JoystickGetProduct(joystick));
    result["productVersion"] = siege::JoystickGetProductVersion(joystick);
    result["serialNumber"] = SDL_JoystickGetSerial(joystick) ? SDL_JoystickGetSerial(joystick) : "";

    if (IsMouse(joystick))
    {
      result["detectedDeviceType"] = "Mouse";
    }
    else
    {
      result["detectedDeviceType"] = to_string(JoystickGetType(joystick));
    }

    if (SDL_IsGameController(device_index))
    {
      result["detectedControllerType"] = to_string(SDL_GameControllerTypeForIndex(device_index));
    }

    result["numButtons"] = SDL_JoystickNumButtons(joystick);
    result["numHats"] = SDL_JoystickNumHats(joystick);
    result["numAxes"] = SDL_JoystickNumAxes(joystick);
    result["numBalls"] = SDL_JoystickNumBalls(joystick);
    result["hasRumble"] = SDL_JoystickHasRumble(joystick) == SDL_bool::SDL_TRUE;
    result["hasTriggerRumble"] = SDL_JoystickHasRumbleTriggers(joystick) == SDL_bool::SDL_TRUE;

    if (SDL_IsGameController(device_index))
    {
      auto controller = std::shared_ptr<SDL_GameController>(SDL_GameControllerOpen(device_index), SDL_GameControllerClose);

      result["hasLed"] = SDL_GameControllerHasLED(controller.get()) == SDL_bool::SDL_TRUE;
      result["numTouchpads"] = SDL_GameControllerGetNumTouchpads(controller.get());

      std::vector<int> touch_pad_fingers(SDL_GameControllerGetNumTouchpads(controller.get()));

      for (auto i = 0; i < touch_pad_fingers.size(); ++i)
      {
        touch_pad_fingers[i] = SDL_GameControllerGetNumTouchpadFingers(controller.get(), i);
      }

      if (!touch_pad_fingers.empty())
      {
        result["touchpadFingerCount"] = touch_pad_fingers;
      }

      bool hasAccelerometer = SDL_GameControllerHasSensor(controller.get(), SDL_SensorType::SDL_SENSOR_ACCEL) == SDL_bool::SDL_TRUE;
      bool hasGyroscope = SDL_GameControllerHasSensor(controller.get(), SDL_SensorType::SDL_SENSOR_GYRO) == SDL_bool::SDL_TRUE;
      result["hasAccelerometer"] = hasAccelerometer;
      result["hasGyroscope"] = hasGyroscope;

      if (hasAccelerometer)
      {
        result["sensorDataRates"]["accelerometer"] = SDL_GameControllerGetSensorDataRate(controller.get(), SDL_SensorType::SDL_SENSOR_ACCEL);
      }

      if (hasGyroscope)
      {
        result["sensorDataRates"]["gyroscope"] = SDL_GameControllerGetSensorDataRate(controller.get(), SDL_SensorType::SDL_SENSOR_GYRO);
      }

      result["bindings"] = {};

      for (auto i = 0; i < int(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_MAX); ++i)
      {
        auto axis_type = SDL_GameControllerAxis(i);
        auto binding = SDL_GameControllerGetBindForAxis(controller.get(), axis_type);

        if (axis_type == SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX)
        {
          result["bindings"]["axes"]["leftStick"]["x"] = to_json(binding);
        }

        if (axis_type == SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY)
        {
          result["bindings"]["axes"]["leftStick"]["y"] = to_json(binding);
        }

        if (axis_type == SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX)
        {
          result["bindings"]["axes"]["rightStick"]["x"] = to_json(binding);
        }

        if (axis_type == SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY)
        {
          result["bindings"]["axes"]["rightStick"]["y"] = to_json(binding);
        }

        if (axis_type == SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT)
        {
          result["bindings"]["axes"]["leftTrigger"] = to_json(binding);
        }

        if (axis_type == SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        {
          result["bindings"]["axes"]["rightTrigger"] = to_json(binding);
        }
      }

      for (auto i = 0; i < int(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MAX); ++i)
      {
        auto button_type = SDL_GameControllerButton(i);
        auto binding = SDL_GameControllerGetBindForButton(controller.get(), button_type);
        result["bindings"]["buttons"][GameControllerGetStringForButton(controller.get(), button_type)] = to_json(binding);
      }
    }

    return result;
  }

  void open_file_explorer()
  {
    static bool has_opened = false;
    // TODO get rid of system specific code

    if (!has_opened)
    {
      std::system("explorer .");
      has_opened = true;
    }
  }

  std::string joystick_name(SDL_Joystick* joystick)
  {
    auto* joystick_name = SDL_JoystickName(joystick) ? SDL_JoystickName(joystick) : "";

    std::stringstream new_name;
    new_name << joystick_name << "-" << JoystickGetVendor(joystick) << "-" << JoystickGetProduct(joystick);
    return new_name.str();
  }


  void SaveDeviceToFile(SDL_Joystick* joystick, int device_index)
  {
    auto filename = joystick_name(joystick) + ".info.json";

    std::ofstream output(filename, std::ios::binary);

    auto data = joystick_to_json(joystick, device_index);

    output << data.dump(4);

    open_file_explorer();
  }

  void SaveAllDevicesToFile()
  {
    auto filename = "all-devices.info.json";

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
    output << result.dump(4);

    open_file_explorer();
  }
}
