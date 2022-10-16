#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CINTERFACE
#include <windows.h>
#include <dinput.h>

#include <vector>
#include <memory>
#include <platform/platform.hpp>

namespace dinput
{
  enum class create_result
  {
    ok = DI_OK,
    unsupported_beta = DIERR_BETADIRECTINPUTVERSION,
    invalid_param = DIERR_INVALIDPARAM,
    version_too_old = DIERR_OLDDIRECTINPUTVERSION,
    out_of_memory = DIERR_OUTOFMEMORY
  };

  namespace core::v1
  {
    HRESULT STDMETHODCALLTYPE QueryInterface(IDirectInputA*, REFIID, void**);
    ULONG STDMETHODCALLTYPE AddRef(IDirectInputA*);
    ULONG STDMETHODCALLTYPE Release(IDirectInputA*);

    HRESULT STDMETHODCALLTYPE DarkCreateDevice(IDirectInputA*, const GUID&, IDirectInputDeviceA**, IUnknown*);
    HRESULT STDMETHODCALLTYPE DarkEnumDevices(IDirectInputA*, DWORD, LPDIENUMDEVICESCALLBACKA, void*, DWORD);
    HRESULT STDMETHODCALLTYPE DarkGetDeviceStatus(IDirectInputA*, const GUID&);
    HRESULT STDMETHODCALLTYPE DarkRunControlPanel(IDirectInputA*, HWND, DWORD);
    HRESULT STDMETHODCALLTYPE DarkInitialize(IDirectInputA*, HINSTANCE, DWORD);

    static IDirectInputAVtbl dinput_vtable = {
      core::v1::QueryInterface,
      core::v1::AddRef,
      core::v1::Release,
      core::v1::DarkCreateDevice,
      core::v1::DarkEnumDevices,
      core::v1::DarkGetDeviceStatus,
      core::v1::DarkRunControlPanel,
      core::v1::DarkInitialize
    };
  }

  create_result WINAPI DarkDirectInputCreateA(HINSTANCE appInstance, DWORD version, IDirectInputA** output, LPUNKNOWN)
  {

    static IDirectInputA instance{ &core::v1::dinput_vtable };

    *output = &instance;

    return create_result::ok;
  }

  namespace device::v1
  {
    HRESULT STDMETHODCALLTYPE QueryInterface(IDirectInputDeviceA*, REFIID, void**);
    ULONG STDMETHODCALLTYPE AddRef(IDirectInputDeviceA*);
    ULONG STDMETHODCALLTYPE Release(IDirectInputDeviceA*);

    HRESULT STDMETHODCALLTYPE GetCapabilities(IDirectInputDeviceA*, DIDEVCAPS*);
    HRESULT STDMETHODCALLTYPE EnumObjects(IDirectInputDeviceA*, LPDIENUMDEVICEOBJECTSCALLBACKA, void*, DWORD);
    HRESULT STDMETHODCALLTYPE GetProperty(IDirectInputDeviceA*, const GUID&, LPDIPROPHEADER);
    HRESULT STDMETHODCALLTYPE SetProperty(IDirectInputDeviceA*, const GUID&, LPCDIPROPHEADER);
    HRESULT STDMETHODCALLTYPE Acquire(IDirectInputDeviceA*);
    HRESULT STDMETHODCALLTYPE Unacquire(IDirectInputDeviceA*);
    HRESULT STDMETHODCALLTYPE GetDeviceState(IDirectInputDeviceA*, DWORD, LPVOID);
    HRESULT STDMETHODCALLTYPE GetDeviceData(IDirectInputDeviceA*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
    HRESULT STDMETHODCALLTYPE SetDataFormat(IDirectInputDeviceA*, LPCDIDATAFORMAT);
    HRESULT STDMETHODCALLTYPE SetEventNotification(IDirectInputDeviceA*, HANDLE);
    HRESULT STDMETHODCALLTYPE SetCooperativeLevel(IDirectInputDeviceA*, HWND, DWORD); // maybe used
    HRESULT STDMETHODCALLTYPE GetObjectInfo(IDirectInputDeviceA*, LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD);
    HRESULT STDMETHODCALLTYPE GetDeviceInfo(IDirectInputDeviceA*, LPDIDEVICEINSTANCEA);
    HRESULT STDMETHODCALLTYPE RunControlPanel(IDirectInputDeviceA*, HWND, DWORD);
    HRESULT STDMETHODCALLTYPE Initialize(IDirectInputDeviceA*, HINSTANCE, DWORD, const GUID&);

    static IDirectInputDeviceAVtbl device_vtable = {
      QueryInterface,
      AddRef,
      Release,
      GetCapabilities,
      EnumObjects,
      GetProperty,
      SetProperty,
      Acquire,
      Unacquire,
      GetDeviceState,
      GetDeviceData,
      SetDataFormat,
      SetEventNotification,
      SetCooperativeLevel,
      GetObjectInfo,
      GetDeviceInfo,
      RunControlPanel,
      Initialize
    };
  }

