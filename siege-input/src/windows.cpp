#include <array>
#include <string_view>
#include <limits>
#include <unordered_map>
#include <memory>
#include <SDL.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CINTERFACE
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>

SDL_JoystickType SDLCALL Siege_JoystickGetType(SDL_Joystick* joystick)
{
  auto type = SDL_JoystickGetType(joystick);

  if (type == SDL_JoystickType::SDL_JOYSTICK_TYPE_UNKNOWN)
  {
    static std::unordered_map<std::basic_string<std::uint16_t>, SDL_JoystickType> cached_types;

    auto vendor_product = std::basic_string<std::uint16_t>{SDL_JoystickGetVendor(joystick), SDL_JoystickGetProduct(joystick)};


    auto existing = cached_types.find(vendor_product);

    if (existing != cached_types.end())
    {
      return existing->second;
    }

    static std::shared_ptr<IDirectInput8A> dinput;

    if (!dinput)
    {
      IDirectInput8A* instance;
      auto result = DirectInput8Create(GetModuleHandleA(nullptr),
        DIRECTINPUT_VERSION,
        IID_IDirectInput8A,
        reinterpret_cast<void**>(&instance),
        nullptr);

      if (result == DI_OK)
      {
        dinput.reset(instance, instance->lpVtbl->Release);
      }
    }

    if (!dinput)
    {
      return type;
    }

    dinput->lpVtbl->EnumDevices(
      dinput.get(), DI8DEVCLASS_ALL, [](const DIDEVICEINSTANCEA* device_instance, void* raw_joystick) -> BOOL {

        std::shared_ptr<IDirectInputDevice8A> device;

        {
          IDirectInputDevice8A* temp;

          if (dinput->lpVtbl->CreateDevice(dinput.get(), device_instance->guidInstance, &temp, nullptr) != DI_OK)
          {
            return DIENUM_CONTINUE;
          }

          device.reset(temp, temp->lpVtbl->Release);
        }

        DIPROPDWORD hardware_info{};

        hardware_info.diph.dwSize = sizeof(hardware_info);
        hardware_info.diph.dwHeaderSize = sizeof(hardware_info.diph);
        hardware_info.diph.dwHow = DIPH_DEVICE;

        if (device->lpVtbl->GetProperty(device.get(), DIPROP_VIDPID, &hardware_info.diph) != DI_OK) {
            return DIENUM_CONTINUE;
        }

        auto* real_joystick = reinterpret_cast<SDL_Joystick*>(raw_joystick);
        auto vendor_id = SDL_JoystickGetVendor(real_joystick);
        auto product_id = SDL_JoystickGetProduct(real_joystick);

        if (vendor_id == LOWORD(hardware_info.dwData) && product_id == HIWORD(hardware_info.dwData))
        {
          auto vendor_product = std::basic_string<std::uint16_t>{SDL_JoystickGetVendor(real_joystick), SDL_JoystickGetProduct(real_joystick)};
          auto mainType = device_instance->dwDevType & std::numeric_limits<std::uint8_t>::max();
          auto subType = device_instance->dwDevType & std::numeric_limits<std::uint16_t>::max();

          if (mainType == DI8DEVTYPE_GAMEPAD)
          {
            cached_types.emplace(vendor_product, SDL_JoystickType::SDL_JOYSTICK_TYPE_GAMECONTROLLER);
          }
          else if (mainType == DI8DEVTYPE_DRIVING)
          {
            cached_types.emplace(vendor_product, SDL_JoystickType::SDL_JOYSTICK_TYPE_WHEEL);
          }
          else if (mainType == DI8DEVTYPE_JOYSTICK || mainType == DI8DEVTYPE_FLIGHT)
          {
            if (SDL_JoystickNumAxes(real_joystick) == 0)
            {
              cached_types.emplace(vendor_product, SDL_JoystickType::SDL_JOYSTICK_TYPE_ARCADE_STICK);
            }
            else
            {
              auto device_name = std::string_view(SDL_JoystickName(real_joystick));

              if (device_name.find("throttle") != std::string::npos || device_name.find("Throttle") != std::string::npos)
              {
                cached_types.emplace(vendor_product, SDL_JoystickType::SDL_JOYSTICK_TYPE_THROTTLE);
              }
              else
              {
                cached_types.emplace(vendor_product, SDL_JoystickType::SDL_JOYSTICK_TYPE_FLIGHT_STICK);
              }

            }
          }
          else if (mainType == DI8DEVTYPE_SUPPLEMENTAL)
          {
            if (subType == DI8DEVTYPESUPPLEMENTAL_THROTTLE || subType == DI8DEVTYPESUPPLEMENTAL_SPLITTHROTTLE)
            {
              cached_types.emplace(vendor_product, SDL_JoystickType::SDL_JOYSTICK_TYPE_THROTTLE);
            }
          }
          return DIENUM_STOP;
        }

        return DIENUM_CONTINUE;
      },
      joystick,
      DIEDFL_ALLDEVICES);

    existing = cached_types.find(vendor_product);

    if (existing != cached_types.end())
    {
      return existing->second;
    }
  }



  return type;
}