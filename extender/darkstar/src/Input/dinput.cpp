#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CINTERFACE
#include <windows.h>
#include <dinput.h>

#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <algorithm>
#include <limits>
#include <chrono>
#include <fstream>
#include <platform/platform.hpp>
#include <virtual_joystick.hpp>

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

  namespace core::v2
  {
    HRESULT STDMETHODCALLTYPE DarkFindDevice(IDirectInput2A*, const GUID& deviceClass, LPCSTR deviceName, GUID* result);

    static IDirectInput2AVtbl dinput_vtable = ([](){
      IDirectInput2AVtbl temp{};
      std::memcpy(&temp, &core::v1::dinput_vtable, sizeof(core::v1::dinput_vtable));
      temp.FindDevice = DarkFindDevice;
      return temp;
    })();
  }

  namespace core::v7
  {
    HRESULT STDMETHODCALLTYPE DarkCreateDeviceEx(IDirectInput7A*,
      const GUID& deviceType,
      const IID& interfaceType,
      void** output,
      IUnknown*);

    static IDirectInput7AVtbl dinput_vtable = ([](){
      IDirectInput7AVtbl temp{};
      std::memcpy(&temp, &core::v2::dinput_vtable, sizeof(core::v2::dinput_vtable));
      temp.CreateDeviceEx = DarkCreateDeviceEx;
      return temp;
    })();
  }

  namespace core::v8
  {
    HRESULT STDMETHODCALLTYPE DarkCreateDevice(IDirectInput8A*, const GUID&, IDirectInputDevice8A**, IUnknown*);
    HRESULT STDMETHODCALLTYPE DarkEnumDevicesBySemantics(IDirectInput8A*, LPCSTR username,DIACTIONFORMATA* actionFormat,LPDIENUMDEVICESBYSEMANTICSCBA callback, void* data, DWORD flags);
    HRESULT STDMETHODCALLTYPE DarkConfigureDevices(IDirectInput8A*, LPDICONFIGUREDEVICESCALLBACK, LPDICONFIGUREDEVICESPARAMSA, DWORD, void*);

    static IDirectInput8AVtbl dinput_vtable = ([](){
      IDirectInput8AVtbl temp{};
      std::memcpy(&temp, &core::v2::dinput_vtable, sizeof(core::v2::dinput_vtable));
      temp.CreateDevice = DarkCreateDevice;
      temp.EnumDevicesBySemantics = DarkEnumDevicesBySemantics;
      temp.ConfigureDevices = DarkConfigureDevices;
      return temp;
    })();
  }

  // dinput.dll was first introduced with DirectX 3
  // IDirectInputA/IDirectInputW also came in DirectX 3
  // DirectX 4 never came out.
  // IDirectInput2A/IDirectInput2W came in DirectX 5
  // DirectInput 3 only supports Keyboard and Mouse. The Windows Joystick API was still expected to be used for Joysticks.
  // DirectInput 5 added support for Joysticks as well as force-feedback.
  create_result WINAPI DarkDirectInputCreateA(HINSTANCE appInstance, DWORD version, IDirectInputA** output, IUnknown*)
  {
    static std::unordered_map<DWORD, IDirectInputA> instances;

    auto result = instances.emplace(version, IDirectInputA{ &core::v1::dinput_vtable });

    result.first->second.lpVtbl->Initialize(&result.first->second, appInstance, version);
    *output = &result.first->second;

    return create_result::ok;
  }

  // DirectInputCreateEX was introduced with DirectX 7.
  HRESULT WINAPI DarkDirectInputCreateExA(HINSTANCE appInstance, DWORD version, const IID& interfaceId, void** output, IUnknown*)
  {
    static std::unordered_map<DWORD, IDirectInputA> instances;

    auto result = instances.emplace(version, IDirectInputA{ &core::v1::dinput_vtable });

    result.first->second.lpVtbl->Initialize(&result.first->second, appInstance, version);
    return result.first->second.lpVtbl->QueryInterface(&result.first->second, interfaceId, output);
  }

  // This resides in dinput8.dll
  // Introduced with DirectX 8
  HRESULT WINAPI DirectInput8Create(HINSTANCE appInstance, DWORD version, const IID& interfaceId, void** output, IUnknown*)
  {
    static std::unordered_map<DWORD, IDirectInput8A> instances;

    auto result = instances.emplace(version, IDirectInput8A{ &core::v8::dinput_vtable });

    result.first->second.lpVtbl->Initialize(&result.first->second, appInstance, version);
    return result.first->second.lpVtbl->QueryInterface(&result.first->second, interfaceId, output);
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

  namespace device::v8
  {
    HRESULT STDMETHODCALLTYPE DarkBuildActionMap(IDirectInputDevice8A*, DIACTIONFORMATA* actionFormat, LPCSTR username, DWORD flags);
    HRESULT STDMETHODCALLTYPE DarkSetActionMap(IDirectInputDevice8A*, DIACTIONFORMATA* actionFormat, LPCSTR username, DWORD flags);
    HRESULT STDMETHODCALLTYPE DarkGetImageInfo(IDirectInputDevice8A*, DIDEVICEIMAGEINFOHEADERA* deviceImage);

    static IDirectInputDevice8AVtbl device_vtable = ([](){
      IDirectInputDevice8AVtbl temp{};
      std::memcpy(&temp, &device::v7::device_vtable, sizeof(device::v7::device_vtable));
      temp.BuildActionMap = DarkBuildActionMap;
      temp.SetActionMap = DarkSetActionMap;
      temp.GetImageInfo = DarkGetImageInfo;
      return temp;
    })();
  }

  struct joystick_state
  {
    std::vector<Sint16> axes;
    std::vector<Uint8> hats;
    std::vector<Uint8> buttons;
  };

  struct device_info
  {
    IDirectInputDeviceA device;
    std::unique_ptr<siege::Joystick, void(*)(siege::Joystick*)> joystick;
    DIDATAFORMAT* format;
    joystick_state previous_values;
    std::vector<DIDEVICEOBJECTDATA> event_buffer;
  };

  static std::vector<device_info> devices;

  static std::optional<IDirectInputDeviceA> keyboard;
  static std::optional<IDirectInputDeviceA> mouse;

  namespace device::v1
  {
    HRESULT STDMETHODCALLTYPE QueryInterface(IDirectInputDeviceA* self, REFIID interface_id, void** result)
    {
        if (interface_id == IID_IDirectInputDeviceA)
        {
          self->lpVtbl = &device::v1::device_vtable;
          *result = self;
        }
        else if (interface_id == IID_IDirectInputDevice2A)
        {
          IDirectInputDevice2A* new_device = reinterpret_cast<IDirectInputDevice2A*>(self);
          new_device->lpVtbl = &device::v2::device_vtable;
          *result = new_device;
        }
        else if (interface_id == IID_IDirectInputDevice7A)
        {
          IDirectInputDevice7A* new_device = reinterpret_cast<IDirectInputDevice7A*>(self);
          new_device->lpVtbl = &device::v7::device_vtable;
          *result = new_device;
        }
        else if (interface_id == IID_IDirectInputDevice8A)
        {
          IDirectInputDevice8A* new_device = reinterpret_cast<IDirectInputDevice8A*>(self);
          new_device->lpVtbl = &device::v8::device_vtable;
          *result = new_device;
        }
        else
        {
          return DIERR_NOINTERFACE;
        }

        return DI_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef(IDirectInputDeviceA*)
    {
      return 1;
    }

    ULONG STDMETHODCALLTYPE Release(IDirectInputDeviceA*)
    {
      return 1;
    }

    HRESULT STDMETHODCALLTYPE DarkGetCapabilities(IDirectInputDeviceA* self, DIDEVCAPS* caps) // maybe used
    {
      auto size = caps->dwSize;

      auto info = std::find_if(devices.begin(), devices.end(), [self](const auto& value) { return &value.device == self; });

      if (info == devices.end())
      {
        return DIERR_INVALIDPARAM;
      }

      caps->dwFlags = DIDC_ATTACHED | DIDC_POLLEDDEVICE;

      // TODO needs a mask here for the sub type as well
      caps->dwDevType = DI8DEVTYPE_FLIGHT;
      caps->dwAxes = siege::JoystickNumAxes(info->joystick.get());
      caps->dwButtons = siege::JoystickNumButtons(info->joystick.get());
      caps->dwPOVs = siege::JoystickNumHats(info->joystick.get());

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

    LONG GetAxisOffset(int num_axes, int index)
    {
      if (index == 0)
      {
        return DIJOFS_X;
      }

      if (index == 1)
      {
        return DIJOFS_Y;
      }

      if (num_axes == 3 && index == 2)
      {
        return DIJOFS_Z;
      }
      else if (num_axes >= 4 && index == 2)
      {
        return DIJOFS_RZ;
      }

      if (num_axes >= 4 && index == 3)
      {
        return DIJOFS_Z;
      }

      if (index == 4)
      {
        return DIJOFS_RX;
      }

      if (index == 5)
      {
        return DIJOFS_RY;
      }

      index = index - 6;

      return DIJOFS_SLIDER(index);
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

    HRESULT STDMETHODCALLTYPE DarkGetObjectInfo(IDirectInputDeviceA* self, DIDEVICEOBJECTINSTANCEA* result, DWORD value, DWORD how)
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

      // TODO need to also support the offset parameter
      if (how == DIPH_BYID)
      {
          auto type = DIDFT_GETTYPE(value);
          auto instance = DIDFT_GETINSTANCE(value);

          if (type == DIDFT_AXIS)
          {
            auto num_axes = siege::JoystickNumAxes(info->joystick.get());

            result->guidType = GetAxisObjectType(num_axes, instance);

            auto name = GetAxisName(result->guidType);
            std::memcpy(&result->tszName, name.data(), name.size());
          }
          else if (type == DIDFT_POV)
          {
            result->guidType = GUID_POV;

            auto name = "Hat " + std::to_string(instance + 1);
            std::memcpy(&result->tszName, name.data(), name.size());
          }
          else if (type == DIDFT_BUTTON)
          {
            result->guidType = GUID_Button;
            auto name = "Button " + std::to_string(instance + 1);
            std::memcpy(&result->tszName, name.data(), name.size());
          }
      }

      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkEnumObjects(IDirectInputDeviceA* self, LPDIENUMDEVICEOBJECTSCALLBACKA callback, void* data, DWORD flags)
    {
      auto info = std::find_if(devices.begin(), devices.end(), [self](const auto& value) { return &value.device == self; });

      if (info == devices.end())
      {
        return DIERR_INVALIDPARAM;
      }

      auto num_axes = siege::JoystickNumAxes(info->joystick.get());
      for (auto i = 0; i < num_axes; ++i)
      {
        DIDEVICEOBJECTINSTANCEA object{};
        object.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
        object.guidType = GetAxisObjectType(num_axes, i);
        object.dwType = DIDFT_MAKEINSTANCE(i) | DIDFT_AXIS;

        auto name = GetAxisName(object.guidType);
        std::memcpy(&object.tszName, name.data(), name.size());

        auto result = callback(&object, data);

        if (result == FALSE)
        {
          return 0;
        }
      }

      auto num_hats = siege::JoystickNumHats(info->joystick.get());
      for (auto i = 0; i < num_hats; ++i)
      {
        DIDEVICEOBJECTINSTANCEA object{};
        object.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
        object.guidType = GUID_POV;
        object.dwType = DIDFT_MAKEINSTANCE(i) | DIDFT_POV;

        auto name = "Hat " + std::to_string(i + 1);
        std::memcpy(&object.tszName, name.data(), name.size());

        auto result = callback(&object, data);

        if (result == FALSE)
        {
          return 0;
        }
      }

      for (auto i = 0; i < siege::JoystickNumButtons(info->joystick.get()); ++i)
      {
        DIDEVICEOBJECTINSTANCEA object{};
        object.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
        object.guidType = GUID_Button;
        object.dwType = DIDFT_MAKEINSTANCE(i) | DIDFT_BUTTON;
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

      if (prop_id == DIPROP_APPDATA)
      {
        // TODO come back to this later
      }
      else if (prop_id == DIPROP_AUTOCENTER)
      {
        return S_FALSE;
      }
      else if (prop_id == DIPROP_AXISMODE && result->dwSize == sizeof(DIPROPDWORD))
      {
        auto* value = reinterpret_cast<DIPROPDWORD*>(result);

        value->dwData = DIPROPAXISMODE_ABS;
      }
      else if (prop_id == DIPROP_BUFFERSIZE)
      {
        return S_FALSE;
      }
      else if (prop_id == DIPROP_CALIBRATION)
      {
        return S_FALSE;
      }
      else if (prop_id == DIPROP_CALIBRATIONMODE)
      {
        return S_FALSE;
      }
      else if (prop_id == DIPROP_CPOINTS)
      {
        return S_FALSE;
      }
      else if (prop_id == DIPROP_DEADZONE)
      {
        return S_FALSE;
      }
      else if (prop_id == DIPROP_FFGAIN)
      {
        return DI_OK;
      }
      else if (prop_id == DIPROP_FFLOAD)
      {
        return S_FALSE;
      }
      else if (prop_id == DIPROP_GRANULARITY)
      {
        return S_FALSE;
      }
      else if (prop_id == DIPROP_USERNAME && result->dwSize == sizeof(DIPROPSTRING))
      {
        auto player_index = siege::JoystickGetPlayerIndex(info->joystick.get());

        if (player_index == -1)
        {
          return S_FALSE;
        }

        auto* value = reinterpret_cast<DIPROPSTRING*>(result);
        auto player_name = "Player " + std::to_string(player_index + 1);
        std::memcpy(&value->wsz, player_name.data(), player_name.size());
      }
      else if (prop_id == DIPROP_GUIDANDPATH)
      {
        auto* product_path = siege::JoystickPath(info->joystick.get());

        if (!product_path)
        {
          return S_FALSE;
        }

        auto* value = reinterpret_cast<DIPROPSTRING*>(result);
        auto product_path_str = std::string_view(product_path);

        std::memcpy(&value->wsz, product_path_str.data(), product_path_str.size());
      }
      else if (prop_id == DIPROP_VIDPID && result->dwSize == sizeof(DIPROPDWORD))
      {
        auto vendor_id = siege::JoystickGetVendor(info->joystick.get());
        auto product_id = siege::JoystickGetVendor(info->joystick.get());
        auto* value = reinterpret_cast<DIPROPDWORD*>(result);
      }
      else if ((prop_id == DIPROP_PRODUCTNAME || prop_id == DIPROP_INSTANCENAME || prop_id == DIPROP_GETPORTDISPLAYNAME) &&
               result->dwSize == sizeof(DIPROPSTRING))
      {
        auto* product_name = siege::JoystickName(info->joystick.get());

        if (!product_name)
        {
          return S_FALSE;
        }

        auto* value = reinterpret_cast<DIPROPSTRING*>(result);
        auto product_name_str = std::string_view(product_name);

        std::memcpy(&value->wsz, product_name_str.data(), product_name_str.size());
      }
      else if (prop_id == DIPROP_JOYSTICKID)
      {

      }
      else if (prop_id == DIPROP_KEYNAME)
      {
        // TODO come back to this
        return S_FALSE;
      }
      else if (prop_id == DIPROP_SCANCODE)
      {
        // TODO come back to this
        return S_FALSE;
      }
      else if (prop_id == DIPROP_LOGICALRANGE && result->dwSize == sizeof(DIPROPRANGE))
      {
        auto* range = reinterpret_cast<DIPROPRANGE*>(result);
        range->lMin = std::numeric_limits<std::int16_t>::min();
        range->lMax = std::numeric_limits<std::int16_t>::max();
      }
      else if (prop_id == DIPROP_PHYSICALRANGE && result->dwSize == sizeof(DIPROPRANGE))
      {
        auto* range = reinterpret_cast<DIPROPRANGE*>(result);
        range->lMin = std::numeric_limits<std::int16_t>::min();
        range->lMax = std::numeric_limits<std::int16_t>::max();
      }
      else if (prop_id == DIPROP_RANGE && result->dwSize == sizeof(DIPROPRANGE))
      {
        auto* range = reinterpret_cast<DIPROPRANGE*>(result);
        range->lMin = std::numeric_limits<std::int16_t>::min();
        range->lMax = std::numeric_limits<std::int16_t>::max();
      }
      else if (prop_id == DIPROP_SATURATION)
      {
        return S_FALSE;
      }
      else if (prop_id == DIPROP_TYPENAME)
      {
        return S_FALSE;
      }

      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkSetProperty(IDirectInputDeviceA*, const GUID& prop_id, DIPROPHEADER* result) // maybe used
    {
      if (!result)
      {
        return DIERR_INVALIDPARAM;
      }

      if (prop_id == DIPROP_APPDATA)
      {
        // TODO come back to this later
      }

      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkAcquire(IDirectInputDeviceA*)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkAcquire returning \n";
      return DI_OK;
    }


    HRESULT STDMETHODCALLTYPE DarkUnacquire(IDirectInputDeviceA*)  // maybe used
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkUnacquire returning \n";
      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkSetDataFormat(IDirectInputDeviceA* self, DIDATAFORMAT* format) // maybe used
    {
      auto info = std::find_if(devices.begin(), devices.end(), [self](const auto& value) { return &value.device == self; });

      if (info == devices.end())
      {
        return DIERR_INVALIDPARAM;
      }

      info->format = format;

      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkGetDeviceState(IDirectInputDeviceA* self, DWORD size, void* result) // maybe used
    {
      auto info = std::find_if(devices.begin(), devices.end(), [self](const auto& value) { return &value.device == self; });

      if (info == devices.end())
      {
        return DIERR_INVALIDPARAM;
      }

      auto update_state = [&](auto& joy_state) {

        //stick
        joy_state.lX = siege::JoystickGetAxis(info->joystick.get(), 0);
        joy_state.lY = siege::JoystickGetAxis(info->joystick.get(), 1);

        // rudder
        joy_state.lRz = siege::JoystickGetAxis(info->joystick.get(), 2);

        // throttle
        joy_state.lZ = siege::JoystickGetAxis(info->joystick.get(), 3);

        joy_state.lRx = siege::JoystickGetAxis(info->joystick.get(), 4);
        joy_state.lRy = siege::JoystickGetAxis(info->joystick.get(), 5);

        joy_state.rglSlider[0] = siege::JoystickGetAxis(info->joystick.get(), 6);
        joy_state.rglSlider[1] = siege::JoystickGetAxis(info->joystick.get(), 7);

        joy_state.rgdwPOV[0] = siege::JoystickGetHat(info->joystick.get(), 0);
        joy_state.rgdwPOV[1] = siege::JoystickGetHat(info->joystick.get(), 1);
        joy_state.rgdwPOV[2] = siege::JoystickGetHat(info->joystick.get(), 2);
        joy_state.rgdwPOV[3] = siege::JoystickGetHat(info->joystick.get(), 3);

        for (auto i = 0; i < 32; ++i)
        {
          joy_state.rgbButtons[i] = siege::JoystickGetButton(info->joystick.get(), i)  == 1 ? std::numeric_limits<BYTE>::max() : std::numeric_limits<BYTE>::min();
        }
      };

      if (info->format == &c_dfDIJoystick)
      {
        DIJOYSTATE joy_state{};
        update_state(joy_state);
        std::memcpy(result, &joy_state, size);
      }
      else if (info->format == &c_dfDIJoystick2)
      {
        DIJOYSTATE2 joy_state{};
        update_state(joy_state);
        std::memcpy(result, &joy_state, size);
      }
      else if (info->format == &c_dfDIMouse)
      {
        DIMOUSESTATE mouse_state{};
        mouse_state.lX = siege::JoystickGetAxis(info->joystick.get(), 0);
        mouse_state.lY = siege::JoystickGetAxis(info->joystick.get(), 1);
        mouse_state.lZ = siege::JoystickGetAxis(info->joystick.get(), 2);

        for (auto i = 0; i < 4; ++i)
        {
          mouse_state.rgbButtons[i] = siege::JoystickGetButton(info->joystick.get(), i) == 1 ? std::numeric_limits<BYTE>::max() : std::numeric_limits<BYTE>::min();
        }
      }
      else if (info->format == &c_dfDIMouse2)
      {
        DIMOUSESTATE2 mouse_state{};
        mouse_state.lX = siege::JoystickGetAxis(info->joystick.get(), 0);
        mouse_state.lY = siege::JoystickGetAxis(info->joystick.get(), 1);
        mouse_state.lZ = siege::JoystickGetAxis(info->joystick.get(), 2);

        for (auto i = 0; i < 8; ++i)
        {
          mouse_state.rgbButtons[i] = siege::JoystickGetButton(info->joystick.get(), i) == 1 ? std::numeric_limits<BYTE>::max() : std::numeric_limits<BYTE>::min();
        }
      }
      else if (info->format == &c_dfDIKeyboard)
      {
        std::vector<BYTE> keys(256, 0);

        for (auto i = 0; i < 256; ++i)
        {
          keys[i] = siege::JoystickGetButton(info->joystick.get(), i) == 1 ? std::numeric_limits<BYTE>::max() : std::numeric_limits<BYTE>::min();
        }
      }

      return DI_OK;
    }

    // https://stackoverflow.com/questions/24957464/is-there-any-c-standard-class-function-which-is-similar-to-gettickcount-on-w
    DWORD DarkTickCount()
    {
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    HRESULT STDMETHODCALLTYPE DarkGetDeviceData(IDirectInputDeviceA* self, DWORD size, DIDEVICEOBJECTDATA* results, DWORD* length, DWORD flags) // maybe used
    {
      if (size != sizeof(DIDEVICEOBJECTDATA))
      {
        return DIERR_INVALIDPARAM;
      }

      if (!length)
      {
        return DIERR_INVALIDPARAM;
      }

      auto info = std::find_if(devices.begin(), devices.end(), [self](const auto& value) { return &value.device == self; });

      if (info == devices.end())
      {
        return DIERR_INVALIDPARAM;
      }

      if (results == nullptr && *length == INFINITE)
      {
        *length = info->event_buffer.size();

        if (flags != DIGDD_PEEK)
        {
          info->event_buffer.clear();
        }

        return DI_OK;
      }

      siege::JoystickUpdate();

      info->previous_values.axes.resize(siege::JoystickNumAxes(info->joystick.get()));
      info->previous_values.hats.resize(siege::JoystickNumHats(info->joystick.get()));
      info->previous_values.buttons.resize(siege::JoystickNumButtons(info->joystick.get()));
      info->event_buffer.reserve(
        info->previous_values.axes.size() +
        info->previous_values.hats.size() +
        info->previous_values.buttons.size());

      static DWORD sequence = 0;

      for (auto i = 0; i < siege::JoystickNumAxes(info->joystick.get()); ++i)
      {
        if (info->previous_values.axes[i] != siege::JoystickGetAxis(info->joystick.get(), i))
        {
          info->previous_values.axes[i] = siege::JoystickGetAxis(info->joystick.get(), i);
          auto& data = info->event_buffer.emplace_back();
          data.dwOfs = GetAxisOffset(siege::JoystickNumAxes(info->joystick.get()), i);
          data.dwData = info->previous_values.axes[i];
          data.dwSequence = sequence++;
          data.dwTimeStamp = DarkTickCount();
          data.uAppData = 0;
        }
      }

      for (auto i = 0; i < siege::JoystickNumHats(info->joystick.get()); ++i)
      {
        if (info->previous_values.hats[i] != siege::JoystickGetHat(info->joystick.get(), i))
        {
          info->previous_values.hats[i] = siege::JoystickGetHat(info->joystick.get(), i);
          auto& data = info->event_buffer.emplace_back();
          data.dwOfs = DIJOFS_POV(i);
          data.dwData = info->previous_values.hats[i];
          data.dwSequence = sequence++;
          data.dwTimeStamp = DarkTickCount();
          data.uAppData = 0;
        }
      }

      for (auto i = 0; i < siege::JoystickNumButtons(info->joystick.get()); ++i)
      {
        if (info->previous_values.buttons[i] != siege::JoystickGetButton(info->joystick.get(), i))
        {
          info->previous_values.buttons[i] = siege::JoystickGetButton(info->joystick.get(), i);
          auto& data = info->event_buffer.emplace_back();
          data.dwOfs = DIJOFS_BUTTON(i);
          data.dwData = info->previous_values.buttons[i] == 1 ? std::numeric_limits<BYTE>::max() : std::numeric_limits<BYTE>::min();
          data.dwSequence = sequence++;
          data.dwTimeStamp = DarkTickCount();
          data.uAppData = 0;
        }
      }

      auto requested_length = *length;
      auto actual_length = info->event_buffer.size();

      if (requested_length >= actual_length)
      {
        std::copy_n(info->event_buffer.begin(), actual_length, results);
        info->event_buffer.clear();
        *length = actual_length;
      }
      else
      {
        std::copy_n(info->event_buffer.begin(), requested_length, results);
        info->event_buffer.clear();
        return DI_BUFFEROVERFLOW;
      }

      return DI_OK;
    }


    HRESULT STDMETHODCALLTYPE DarkSetEventNotification(IDirectInputDeviceA*, HANDLE)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkSetEventNotification returning \n";
      return DI_OK;
    }


    HRESULT STDMETHODCALLTYPE DarkSetCooperativeLevel(IDirectInputDeviceA*, HWND, DWORD) // maybe used
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkSetCooperativeLevel returning \n";
      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkGetDeviceInfo(IDirectInputDeviceA*, DIDEVICEINSTANCEA* info)
    {
      // TODO this needs to be implemented properly
      info->dwSize = sizeof(DIDEVICEINSTANCEA);
      info->dwDevType = DI8DEVTYPE_FLIGHT;
      return DI_OK;
    }


    HRESULT STDMETHODCALLTYPE DarkRunControlPanel(IDirectInputDeviceA*, HWND, DWORD)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "Device::DarkRunControlPanel returning \n";
      return DI_OK;
    }


    HRESULT STDMETHODCALLTYPE DarkInitialize(IDirectInputDeviceA*, HINSTANCE, DWORD version)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "Device::DarkInitialize returning \n";
      return DI_OK;
    }
  }

  namespace device::v2
  {
    HRESULT STDMETHODCALLTYPE DarkCreateEffect(IDirectInputDevice2A*, const GUID&, const DIEFFECT*, IDirectInputEffect**, IUnknown*)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkCreateEffect returning \n";
      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkEnumEffects(IDirectInputDevice2A*, LPDIENUMEFFECTSCALLBACKA callback, LPVOID data, DWORD filter)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkEnumEffects returning \n";
      // TODO figure this out later
//      DIEFFECTINFOA effect{};
//
//      effect.dwSize = sizeof(DIEFFECTINFOA);
//      effect.dwEffType = DIEFT_CONDITION;
//      effect.dwStaticParams = DIEP_ENVELOPE;
//      effect.dwDynamicParams = DIEP_ENVELOPE;
//      //effect.tszName = "Vibration";
//
//      auto result = callback(&effect, data);

      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkGetEffectInfo(IDirectInputDevice2A*, DIEFFECTINFOA* info, const GUID&)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkGetEffectInfo returning \n";
      // TODO figure this out later
//      info->dwSize = sizeof(DIEFFECTINFOA);
//      info->dwEffType = DIEFT_CONDITION;
//      info->dwStaticParams = DIEP_ENVELOPE;
//      info->dwDynamicParams = DIEP_ENVELOPE;
      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkGetForceFeedbackState(IDirectInputDevice2A*, DWORD* value)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkGetForceFeedbackState returning \n";
      *value = DIGFFS_STOPPED;
      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkSendForceFeedbackCommand(IDirectInputDevice2A*, DWORD flags)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkSendForceFeedbackCommand returning \n";
      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkEnumCreatedEffectObjects(IDirectInputDevice2A*, LPDIENUMCREATEDEFFECTOBJECTSCALLBACK callback, void* data, DWORD)
    {
      // TODO come back to this later
     // IDirectInputEffect* effect = nullptr;

     // auto result = callback(effect, data);
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkEnumCreatedEffectObjects returning \n";
      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkEscape(IDirectInputDevice2A*, DIEFFESCAPE* command)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkEscape returning \n";
      return DIERR_DEVICEFULL;
    }

    HRESULT STDMETHODCALLTYPE DarkPoll(IDirectInputDevice2A*)  // maybe used
    {
      siege::JoystickUpdate();
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkSendDeviceData(IDirectInputDevice2A*, DWORD size, const DIDEVICEOBJECTDATA* data, DWORD* length, DWORD flags)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkSendDeviceData returning \n";
      return DIERR_REPORTFULL;
    }
  }

  namespace device::v7
  {
    HRESULT STDMETHODCALLTYPE DarkEnumEffectsInFile(IDirectInputDevice7A*, LPCSTR fileName, LPDIENUMEFFECTSINFILECALLBACK callback, void* data, DWORD flags)
    {
      // TODO finish this when I know how it should work
//      DIFILEEFFECT effect{};
//
//      auto result = callback(&effect, data);

      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkEnumEffectsInFile returning \n";
      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkWriteEffectToFile(IDirectInputDevice7A*, LPCSTR filename, DWORD, DIFILEEFFECT* effect, DWORD flags)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkWriteEffectToFile returning \n";
      return DI_OK;
    }
  }

  namespace core::v1
  {
    HRESULT STDMETHODCALLTYPE QueryInterface(IDirectInputA* self, REFIID interface_id, void** result)
    {
      bool should_init = SDL_WasInit(SDL_INIT_JOYSTICK) & SDL_INIT_JOYSTICK;

      auto re_init = [&](std::string_view expected_value) {
        auto enabled = SDL_GetHint(SDL_HINT_DIRECTINPUT_ENABLED);

        if (!enabled)
        {
          enabled = "1";
        }

        if (enabled != expected_value)
        {
          SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
          SDL_SetHint(SDL_HINT_DIRECTINPUT_ENABLED, expected_value == "1" ? "1" : "0");

          if (should_init)
          {
            SDL_InitSubSystem(SDL_INIT_JOYSTICK);
          }
        }
      };

      if (interface_id == IID_IDirectInputA)
      {
        re_init("1");

        self->lpVtbl = &core::v1::dinput_vtable;
        *result = self;
      }
      else if (interface_id == IID_IDirectInput2A)
      {
        re_init("1");
        IDirectInput2A* new_dinput = reinterpret_cast<IDirectInput2A*>(self);
        new_dinput->lpVtbl = &core::v2::dinput_vtable;
        *result = new_dinput;
      }
      else if (interface_id == IID_IDirectInput7A)
      {
        re_init("1");
        IDirectInput7A* new_dinput = reinterpret_cast<IDirectInput7A*>(self);
        new_dinput->lpVtbl = &core::v7::dinput_vtable;
        *result = new_dinput;
      }
      else if (interface_id == IID_IDirectInput8A)
      {
        re_init("0");
        IDirectInput8A* new_dinput = reinterpret_cast<IDirectInput8A*>(self);
        new_dinput->lpVtbl = &core::v8::dinput_vtable;
        *result = new_dinput;
      }
      else
      {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        return DIERR_NOINTERFACE;
      }

      return DI_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef(IDirectInputA*)
    {
      return 1;
    }

    ULONG STDMETHODCALLTYPE Release(IDirectInputA*)
    {
      return 1;
    }

    auto sdl_to_dinput_type(siege::JoystickType sdl_type)
    {
      return DI8DEVTYPE_FLIGHT;
    }

    HRESULT STDMETHODCALLTYPE DarkEnumDevices(IDirectInputA* me, DWORD typeFilter, LPDIENUMDEVICESCALLBACKA callback, void* data, DWORD flags)
    {
      for (auto i = 0; i < SDL_NumJoysticks(); ++i)
      {
        DIDEVICEINSTANCEA device{};

        device.dwSize = sizeof(DIDEVICEINSTANCEA);

        auto device_guid = siege::JoystickGetDeviceGUID(i);
        static_assert(sizeof(device_guid) == sizeof(device.guidProduct));

        std::memcpy(&device.guidProduct, &device_guid, sizeof(device_guid));

        static_assert(sizeof(i) <= sizeof(device.guidInstance));

        std::memcpy(&device.guidInstance, &i, sizeof(i));

        device.dwDevType = sdl_to_dinput_type(siege::JoystickGetDeviceType(i));

        auto instance_name = siege::JoystickNameForIndex(i);
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

    HRESULT STDMETHODCALLTYPE DarkCreateDevice(IDirectInputA* self, const GUID& deviceType, IDirectInputDeviceA** output, IUnknown*)
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

      if (!self)
      {
        return DIERR_INVALIDPARAM;
      }

      self->lpVtbl->Initialize(self, 0, 0);

      if (devices.empty())
      {
        for (auto i = 0; i < SDL_NumJoysticks(); ++i)
        {
          auto& device = devices.emplace_back(device_info{
            { &device::v1::device_vtable },
            { siege::JoystickOpen(i), [](siege::Joystick* joy){ siege::JoystickClose(joy); } },
            const_cast<DIDATAFORMAT*>(&c_dfDIJoystick)
          });
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
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkGetDeviceStatus returning \n";
      return DI_OK;
    }

    HRESULT STDMETHODCALLTYPE DarkRunControlPanel(IDirectInputA*, DWORD)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::app);
      log << "DarkRunControlPanel returning \n";
      return 0;
    }

    HRESULT STDMETHODCALLTYPE DarkInitialize(IDirectInputA*, HINSTANCE, DWORD)
    {
      if (SDL_WasInit(SDL_INIT_JOYSTICK) & SDL_INIT_JOYSTICK)
      {
        return DI_OK;
      }

      auto result = SDL_InitSubSystem(SDL_INIT_JOYSTICK);

      if (result != 0)
      {
        return DIERR_OLDDIRECTINPUTVERSION;
      }

      std::ofstream log("darkstar.winmm.log", std::ios::trunc);
      log << "SDL Joystick sub system init\n";
      siege::JoystickUpdate();
      siege::InitVirtualJoysticksFromJoysticks();

      return DI_OK;
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
        return std::string_view(siege::JoystickName(info.joystick.get())) == deviceName;
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
      IDirectInputDeviceA* temp;
      auto result = core::v1::DarkCreateDevice(nullptr, deviceType, &temp, nullptr);

      if (result != DI_OK)
      {
        return result;
      }

      return temp->lpVtbl->QueryInterface(temp, interfaceType, output);
    }
  }

  namespace core::v8
  {
    HRESULT STDMETHODCALLTYPE DarkCreateDevice(IDirectInput8A*, const GUID& deviceType, IDirectInputDevice8A** output, IUnknown*)
    {
      IDirectInputDeviceA* temp;
      auto result = core::v1::DarkCreateDevice(nullptr, deviceType, &temp, nullptr);

      if (result != DI_OK)
      {
        return result;
      }

      return temp->lpVtbl->QueryInterface(temp, IID_IDirectInputDevice8A, reinterpret_cast<void**>(output));
    }
  }
  namespace effect
  {

  }
}