  namespace device::v2
  {
    HRESULT STDMETHODCALLTYPE DarkCreateEffect(IDirectInputDevice2A*, const GUID&, const DIEFFECT*, IDirectInputEffect**, IUnknown*);
    HRESULT STDMETHODCALLTYPE DarkEnumEffects(IDirectInputDevice2A*, LPDIENUMEFFECTSCALLBACKA callback, LPVOID data, DWORD filter);
    HRESULT STDMETHODCALLTYPE DarkGetEffectInfo(IDirectInputDevice2A*, DIEFFECTINFOA* info, const GUID&);
    HRESULT STDMETHODCALLTYPE DarkGetForceFeedbackState(IDirectInputDevice2A*, DWORD* value);
    HRESULT STDMETHODCALLTYPE DarkSendForceFeedbackCommand(IDirectInputDevice2A*, DWORD flags);
    HRESULT STDMETHODCALLTYPE DarkEnumCreatedEffectObjects(IDirectInputDevice2A*, LPDIENUMCREATEDEFFECTOBJECTSCALLBACK callback, void* data, DWORD);
    HRESULT STDMETHODCALLTYPE DarkEscape(IDirectInputDevice2A*, DIEFFESCAPE* command);
    HRESULT STDMETHODCALLTYPE DarkPoll(IDirectInputDevice2A*);
    HRESULT STDMETHODCALLTYPE DarkSendDeviceData(IDirectInputDevice2A*, DWORD size, const DIDEVICEOBJECTDATA* data, DWORD* length, DWORD flags);

    static IDirectInputDevice2AVtbl device_vtable = ([](){
      IDirectInputDevice2AVtbl temp{};
      std::memcpy(&temp, &device::v1::device_vtable, sizeof(device::v1::device_vtable));
      temp.CreateEffect = DarkCreateEffect;
      temp.EnumEffects = DarkEnumEffects;
      temp.GetEffectInfo = DarkGetEffectInfo;
      temp.GetForceFeedbackState = DarkGetForceFeedbackState;
      temp.SendForceFeedbackCommand = DarkSendForceFeedbackCommand;
      temp.EnumCreatedEffectObjects = DarkEnumCreatedEffectObjects;
      temp.Escape = DarkEscape;
      temp.Poll = DarkPoll;
      temp.SendDeviceData = DarkSendDeviceData;

      return temp;
    })();
  }

  namespace device::v7
  {
    HRESULT STDMETHODCALLTYPE DarkEnumEffectsInFile(IDirectInputDevice7A*, LPCSTR fileName, LPDIENUMEFFECTSINFILECALLBACK callback, void* data, DWORD flags);

    HRESULT STDMETHODCALLTYPE DarkWriteEffectToFile(IDirectInputDevice7A*, LPCSTR filename, DWORD, DIFILEEFFECT* effect, DWORD flags);

    static IDirectInputDevice7AVtbl device_vtable = ([](){
      IDirectInputDevice7AVtbl temp{};
      std::memcpy(&temp, &device::v2::device_vtable, sizeof(device::v2::device_vtable));
      temp.EnumEffectsInFile = DarkEnumEffectsInFile;
      temp.WriteEffectToFile = DarkWriteEffectToFile;

      return temp;
    })();
  }

  struct device_info
  {
    IDirectInputDeviceA device;
    std::unique_ptr<SDL_Joystick, void(*)(SDL_Joystick*)> joystick;
  };

