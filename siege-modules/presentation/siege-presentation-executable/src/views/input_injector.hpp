#ifndef SIEGE_LAUNCHER_INPUT_INJECTOR_HPP
#define SIEGE_LAUNCHER_INPUT_INJECTOR_HPP

#include <siege/platform/win/window.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <optional>
#include <array>
#include <set>
#include <map>
#include <bitset>
#include <hidusage.h>
#include <xinput.h>
#include <mmsystem.h>
#include <siege/extension/input_filter.hpp>

namespace siege
{

  struct input_injector_args
  {
    enum mode
    {
      none,
      bind_input,
      bind_action
    };

    siege::platform::game_command_line_args& args;
    std::wstring script_host;
    mode input_mode = bind_input;
    std::function<HRESULT(const siege::platform::game_command_line_args* game_args, PROCESS_INFORMATION* process_info)> launch_game_with_extension;
    std::function<void()> on_process_closed;
  };

  enum class input_state
  {
    down,
    up
  };

  inline ::INPUT vk_to_input(WORD vkey, siege::platform::hardware_context context, std::uint32_t device_id, input_state state, std::optional<WORD> intensity = std::nullopt)
  {
    ::INPUT result{};

    if (vkey == VK_LBUTTON)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_LEFTDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (vkey == VK_RBUTTON)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_RIGHTDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (vkey == VK_MBUTTON)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_MIDDLEUP : MOUSEEVENTF_MIDDLEDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (vkey == VK_XBUTTON1)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = XBUTTON1;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_XUP : MOUSEEVENTF_XDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (vkey == VK_XBUTTON2)
    {
      result.type = INPUT_MOUSE;
      result.mi.dwFlags = XBUTTON2;
      result.mi.dwFlags = state == input_state::up ? MOUSEEVENTF_XUP : MOUSEEVENTF_XDOWN;
      result.mi.dwExtraInfo = device_id;
      return result;
    }

    if (vkey >= VK_GAMEPAD_A && vkey <= VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
    {
      result.type = INPUT_KEYBOARD;
      result.ki.wVk = vkey;


      result.ki.dwFlags = state == input_state::up ? KEYEVENTF_KEYUP : 0;

      std::uint16_t controller_intensity = intensity ? *intensity : state == input_state::up ? 0
                                                                                             : (std::uint16_t)-1;

      result.ki.dwExtraInfo = MAKELPARAM(device_id, controller_intensity);

      return result;
    }

    // TODO handle extended codes correctly
    result.type = INPUT_KEYBOARD;
    result.ki.wScan = ::MapVirtualKeyW(vkey, MAPVK_VK_TO_VSC);
    result.ki.dwFlags = state == input_state::up ? KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP : KEYEVENTF_SCANCODE;
    result.ki.dwExtraInfo = device_id;

    if (context == siege::platform::hardware_context::keyboard && (vkey == VK_HOME || vkey == VK_INSERT || vkey == VK_DELETE || vkey == VK_END || vkey == VK_NEXT || vkey == VK_PRIOR))
    {
      result.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }

    if (context == siege::platform::hardware_context::keypad && (vkey == VK_RETURN))
    {
      result.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }

    return result;
  }

  struct input_injector
  {
    win32::hwnd_t target_window;
    input_injector_args injector_args;
    HANDLE process_handle;
    HANDLE thread_handle;
    PROCESS_INFORMATION child_process;
    std::vector<INPUT> simulated_inputs;

    std::set<HANDLE> registered_mice;
    std::set<HANDLE> registered_keyboards;
    std::set<HANDLE> registered_controllers;
    std::set<HANDLE> regular_controllers;
    std::map<int, XINPUT_STATE> controller_state;
    std::map<std::string_view, bool> action_states;

    win32::module xinput_module;
    std::add_pointer_t<decltype(::XInputGetState)> xinput_get_state = nullptr;
    std::add_pointer_t<decltype(::XInputGetKeystroke)> xinput_get_key_stroke = nullptr;

    static LRESULT __stdcall sub_class_callback(
      HWND hwnd,
      UINT message,
      WPARAM wparam,
      LPARAM lparam,
      UINT_PTR uIdSubclass,
      DWORD_PTR dwRefData)
    {
      if (message == WM_INPUT && uIdSubclass)
      {
        auto* self = (input_injector*)uIdSubclass;
        return self->wm_input(win32::input_message(wparam, lparam));
      }

      if (message == WM_INPUT_DEVICE_CHANGE && uIdSubclass)
      {
        auto* self = (input_injector*)uIdSubclass;
        return self->wm_input_device_change(win32::input_device_change_message(wparam, lparam));
      }

      if (message == WM_NCDESTROY)
      {
        win32::remove_window_subclass(hwnd, sub_class_callback, uIdSubclass);
      }

      return win32::def_subclass_proc(hwnd, message, wparam, lparam);
    }

    input_injector(win32::hwnd_t target_window, input_injector_args args) : target_window(target_window), injector_args(std::move(args))
    {
      simulated_inputs.reserve(64);
      siege::init_active_input_state();

      auto version_and_name = win32::get_xinput_version();

      if (version_and_name)
      {
        xinput_module.reset(::LoadLibraryExW(version_and_name->second.data(), nullptr, ::IsWindowsVistaOrGreater() ? LOAD_LIBRARY_SEARCH_SYSTEM32 : 0));
        xinput_get_state = xinput_module.GetProcAddress<decltype(xinput_get_state)>("XInputGetState");
        xinput_get_key_stroke = xinput_module.GetProcAddress<decltype(xinput_get_key_stroke)>("XInputGetKeystroke");
      }

      win32::set_window_subclass(target_window, sub_class_callback, (UINT_PTR)this, (DWORD_PTR)this);

      std::array<RAWINPUTDEVICE, 4> descriptors{};

      auto flags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
      descriptors[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
      descriptors[0].dwFlags = flags;
      descriptors[0].hwndTarget = target_window;

      descriptors[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;
      descriptors[1].dwFlags = flags;
      descriptors[1].hwndTarget = target_window;

      descriptors[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[2].usUsage = HID_USAGE_GENERIC_MOUSE;
      descriptors[2].dwFlags = flags;
      descriptors[2].hwndTarget = target_window;

      descriptors[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[3].usUsage = HID_USAGE_GENERIC_KEYBOARD;
      descriptors[3].dwFlags = flags | RIDEV_NOLEGACY;
      descriptors[3].hwndTarget = target_window;

      if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
      {
        auto error = GetLastError();
        DebugBreak();
        // registration failed. Call GetLastError for the cause of the error.
      }

      try
      {
        if (this->injector_args.launch_game_with_extension(&this->injector_args.args, &child_process) == S_OK && child_process.hProcess)
        {
          auto& state = siege::get_active_input_state();

          for (auto& group : state.groups)
          {
            group.process_id = child_process.dwProcessId;
            group.thread_id = child_process.dwThreadId;
          }

          process_handle = child_process.hProcess;
          thread_handle = child_process.hThread;
        }
      }
      catch (...)
      {
      }
    }

    ~input_injector()
    {
      win32::remove_window_subclass(target_window, sub_class_callback, (UINT_PTR)this);
      std::array<RAWINPUTDEVICE, 4> descriptors{};

      auto flags = RIDEV_REMOVE;
      descriptors[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
      descriptors[0].dwFlags = flags;

      descriptors[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;
      descriptors[1].dwFlags = flags;

      descriptors[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[2].usUsage = HID_USAGE_GENERIC_MOUSE;
      descriptors[2].dwFlags = flags;

      descriptors[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[3].usUsage = HID_USAGE_GENERIC_KEYBOARD;
      descriptors[3].dwFlags = flags;

      if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
      {
        auto error = GetLastError();
        DebugBreak();
        // registration failed. Call GetLastError for the cause of the error.
      }

      ::CloseHandle(process_handle);
      ::CloseHandle(thread_handle);
    }

    std::uint16_t normalise(std::int16_t value)
    {
      auto result = (std::uint16_t)32768 + (std::uint16_t)value;

      if (value < 0)
      {
        return std::numeric_limits<std::uint16_t>::max() - result;
      }

      return result;
    }

    auto calculate_deadzone(std::pair<short, short> x_y_pair, int deadzone)
    {
      auto [x, y] = x_y_pair;
      if (std::abs(x) < deadzone)
      {
        x = 0;
      }

      if (std::abs(y) < deadzone)
      {
        y = 0;
      }

      return std::make_pair(x, y);
    }

    std::optional<WORD> map_virtual_key(WORD vkey)
    {
      switch (vkey)
      {
      case VK_PAD_A:
        return VK_GAMEPAD_A;
      case VK_PAD_B:
        return VK_GAMEPAD_B;
      case VK_PAD_X:
        return VK_GAMEPAD_X;
      case VK_PAD_Y:
        return VK_GAMEPAD_Y;
      case VK_PAD_LTRIGGER:
        return VK_GAMEPAD_LEFT_TRIGGER;
      case VK_PAD_RTRIGGER:
        return VK_GAMEPAD_RIGHT_TRIGGER;
      case VK_PAD_LTHUMB_UP:
        return VK_GAMEPAD_LEFT_THUMBSTICK_UP;
      case VK_PAD_LTHUMB_DOWN:
        return VK_GAMEPAD_LEFT_THUMBSTICK_DOWN;
      case VK_PAD_LTHUMB_LEFT:
        return VK_GAMEPAD_LEFT_THUMBSTICK_LEFT;
      case VK_PAD_LTHUMB_RIGHT:
        return VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT;
      case VK_PAD_RTHUMB_UP:
        return VK_GAMEPAD_RIGHT_THUMBSTICK_UP;
      case VK_PAD_RTHUMB_DOWN:
        return VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN;
      case VK_PAD_RTHUMB_LEFT:
        return VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT;
      case VK_PAD_RTHUMB_RIGHT:
        return VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT;
      default:
        return std::nullopt;
      }
    }

    LRESULT wm_input(win32::input_message message)
    {
      DWORD exit_code = 0;
      if (::GetExitCodeProcess(child_process.hProcess, &exit_code) && exit_code != STILL_ACTIVE)
      {
        if (injector_args.on_process_closed)
        {
          injector_args.on_process_closed();
        }
        return 0;
      }

      RAWINPUTHEADER header{};

      UINT size = sizeof(header);
      if (::GetRawInputData(message.handle, RID_HEADER, &header, &size, sizeof(header)) == (UINT)-1)
      {
        return 0;
      }

      auto device_id = siege::find_device_id(header.hDevice);

      if (!device_id)
      {
        return 0;
      }

      if (header.dwType == RIM_TYPEKEYBOARD)
      {
        // TODO only send keyboard/mouse input if input filtering is enabled
        // RAWINPUT input{};
        // UINT size = sizeof(input);

        // if (::GetRawInputData(message.handle, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) > 0)
        //{
        //   INPUT temp{ .type = INPUT_KEYBOARD };
        //   temp.ki.dwExtraInfo = *device_id;
        //   //    temp.ki.wVk = input.data.keyboard.VKey;
        //   temp.ki.wScan = input.data.keyboard.MakeCode;
        //   temp.ki.dwFlags = KEYEVENTF_SCANCODE;

        //  if (input.data.keyboard.Flags & RI_KEY_BREAK)
        //  {
        //    temp.ki.dwFlags |= KEYEVENTF_KEYUP;
        //  }

        //  if (input.data.keyboard.Flags & RI_KEY_E0)
        //  {
        //    temp.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        //  }

        //  ::SendInput(1, &temp, sizeof(temp));
        // }

        return 0;
      }

      if (header.dwType == RIM_TYPEMOUSE)
      {
        RAWINPUT input{};
        UINT size = sizeof(input);

        if (::GetRawInputData(message.handle, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) > 0)
        {
          //        INPUT temp{ .type = INPUT_MOUSE };
          //          ::SendInput(1, &temp, sizeof(temp));
        }
        return 0;
      }

      simulated_inputs.clear();

      if (regular_controllers.contains(header.hDevice))
      {
        auto item = registered_controllers.find(header.hDevice);
        auto index = std::distance(item, registered_controllers.end()) - 1;
        JOYINFOEX info{
          .dwSize = sizeof(JOYINFOEX),
          .dwFlags = JOY_RETURNALL
        };

        if (joyGetPosEx(index, &info) == JOYERR_NOERROR)
        {
          auto& button_a = simulated_inputs.emplace_back();
          button_a.type = INPUT_KEYBOARD;
          button_a.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
          button_a.ki.wScan = 0x0039;// space

          if (info.dwButtons & JOY_BUTTON1)
          {
            button_a.ki.dwFlags = KEYEVENTF_SCANCODE;
          }

          auto& button_b = simulated_inputs.emplace_back();
          button_b.type = INPUT_KEYBOARD;

          button_b.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
          button_b.ki.wScan = 0x001D;// left control

          if (info.dwButtons & JOY_BUTTON2)
          {
            button_b.ki.dwFlags = KEYEVENTF_SCANCODE;
          }
        }

        if (!simulated_inputs.empty())
        {
          ::SendInput(simulated_inputs.size(), simulated_inputs.data(), sizeof(INPUT));
        }

        return 0;
      }

      if (controller_state.empty() || !xinput_get_state)
      {
        return 0;
      }

      XINPUT_STATE temp{};

      auto& mappings = injector_args.args.controller_to_send_input_mappings;

      for (auto& state : controller_state)
      {
        auto result = xinput_get_state(state.first, &temp);

        if (result != S_OK || state.second.dwPacketNumber == temp.dwPacketNumber)
        {
          continue;
        }

        auto [newLx, newLy] = calculate_deadzone(std::make_pair(temp.Gamepad.sThumbLX, temp.Gamepad.sThumbLY), XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        auto [oldLx, oldLy] = calculate_deadzone(std::make_pair(state.second.Gamepad.sThumbLX, state.second.Gamepad.sThumbLY), XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

        auto find_mapping = [&](auto vkey) {
          return std::find_if(mappings.begin(), mappings.end(), [&](auto& mapping) {
            return mapping.from_vkey == vkey;
          });
        };
        XINPUT_KEYSTROKE stroke{};

        if (xinput_get_key_stroke(state.first, 0, &stroke) != ERROR_SUCCESS)
        {
          continue;
        }

        auto mapped_vkey = map_virtual_key(stroke.VirtualKey);

        if (!mapped_vkey)
        {
          continue;
        }

        for (auto& mapping : mappings)
        {
          if (!mapping.from_vkey || !mapping.to_vkey)
          {
            break;
          }

          if (mapping.from_vkey != mapped_vkey)
          {
            continue;
          }

          if (!(mapping.to_context == siege::platform::hardware_context::global || mapping.to_context == siege::platform::hardware_context::keyboard || mapping.to_context == siege::platform::hardware_context::mouse))
          {
            continue;
          }

          if (stroke.Flags & XINPUT_KEYSTROKE_KEYUP)
          {
            simulated_inputs.emplace_back(vk_to_input(mapping.to_vkey, mapping.to_context, *device_id, input_state::up));
          }
          else if (stroke.Flags & XINPUT_KEYSTROKE_KEYDOWN)
          {
            simulated_inputs.emplace_back(vk_to_input(mapping.to_vkey, mapping.to_context, *device_id, input_state::down));
          }
        }

        if (!simulated_inputs.empty())
        {
          assert(::SendInput(simulated_inputs.size(), simulated_inputs.data(), sizeof(INPUT)) == simulated_inputs.size());
        }

        state.second = temp;
      }

      return 0;
    }

    LRESULT wm_input_device_change(win32::input_device_change_message message)
    {
      DWORD exit_code = 0;
      if (::GetExitCodeProcess(child_process.hProcess, &exit_code) && exit_code != STILL_ACTIVE)
      {
        if (injector_args.on_process_closed)
        {
          injector_args.on_process_closed();
        }
        return 0;
      }

      if (message.code == GIDC_ARRIVAL)
      {
        auto& state = siege::get_active_input_state();

        auto device_iter = std::find_if(state.devices.begin(), state.devices.end(), [](auto& device) {
          return device.id == 0;
        });

        // Max devices reached
        if (device_iter == state.devices.end())
        {
          return 0;
        }

        RID_DEVICE_INFO device_info{};
        UINT size = sizeof(device_info);

        if (::GetRawInputDeviceInfoW(message.device_handle, RIDI_DEVICEINFO, &device_info, &size) > 0)
        {
          update_device_id(RAWINPUTDEVICELIST{
            .hDevice = message.device_handle,
            .dwType = device_info.dwType,
          });

          if (device_info.dwType == RIM_TYPEMOUSE)
          {
            registered_mice.insert(message.device_handle);
            return 0;
          }
          else if (device_info.dwType == RIM_TYPEKEYBOARD)
          {
            registered_keyboards.insert(message.device_handle);
            return 0;
          }
        }

        registered_controllers.insert(message.device_handle);

        std::vector<wchar_t> device_buffer(255, '\0');
        size = device_buffer.size();

        if (::GetRawInputDeviceInfoW(message.device_handle, RIDI_DEVICENAME, device_buffer.data(), &size) > 0)
        {
          std::wstring_view device_path(device_buffer.data());

          if (device_path.rfind(L"IG_") == std::wstring_view::npos)
          {
            regular_controllers.insert(message.device_handle);
          }
        }

        XINPUT_STATE temp;

        for (auto i = 0; i < XUSER_MAX_COUNT; ++i)
        {
          if (xinput_get_state && xinput_get_state(i, &temp) == S_OK)
          {
            controller_state.emplace(i, temp);
          }
        }
      }
      else if (message.code == GIDC_REMOVAL)
      {
        remove_device(message.device_handle);
        registered_mice.erase(message.device_handle);
        registered_keyboards.erase(message.device_handle);
        registered_controllers.erase(message.device_handle);
        registered_controllers.erase(message.device_handle);
        regular_controllers.erase(message.device_handle);

        std::set<int> controllers_to_remove;

        for (auto& kv : controller_state)
        {
          if (xinput_get_state && xinput_get_state(kv.first, &kv.second) == ERROR_DEVICE_NOT_CONNECTED)
          {
            controllers_to_remove.insert(kv.first);
          }
        }

        for (auto& key : controllers_to_remove)
        {
          controller_state.erase(key);
        }
      }
      return 0;
    }
  };

}// namespace siege

#endif// !SIEGE_LAUNCHER_INPUT_INJECTOR_HPP
