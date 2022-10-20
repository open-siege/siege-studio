#include <array>
#include <string>
#include <string_view>
#include <limits>
#include <unordered_map>
#include <optional>
#include <memory>
#include <SDL.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CINTERFACE
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>

namespace siege
{
   SDL_JoystickType JoystickGetType(SDL_Joystick* joystick)
   {
     auto type = SDL_JoystickGetType(joystick);

     if (type == SDL_JoystickType::SDL_JOYSTICK_TYPE_UNKNOWN || type == SDL_JoystickType::SDL_JOYSTICK_TYPE_GAMECONTROLLER)
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
             else
             {
               cached_types.emplace(vendor_product, SDL_JoystickType::SDL_JOYSTICK_TYPE_UNKNOWN);
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

   struct VirtualJoystick
   {
     SDL_Joystick* joystick;
     std::array<Sint16, 4> axes;
   };

   struct HidAttributes
   {
     Uint16 vendor_id;
     Uint16 product_id;
     Uint16 version_id;
   };

   static std::unordered_map<HANDLE, VirtualJoystick> virtual_joysticks;
   static std::unordered_map<SDL_Joystick*, std::optional<HidAttributes>> joystick_mice;
   static std::unordered_map<SDL_Joystick*, std::optional<HidAttributes>> joystick_keyboards;
   static std::shared_ptr<void> message_hook;

   LRESULT CALLBACK MessageHookCallback(
     int    nCode,
     WPARAM wParam,
     LPARAM lParam
   )
   {
     if (nCode < 0)
     {
       return CallNextHookEx(nullptr, nCode, wParam, lParam);
     }

     auto* data = reinterpret_cast<MSG*>(lParam);

     if (data->message == WM_INPUT && wParam == PM_REMOVE)
     {
       UINT dwSize;

       GetRawInputData((HRAWINPUT)data->lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
       RAWINPUT final_data;
       dwSize = sizeof(RAWINPUT);

       if (GetRawInputData((HRAWINPUT)data->lParam, RID_INPUT, &final_data, &dwSize, sizeof(RAWINPUTHEADER)) <= sizeof(RAWINPUTHEADER))
       {
         return CallNextHookEx(nullptr, nCode, wParam, lParam);
       }

       if (final_data.header.dwType == RIM_TYPEMOUSE)
       {
         auto joystick = virtual_joysticks.find(final_data.header.hDevice);

         if (joystick != virtual_joysticks.end())
         {
           auto& v_joy = joystick->second;

           constexpr static std::array<std::pair<USHORT, USHORT>, 5> mouse_events = {
             std::make_pair(RI_MOUSE_BUTTON_1_DOWN, RI_MOUSE_BUTTON_1_UP),
             std::make_pair(RI_MOUSE_BUTTON_2_DOWN, RI_MOUSE_BUTTON_2_UP),
             std::make_pair(RI_MOUSE_BUTTON_3_DOWN, RI_MOUSE_BUTTON_3_UP),
             std::make_pair(RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_4_UP),
             std::make_pair(RI_MOUSE_BUTTON_5_DOWN, RI_MOUSE_BUTTON_5_UP)
           };

           for (auto i = 0; i < mouse_events.size(); ++i)
           {
             const auto& [down, up] = mouse_events[i];
             if (final_data.data.mouse.usButtonFlags & down)
             {
               SDL_JoystickSetVirtualButton(v_joy.joystick, i, 1);
             }
             else if (final_data.data.mouse.usButtonFlags & up)
             {
               SDL_JoystickSetVirtualButton(v_joy.joystick, i, 0);
             }
           }

           if ((final_data.data.mouse.usFlags & MOUSE_MOVE_RELATIVE) == MOUSE_MOVE_RELATIVE)
           {
             v_joy.axes[0] += Sint16(final_data.data.mouse.lLastX) * 8;
             SDL_JoystickSetVirtualAxis(v_joy.joystick, 0, v_joy.axes[0]);

             v_joy.axes[1] += Sint16(final_data.data.mouse.lLastY) * 8;
             SDL_JoystickSetVirtualAxis(v_joy.joystick, 1, v_joy.axes[1]);
           }
           else if ((final_data.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
           {
             v_joy.axes[0] = Sint16(final_data.data.mouse.lLastX);
             SDL_JoystickSetVirtualAxis(v_joy.joystick, 0, v_joy.axes[0]);

             v_joy.axes[1] = Sint16(final_data.data.mouse.lLastY);
             SDL_JoystickSetVirtualAxis(v_joy.joystick, 1, v_joy.axes[1]);
           }

           if (final_data.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
           {
             v_joy.axes[2] += Sint16(final_data.data.mouse.usButtonData) * 8;
             SDL_JoystickSetVirtualAxis(v_joy.joystick, 2, v_joy.axes[2]);
           }
           else if (final_data.data.mouse.usButtonFlags & RI_MOUSE_HWHEEL)
           {
             v_joy.axes[3] += Sint16(final_data.data.mouse.usButtonData) * 8;
             SDL_JoystickSetVirtualAxis(v_joy.joystick, 3, v_joy.axes[3]);
           }
         }
       }
       else if (final_data.header.dwType == RIM_TYPEKEYBOARD)
       {
         auto joystick = virtual_joysticks.find(final_data.header.hDevice);

         if (joystick != virtual_joysticks.end())
         {
           auto& v_joy = joystick->second;

           auto is_down = (final_data.data.keyboard.Flags & RI_KEY_BREAK) != RI_KEY_BREAK;
           auto is_extended = (final_data.data.keyboard.Flags & RI_KEY_E0) == RI_KEY_E0;

           // Raw Input always provides the 'make' version of a scan code.
           // The below aligns the reported code with the scan-codes used by DirectInput.
           // Extended codes are converted into the 'break' version of the code.
           // See for full list of codes: https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input#scan-codes
           auto code = is_extended ? final_data.data.keyboard.MakeCode | 0x80 : final_data.data.keyboard.MakeCode;

           if (is_down)
           {
             SDL_JoystickSetVirtualButton(v_joy.joystick, code, 1);
           }
           else
           {
             SDL_JoystickSetVirtualButton(v_joy.joystick, code, 0);
           }
         }
       }
     }

     return CallNextHookEx(nullptr, nCode, wParam, lParam);
   }

   std::optional<HidAttributes> DeviceNameToAttributes(std::string device_name)
   {
     constexpr static auto vid_key = std::string_view("VID_");
     constexpr static auto pid_key = std::string_view("PID_");
     auto vid_index = device_name.find(vid_key);
     auto pid_index = device_name.find(pid_key);

     if (vid_index != std::string::npos && pid_index != std::string::npos)
     {
       vid_index += vid_key.size();
       pid_index += pid_key.size();
       auto vid = device_name.substr(vid_index, 4);
       auto pid = device_name.substr(pid_index, 4);

       HidAttributes result{};

       try
       {
         result.vendor_id = Uint16(std::stoul(vid, nullptr, 16));
         result.product_id = Uint16(std::stoul(pid, nullptr, 16));
       }
       catch(...)
       {
         return std::nullopt;
       }

       return result;
     }

     return std::nullopt;
   }

   void InitVirtualJoysticksFromKeyboardsAndMice()
   {
     std::array<RAWINPUTDEVICE, 2> deviceRequest;

     deviceRequest[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
     deviceRequest[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
     deviceRequest[0].dwFlags = 0;
     deviceRequest[0].hwndTarget = 0;

     deviceRequest[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
     deviceRequest[1].usUsage = 0x06;              // HID_USAGE_GENERIC_KEYBOARD
     deviceRequest[1].dwFlags = 0;
     deviceRequest[1].hwndTarget = 0;

     if (RegisterRawInputDevices(deviceRequest.data(), UINT(deviceRequest.size()), sizeof(RAWINPUTDEVICE)) == FALSE)
     {
       return;
     }

     auto hook = SetWindowsHookExA(
       WH_GETMESSAGE,
       MessageHookCallback,
       GetModuleHandleA(nullptr),
       GetCurrentThreadId()
     );

     if (!hook)
     {
       return;
     }

     message_hook.reset(reinterpret_cast<void*>(hook), [](void* value) { UnhookWindowsHookEx(reinterpret_cast<HHOOK>(value)); });

     std::vector<RAWINPUTDEVICELIST> raw_input_devices;

     UINT num_devices;
     GetRawInputDeviceList(nullptr, &num_devices, sizeof(RAWINPUTDEVICELIST));
     raw_input_devices.assign(num_devices, {});
     GetRawInputDeviceList(raw_input_devices.data(), &num_devices, sizeof(RAWINPUTDEVICELIST));

     for (auto& device : raw_input_devices)
     {
       std::string device_name;
       UINT name_size;
       GetRawInputDeviceInfoA(device.hDevice, RIDI_DEVICENAME, nullptr, &name_size);
       device_name.assign(name_size + 12, '\0');
       GetRawInputDeviceInfoA(device.hDevice, RIDI_DEVICENAME, device_name.data(), &name_size);

       RID_DEVICE_INFO device_info{};
       UINT device_size = sizeof(device_info);
       GetRawInputDeviceInfoA(device.hDevice, RIDI_DEVICEINFO, &device_info, &device_size);

       constexpr static HidAttributes default_attributes = { 0 };

       if (device.dwType == RIM_TYPEMOUSE)
       {
         auto attributes = DeviceNameToAttributes(device_name);
         VirtualJoystick joystick{};

         SDL_VirtualJoystickDesc joy{};

         joy.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
         joy.type = SDL_JoystickType::SDL_JOYSTICK_TYPE_FLIGHT_STICK;
         joy.nbuttons = Uint16(device_info.mouse.dwNumberOfButtons);
         joy.naxes = device_info.mouse.fHasHorizontalWheel ? 4 : 3, // 3 for a mouse with a scroll wheels, 4 for a mouse with two scroll wheels;
           joy.nhats = 0;
         joy.vendor_id = attributes.value_or(default_attributes).vendor_id;
         joy.product_id = attributes.value_or(default_attributes).product_id;
         device_name = "Mouse: " + device_name;
         joy.name = device_name.c_str();

         int joystick_index = SDL_JoystickAttachVirtualEx(&joy);

         if (joystick_index != -1)
         {
           joystick.joystick = SDL_JoystickOpen(joystick_index);

           virtual_joysticks.emplace(device.hDevice, joystick);
           joystick_mice.emplace(joystick.joystick, attributes);
         }
       }
       else if (device.dwType == RIM_TYPEKEYBOARD)
       {
         auto attributes = DeviceNameToAttributes(device_name);
         VirtualJoystick joystick{};

         SDL_VirtualJoystickDesc joy{};

         joy.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
         joy.type = SDL_JoystickType::SDL_JOYSTICK_TYPE_FLIGHT_STICK;
         joy.nbuttons = Uint16(device_info.keyboard.dwNumberOfKeysTotal);
         joy.naxes = 0;
         joy.nhats = 0;
         joy.vendor_id = attributes.value_or(default_attributes).vendor_id;
         joy.product_id = attributes.value_or(default_attributes).product_id;
         device_name = "Keyboard: " + device_name;
         joy.name = device_name.c_str();

         int joystick_index = SDL_JoystickAttachVirtualEx(&joy);

         if (joystick_index != -1)
         {
           joystick.joystick = SDL_JoystickOpen(joystick_index);

           virtual_joysticks.emplace(device.hDevice, joystick);
           joystick_keyboards.emplace(joystick.joystick, attributes);
         }
       }
     }
   }

   SDL_bool IsMouse(SDL_Joystick* joystick)
   {
     if (!joystick)
     {
       return SDL_bool::SDL_FALSE;
     }

     auto result = joystick_mice.find(joystick);

     return result == joystick_mice.end() ? SDL_bool::SDL_FALSE : SDL_bool::SDL_TRUE;
   }

   SDL_bool IsKeyboard(SDL_Joystick* joystick)
   {
     if (!joystick)
     {
       return SDL_bool::SDL_FALSE;
     }

     auto result = joystick_keyboards.find(joystick);

     return result == joystick_keyboards.end() ? SDL_bool::SDL_FALSE : SDL_bool::SDL_TRUE;
   }

   std::optional<HidAttributes> GetHidAttributes(SDL_Joystick *joystick)
   {
     if (!joystick)
     {
       return std::nullopt;
     }

     auto result = joystick_mice.find(joystick);

     if (result == joystick_mice.end())
     {
       return std::nullopt;
     }

     if (!result->second.has_value())
     {
       return std::nullopt;
     }

     return result->second;
   }

   Uint16 JoystickGetVendor(SDL_Joystick *joystick)
   {
     auto attrs = GetHidAttributes(joystick);

     if (attrs.has_value())
     {
       return attrs->vendor_id;
     }

     return SDL_JoystickGetVendor(joystick);
   }

   Uint16 JoystickGetProduct(SDL_Joystick *joystick)
   {
     auto attrs = GetHidAttributes(joystick);

     if (attrs.has_value())
     {
       return attrs->product_id;
     }

     return SDL_JoystickGetProduct(joystick);
   }

   Uint16 JoystickGetProductVersion(SDL_Joystick *joystick)
   {
     return SDL_JoystickGetProductVersion(joystick);
   }
}