  static std::vector<device_info> devices;

  namespace device::v1
  {
    template<typename device_type = IDirectInputDeviceA>
    HRESULT STDMETHODCALLTYPE DarkGetCapabilities(device_type* self, DIDEVCAPS* caps) // maybe used
    {
      auto size = caps->dwSize;

      auto info = std::find_if(devices.begin(), devices.end(), [self](const auto& value) { return &value.device == self; });

      if (info == devices.end())
      {
        return DIERR_INVALIDPARAM;
      }

      caps->dwFlags = DIDC_ATTACHED;
      caps->dwDevType = DI8DEVTYPE_FLIGHT;
      caps->dwAxes = SDL_JoystickNumAxes(info->joystick.get());
      caps->dwButtons = SDL_JoystickNumButtons(info->joystick.get());
      caps->dwPOVs = SDL_JoystickNumHats(info->joystick.get());

      if (size > sizeof(DIDEVCAPS_DX3))
      {

      }
      return 0;
    }

    auto GetAxisObjectType(int num_axes, int index)
    {
      if (index == 0)
      {
        return GUID_XAxis;
      }

      if (index == 1)
      {
        return GUID_YAxis;
      }

      if (num_axes == 3 && index == 2)
      {
        return GUID_ZAxis;
      }
      else if (num_axes >= 4 && index == 2)
      {
        return GUID_RzAxis;
      }

      if (num_axes >= 4 && index == 3)
      {
        return GUID_ZAxis;
      }

      if (index == 4)
      {
        return GUID_RxAxis;
      }

      if (index == 5)
      {
        return GUID_RyAxis;
      }

      return GUID_Slider;
    }

    std::string_view GetAxisName(const GUID& axis_type)
    {
      const static auto names = std::array<std::pair<GUID, std::string_view>, 7> {{
        std::make_pair(GUID_XAxis, "X-Axis"),
        std::make_pair(GUID_YAxis, "Y-Axis"),
        std::make_pair(GUID_ZAxis, "Throttle"),
        std::make_pair(GUID_RxAxis, "X-Axis Right"),
        std::make_pair(GUID_RyAxis, "Y-Axis Right"),
        std::make_pair(GUID_RzAxis, "Rudder"),
        std::make_pair(GUID_Slider, "Slider"),
        }};

      auto name = std::find_if(names.begin(), names.end(), [&](const auto& value) {
        return value.first == axis_type;
      });

      if (name == names.end())
      {
        return "";
      }

      return name->second;
    }



