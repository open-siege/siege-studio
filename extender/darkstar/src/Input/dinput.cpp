#include <windows.h>
#include <dinput.h>

namespace dinput
{
  HRESULT WINAPI DarkDirectInputCreateA(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN)
  {
    return 0;
  }
  
  namespace device::v1
  {
    HRESULT STDMETHODCALLTYPE GetCapabilities(void*, LPDIDEVCAPS) // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE EnumObjects(void*, LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD) // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE GetProperty(void*, REFGUID, LPDIPROPHEADER) // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE SetProperty(void*, REFGUID, LPCDIPROPHEADER) // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE Acquire(void*) // maybe used
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE Unacquire(void*)  // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE GetDeviceState(void*, DWORD, LPVOID) // maybe used
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE GetDeviceData(void*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE SetDataFormat(void*, LPCDIDATAFORMAT) // maybe used
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE SetEventNotification(void*, HANDLE)
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE SetCooperativeLevel(void*, HWND, DWORD) // maybe used
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE GetObjectInfo(void*, LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD)
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE GetDeviceInfo(void*, LPDIDEVICEINSTANCEA)
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE RunControlPanel(void*, HWND, DWORD)
    {
      return 0;
    }


    HRESULT STDMETHODCALLTYPE Initialize(void*, HINSTANCE, DWORD, REFGUID)
    {
      return 0;
    }

  }
  
  namespace device::v2
  {
    HRESULT STDMETHODCALLTYPE CreateEffect(void*, REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT *, LPUNKNOWN)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE EnumEffects(void*, LPDIENUMEFFECTSCALLBACKA, LPVOID, DWORD)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE GetEffectInfo(void*, LPDIEFFECTINFOA, REFGUID)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE GetForceFeedbackState(void*, LPDWORD)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(void*, DWORD)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(void*, LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE Escape(void*, LPDIEFFESCAPE)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE Poll(void*)  // maybe used
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE SendDeviceData(void*, DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD)
    {
      return 0;
    }
  }

  namespace core::v1
  {
    HRESULT STDMETHODCALLTYPE DarkCreateDevice(void*, REFGUID, LPDIRECTINPUTDEVICEA*, LPUNKNOWN*)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkEnumDevices(void*, DWORD, LPDIENUMDEVICESCALLBACKA*, LPVOID*, DWORD)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkGetDeviceStatus(void*, REFGUID)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkRunControlPanel(void*, DWORD)
    {
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkInitialize(void*, HINSTANCE, DWORD)
    {
      return 0;
    }
  }

  namespace core::v2
  {
    HRESULT STDMETHODCALLTYPE DarkFindDevice(void*, REFGUID)
    {
      return 0;
    }
  }
}
