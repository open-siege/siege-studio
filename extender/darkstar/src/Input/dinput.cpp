#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CINTERFACE
#include <windows.h>
#include <dinput.h>

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
  }

  create_result WINAPI DarkDirectInputCreateA(HINSTANCE appInstance, DWORD version, IDirectInputA*, LPUNKNOWN)
  {
    IDirectInputA* temp;

    temp->lpVtbl->QueryInterface = core::v1::QueryInterface;
    temp->lpVtbl->AddRef = core::v1::AddRef;
    temp->lpVtbl->Release = core::v1::Release;

    temp->lpVtbl->CreateDevice = core::v1::DarkCreateDevice;
    temp->lpVtbl->EnumDevices = core::v1::DarkEnumDevices;
    temp->lpVtbl->GetDeviceStatus = core::v1::DarkGetDeviceStatus;
    temp->lpVtbl->RunControlPanel = core::v1::DarkRunControlPanel;
    temp->lpVtbl->Initialize = core::v1::DarkInitialize;

    return create_result::ok;
  }

  namespace device::v1
  {
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
  }
  
  namespace device::v1
  {
    HRESULT STDMETHODCALLTYPE GetCapabilities(IDirectInputDeviceA*, DIDEVCAPS* caps) // maybe used
    {
      auto size = caps->dwSize;

      caps->dwFlags = DIDC_ATTACHED;
      caps->dwDevType = DI8DEVTYPE_FLIGHT;
      caps->dwAxes = SDL_JoystickNumAxes(nullptr);
      caps->dwButtons = SDL_JoystickNumButtons(nullptr);
      caps->dwPOVs = SDL_JoystickNumHats(nullptr);

      if (size > sizeof(DIDEVCAPS_DX3))
      {

      }
      return 0;
    }

    HRESULT STDMETHODCALLTYPE EnumObjects(IDirectInputDeviceA*, LPDIENUMDEVICEOBJECTSCALLBACKA callback, void* data, DWORD flags)
    {
      DIDEVICEOBJECTINSTANCEA object{};
      object.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
      object.guidType = GUID_XAxis;
      object.dwType = DIDFT_AXIS;
      //object.tszName = "X-Axis";

      auto result = callback(&object, data);

      return 0;
    }

    HRESULT STDMETHODCALLTYPE GetProperty(IDirectInputDeviceA*, const GUID& prop_id, DIPROPHEADER* result) // maybe used
    {
      if (!result)
      {
        return DIERR_INVALIDPARAM;
      }

      result->dwSize = sizeof(DIPROPHEADER);
      result->dwHeaderSize = sizeof(DIPROPHEADER);
      result->dwObj = 0;
      result->dwHow = 0;

      return 0;
    }

    HRESULT STDMETHODCALLTYPE SetProperty(IDirectInputDeviceA*, const GUID& prop_id, DIPROPHEADER* result) // maybe used
    {
      if (!result)
      {
        return DIERR_INVALIDPARAM;
      }
      return 0;
    }

    HRESULT STDMETHODCALLTYPE Acquire(IDirectInputDeviceA*) // maybe used
    {
      SDL_JoystickOpen(0);
      return 0;
    }


    HRESULT STDMETHODCALLTYPE Unacquire(IDirectInputDeviceA*)  // maybe used
    {
      SDL_JoystickClose(nullptr);
      return 0;
    }

    HRESULT STDMETHODCALLTYPE SetDataFormat(IDirectInputDeviceA*, DIDATAFORMAT*) // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE GetDeviceState(IDirectInputDeviceA*, DWORD, LPVOID) // maybe used
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


    HRESULT STDMETHODCALLTYPE GetDeviceData(IDirectInputDeviceA*, DWORD size, DIDEVICEOBJECTDATA*, DWORD* length, DWORD flags) // maybe used
    {
      DIDEVICEOBJECTDATA data{};
      data.dwOfs = DIJOFS_BUTTON0;
      data.dwData = 0;
      data.dwSequence = 0;
      data.uAppData = 0;

      return 0;
    }


    HRESULT STDMETHODCALLTYPE SetEventNotification(IDirectInputDeviceA*, HANDLE)
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE SetCooperativeLevel(IDirectInputDeviceA*, HWND, DWORD) // maybe used
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE GetObjectInfo(IDirectInputDeviceA*, DIDEVICEOBJECTINSTANCEA* info, DWORD value, DWORD how)
    {
      info->dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
      info->guidType = GUID_XAxis;
      info->dwType = DIDFT_AXIS;
      //object.tszName = "X-Axis";
      return 0;
    }


    HRESULT STDMETHODCALLTYPE GetDeviceInfo(IDirectInputDeviceA*, DIDEVICEINSTANCEA* info)
    {
      info->dwSize = sizeof(DIDEVICEINSTANCEA);
      info->dwDevType = DI8DEVTYPE_FLIGHT;
      return 0;
    }


    HRESULT STDMETHODCALLTYPE RunControlPanel(IDirectInputDeviceA*, HWND, DWORD)
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE Initialize(IDirectInputDeviceA*, HINSTANCE, DWORD version)
    {
      return 0;
    }
  }
  
  namespace device::v2
  {
    HRESULT STDMETHODCALLTYPE CreateEffect(IDirectInputDevice2A*, const GUID&, DIEFFECT*, IDirectInputEffect**, IUnknown*)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE EnumEffects(IDirectInputDevice2A*, LPDIENUMEFFECTSCALLBACKA callback, LPVOID data, DWORD filter)
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

    HRESULT STDMETHODCALLTYPE GetEffectInfo(IDirectInputDevice2A*, DIEFFECTINFOA* info, const GUID&)
    {
      info->dwSize = sizeof(DIEFFECTINFOA);
      info->dwEffType = DIEFT_CONDITION;
      info->dwStaticParams = DIEP_ENVELOPE;
      info->dwDynamicParams = DIEP_ENVELOPE;
      return 0;
    }

    HRESULT STDMETHODCALLTYPE GetForceFeedbackState(IDirectInputDevice2A*, DWORD* value)
    {
      *value = DIGFFS_STOPPED;
      return 0;
    }

    HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(IDirectInputDevice2A*, DWORD flags)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(IDirectInputDevice2A*, LPDIENUMCREATEDEFFECTOBJECTSCALLBACK callback, void* data, DWORD)
    {
      IDirectInputEffect* effect = nullptr;

      auto result = callback(effect, data);
      return 0;
    }

    HRESULT STDMETHODCALLTYPE Escape(IDirectInputDevice2A*, DIEFFESCAPE* command)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE Poll(IDirectInputDevice2A*)  // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE SendDeviceData(IDirectInputDevice2A*, DWORD size, DIDEVICEOBJECTDATA* data, DWORD* length, DWORD flags)
    {
      return 0;
    }
  }

  namespace device::v7
  {
    HRESULT STDMETHODCALLTYPE EnumEffectsInFile(IDirectInputDevice7A*, LPCSTR fileName, LPDIENUMEFFECTSINFILECALLBACK callback, void* data, DWORD flags)
    {
      DIFILEEFFECT effect{};

      auto result = callback(&effect, data);
      return 0;
    }

    HRESULT STDMETHODCALLTYPE WriteEffectToFile(IDirectInputDevice7A*, LPCSTR filename, DWORD, DIFILEEFFECT* effect, DWORD flags)
    {
      return 0;
    }
  }

  namespace core::v1
  {
    HRESULT STDMETHODCALLTYPE DarkCreateDevice(IDirectInputA*, const GUID& deviceType, IDirectInputDeviceA** output, IUnknown*)
    {
      IDirectInputDeviceA* result;

      *output = result;
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkEnumDevices(IDirectInputA*, DWORD typeFilter, LPDIENUMDEVICESCALLBACKA callback, void* data, DWORD flags)
    {
      DIDEVICEINSTANCEA device{};

      auto result = callback(&device, data);

      return 0;
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
      return 0;
    }
  }

  namespace effect
  {

  }
}