    HRESULT STDMETHODCALLTYPE DarkEnumObjects(IDirectInputDeviceA* self, LPDIENUMDEVICEOBJECTSCALLBACKA callback, void* data, DWORD flags)
    {
      auto info = std::find_if(devices.begin(), devices.end(), [self](const auto& value) { return &value.device == self; });

      if (info == devices.end())
      {
        return DIERR_INVALIDPARAM;
      }

      for (auto i = 0; i < SDL_JoystickNumAxes(info->joystick.get()); ++i)
      {
        DIDEVICEOBJECTINSTANCEA object{};
        object.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
        object.guidType = GetAxisObjectType(SDL_JoystickNumAxes(info->joystick.get()), i);
        object.dwType = DIDFT_AXIS;

        auto name = GetAxisName(object.guidType);
        std::memcpy(&object.tszName, name.data(), name.size());

        auto result = callback(&object, data);

        if (result == FALSE)
        {
          return 0;
        }
      }

      for (auto i = 0; i < SDL_JoystickNumHats(info->joystick.get()); ++i)
      {
        DIDEVICEOBJECTINSTANCEA object{};
        object.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
        object.guidType = GUID_POV;
        object.dwType = DIDFT_POV;

        auto name = "Hat " + std::to_string(i + 1);
        std::memcpy(&object.tszName, name.data(), name.size());

        auto result = callback(&object, data);

        if (result == FALSE)
        {
          return 0;
        }
      }

      for (auto i = 0; i < SDL_JoystickNumButtons(info->joystick.get()); ++i)
      {
        DIDEVICEOBJECTINSTANCEA object{};
        object.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
        object.guidType = GUID_Button;
        object.dwType = DIDFT_BUTTON;
        auto name = "Button " + std::to_string(i + 1);
        std::memcpy(&object.tszName, name.data(), name.size());

        auto result = callback(&object, data);

        if (result == FALSE)
        {
          return 0;
        }
      }

      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkGetProperty(IDirectInputDeviceA* self, const GUID& prop_id, DIPROPHEADER* result) // maybe used
    {
      if (!result)
      {
        return DIERR_INVALIDPARAM;
      }

      auto info = std::find_if(devices.begin(), devices.end(), [self](const auto& value) { return &value.device == self; });

      if (info == devices.end())
      {
        return DIERR_INVALIDPARAM;
      }

      result->dwSize = sizeof(DIPROPHEADER);
      result->dwHeaderSize = sizeof(DIPROPHEADER);
      result->dwObj = 0;
      result->dwHow = 0;

      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkSetProperty(IDirectInputDeviceA*, const GUID& prop_id, DIPROPHEADER* result) // maybe used
    {
      if (!result)
      {
        return DIERR_INVALIDPARAM;
      }
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkAcquire(IDirectInputDeviceA*) // maybe used
    {
      SDL_JoystickOpen(0);
      return 0;
    }


    HRESULT STDMETHODCALLTYPE DarkUnacquire(IDirectInputDeviceA*)  // maybe used
    {
      SDL_JoystickClose(nullptr);
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkSetDataFormat(IDirectInputDeviceA*, DIDATAFORMAT*) // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkGetDeviceState(IDirectInputDeviceA*, DWORD, LPVOID) // maybe used
    {

      DIJOYSTATE joy_state{};

      //stick
      joy_state.lX = SDL_JoystickGetAxis(nullptr, 0);
      joy_state.lY = SDL_JoystickGetAxis(nullptr, 1);

      // rudder
      joy_state.lRz = SDL_JoystickGetAxis(nullptr, 2);

      // throttle
      joy_state.lZ = SDL_JoystickGetAxis(nullptr, 3);

      joy_state.lRx = SDL_JoystickGetAxis(nullptr, 4);
      joy_state.lRy = SDL_JoystickGetAxis(nullptr, 5);

      joy_state.rglSlider[0] = SDL_JoystickGetAxis(nullptr, 6);
      joy_state.rglSlider[1] = SDL_JoystickGetAxis(nullptr, 7);

      joy_state.rgdwPOV[0] = SDL_JoystickGetHat(nullptr, 0);
      joy_state.rgdwPOV[1] = SDL_JoystickGetHat(nullptr, 1);
      joy_state.rgdwPOV[2] = SDL_JoystickGetHat(nullptr, 2);
      joy_state.rgdwPOV[3] = SDL_JoystickGetHat(nullptr, 3);

      for (auto i = 0; i < 32; ++i)
      {
        joy_state.rgbButtons[i] = SDL_JoystickGetButton(nullptr, 0);
      }

      DIJOYSTATE2 joy_state2{};

      //stick
      joy_state2.lX = SDL_JoystickGetAxis(nullptr, 0);
      joy_state2.lY = SDL_JoystickGetAxis(nullptr, 1);

      // rudder
      joy_state2.lRz = SDL_JoystickGetAxis(nullptr, 2);

      // throttle
      joy_state2.lZ = SDL_JoystickGetAxis(nullptr, 3);

      joy_state2.lRx = SDL_JoystickGetAxis(nullptr, 4);
      joy_state2.lRy = SDL_JoystickGetAxis(nullptr, 5);

      joy_state2.rglSlider[0] = SDL_JoystickGetAxis(nullptr, 6);
      joy_state2.rglSlider[1] = SDL_JoystickGetAxis(nullptr, 7);

      joy_state2.rgdwPOV[0] = SDL_JoystickGetHat(nullptr, 0);
      joy_state2.rgdwPOV[1] = SDL_JoystickGetHat(nullptr, 1);
      joy_state2.rgdwPOV[2] = SDL_JoystickGetHat(nullptr, 2);
      joy_state2.rgdwPOV[3] = SDL_JoystickGetHat(nullptr, 3);

      for (auto i = 0; i < 32; ++i)
      {
        joy_state2.rgbButtons[i] = SDL_JoystickGetButton(nullptr, 0);
      }

      return 0;
    }


    HRESULT STDMETHODCALLTYPE DarkGetDeviceData(IDirectInputDeviceA*, DWORD size, DIDEVICEOBJECTDATA*, DWORD* length, DWORD flags) // maybe used
    {
      DIDEVICEOBJECTDATA data{};
      data.dwOfs = DIJOFS_BUTTON0;
      data.dwData = 0;
      data.dwSequence = 0;
      data.uAppData = 0;

      return 0;
    }


    HRESULT STDMETHODCALLTYPE DarkSetEventNotification(IDirectInputDeviceA*, HANDLE)
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE DarkSetCooperativeLevel(IDirectInputDeviceA*, HWND, DWORD) // maybe used
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE DarkGetObjectInfo(IDirectInputDeviceA*, DIDEVICEOBJECTINSTANCEA* info, DWORD value, DWORD how)
    {
      info->dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
      info->guidType = GUID_XAxis;
      info->dwType = DIDFT_AXIS;
      //object.tszName = "X-Axis";
      return 0;
    }


    HRESULT STDMETHODCALLTYPE DarkGetDeviceInfo(IDirectInputDeviceA*, DIDEVICEINSTANCEA* info)
    {
      info->dwSize = sizeof(DIDEVICEINSTANCEA);
      info->dwDevType = DI8DEVTYPE_FLIGHT;
      return 0;
    }


    HRESULT STDMETHODCALLTYPE DarkRunControlPanel(IDirectInputDeviceA*, HWND, DWORD)
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE DarkInitialize(IDirectInputDeviceA*, HINSTANCE, DWORD version)
    {
      return 0;
    }
  }

  namespace device::v2
  {
    HRESULT STDMETHODCALLTYPE DarkCreateEffect(IDirectInputDevice2A*, const GUID&, const DIEFFECT*, IDirectInputEffect**, IUnknown*)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkEnumEffects(IDirectInputDevice2A*, LPDIENUMEFFECTSCALLBACKA callback, LPVOID data, DWORD filter)
    {
      DIEFFECTINFOA effect{};

      effect.dwSize = sizeof(DIEFFECTINFOA);
      effect.dwEffType = DIEFT_CONDITION;
      effect.dwStaticParams = DIEP_ENVELOPE;
      effect.dwDynamicParams = DIEP_ENVELOPE;
      //effect.tszName = "Vibration";

      auto result = callback(&effect, data);

      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkGetEffectInfo(IDirectInputDevice2A*, DIEFFECTINFOA* info, const GUID&)
    {
      info->dwSize = sizeof(DIEFFECTINFOA);
      info->dwEffType = DIEFT_CONDITION;
      info->dwStaticParams = DIEP_ENVELOPE;
      info->dwDynamicParams = DIEP_ENVELOPE;
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkGetForceFeedbackState(IDirectInputDevice2A*, DWORD* value)
    {
      *value = DIGFFS_STOPPED;
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkSendForceFeedbackCommand(IDirectInputDevice2A*, DWORD flags)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkEnumCreatedEffectObjects(IDirectInputDevice2A*, LPDIENUMCREATEDEFFECTOBJECTSCALLBACK callback, void* data, DWORD)
    {
      IDirectInputEffect* effect = nullptr;

      auto result = callback(effect, data);
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkEscape(IDirectInputDevice2A*, DIEFFESCAPE* command)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkPoll(IDirectInputDevice2A*)  // maybe used
    {
      SDL_JoystickUpdate();
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkSendDeviceData(IDirectInputDevice2A*, DWORD size, const DIDEVICEOBJECTDATA* data, DWORD* length, DWORD flags)
    {
      return 0;
    }
  }

  namespace device::v7
  {
    HRESULT STDMETHODCALLTYPE DarkEnumEffectsInFile(IDirectInputDevice7A*, LPCSTR fileName, LPDIENUMEFFECTSINFILECALLBACK callback, void* data, DWORD flags)
    {
      DIFILEEFFECT effect{};

      auto result = callback(&effect, data);
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkWriteEffectToFile(IDirectInputDevice7A*, LPCSTR filename, DWORD, DIFILEEFFECT* effect, DWORD flags)
    {
      return 0;
    }
  }

  namespace core::v1
  {

    auto sdl_to_dinput_type(SDL_JoystickType sdl_type)
    {
      return DI8DEVTYPE_FLIGHT;
    }

    HRESULT STDMETHODCALLTYPE DarkEnumDevices(IDirectInputA* me, DWORD typeFilter, LPDIENUMDEVICESCALLBACKA callback, void* data, DWORD flags)
    {
      for (auto i = 0; i < SDL_NumJoysticks(); ++i)
      {
        DIDEVICEINSTANCEA device{};

        device.dwSize = sizeof(DIDEVICEINSTANCEA);

        auto device_guid = SDL_JoystickGetDeviceGUID(i);
        static_assert(sizeof(device_guid) == sizeof(device.guidProduct));

        std::memcpy(&device.guidProduct, &device_guid, sizeof(device_guid));

        static_assert(sizeof(i) <= sizeof(device.guidInstance));

        std::memcpy(&device.guidInstance, &i, sizeof(i));

        device.dwDevType = sdl_to_dinput_type(SDL_JoystickGetDeviceType(i));

        auto instance_name = SDL_JoystickNameForIndex(i);
        auto length = std::strlen(instance_name);
        std::memcpy(device.tszInstanceName, instance_name, length);
        std::memcpy(device.tszProductName, instance_name, length);

        auto result = callback(&device, data);

        if (result == FALSE)
        {
          break;
        }
      }
      return 0;
    }


    HRESULT STDMETHODCALLTYPE DarkCreateDevice(IDirectInputA*, const GUID& deviceType, IDirectInputDeviceA** output, IUnknown*)
    {
      if (deviceType == GUID_SysKeyboard)
      {
        // TODO complete this later
        return 0;
      }

      if (deviceType == GUID_SysMouse)
      {
        // TODO complete this later
        return 0;
      }

      if (devices.empty())
      {
        for (auto i = 0; i < SDL_NumJoysticks(); ++i)
        {
          devices.emplace_back(device_info{ { &device::v1::device_vtable },
            { SDL_JoystickOpen(i), [](SDL_Joystick* joy){ SDL_JoystickClose(joy); } }});
        }
      }

      if (deviceType == GUID_Joystick && !devices.empty())
      {
        *output = &devices.rbegin()->device;
        return 0;
      }

      auto index = 0;

      std::memcpy(&index, &deviceType, sizeof(index));

      if (index < devices.size())
      {
        *output = &devices[index].device;
        return 0;
      }

      return DIERR_INVALIDPARAM;
    }

    HRESULT STDMETHODCALLTYPE DarkGetDeviceStatus(IDirectInputA*, const GUID&)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkRunControlPanel(IDirectInputA*, DWORD)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkInitialize(IDirectInputA*, HINSTANCE, DWORD)
    {
      return 0;
    }
  }

  namespace core::v2
  {
    HRESULT STDMETHODCALLTYPE DarkFindDevice(IDirectInput2A*, const GUID& deviceClass, LPCSTR deviceName, GUID* result)
    {
      if (!deviceName)
      {
        return DIERR_INVALIDPARAM;
      }

      if (!result)
      {
        return DIERR_INVALIDPARAM;
      }

      auto device = std::find_if(devices.begin(), devices.end(), [&](const auto& info) {
        return std::string_view(SDL_JoystickName(info.joystick.get())) == deviceName;
      });

      if (device != devices.end())
      {
        auto index = std::distance(devices.begin(), device);
        static_assert(sizeof(index) <= sizeof(GUID));
        std::memcpy(result, &index, sizeof(index));
      }

      return 0;
    }
  }

  namespace core::v7
  {
    HRESULT STDMETHODCALLTYPE DarkCreateDeviceEx(IDirectInput7A*,
      const GUID& deviceType,
      const IID& interfaceType,
      void** output,
      IUnknown*)
    {
      return core::v1::DarkCreateDevice(nullptr, deviceType, reinterpret_cast<IDirectInputDeviceA**>(output), nullptr);
    }
  }

  namespace effect
  {

  }
}
