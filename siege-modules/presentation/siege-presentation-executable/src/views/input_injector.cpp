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
#include <siege/platform/win/common_controls.hpp>
#include "input_injector.hpp"
#include "exe_shared.hpp"

namespace siege::views
{
  using controller_state = siege::views::controller_state;

  enum class input_state
  {
    down,
    up
  };

  ::INPUT vk_to_input(WORD vkey, siege::platform::hardware_context context, std::uint32_t device_id, input_state state, std::optional<WORD> intensity = std::nullopt);

  struct input_injector
  {
    win32::hwnd_t target_window;
    input_injector_args injector_args;
    HANDLE process_handle{};
    HANDLE thread_handle{};
    PROCESS_INFORMATION child_process{};
    std::vector<INPUT> simulated_inputs;

    std::set<HANDLE> registered_mice;
    std::set<HANDLE> registered_keyboards;
    std::map<HANDLE, controller_state> registered_controllers;
    std::map<std::string_view, bool> action_states;

    input_injector(win32::hwnd_t target_window, input_injector_args args) : target_window(target_window), injector_args(std::move(args))
    {
      simulated_inputs.reserve(64);
      siege::init_active_input_state();

      win32::set_window_subclass(target_window, sub_class_callback, (UINT_PTR)this, (DWORD_PTR)this);

      DWORD flags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
      std::array<RAWINPUTDEVICE, 4> descriptors{ {
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_GAMEPAD,
          .dwFlags = flags,
          .hwndTarget = target_window },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_JOYSTICK,
          .dwFlags = flags,
          .hwndTarget = target_window },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_MOUSE,
          .dwFlags = flags,
          .hwndTarget = target_window },
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_KEYBOARD,
          .dwFlags = flags | RIDEV_NOLEGACY,
          .hwndTarget = target_window },
      } };

      if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
      {
        auto error = GetLastError();
        DebugBreak();
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

      std::array<RAWINPUTDEVICE, 4> descriptors{ {
        { .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_GAMEPAD,
          .dwFlags = RIDEV_REMOVE },
        {
          .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_JOYSTICK,
          .dwFlags = RIDEV_REMOVE,
        },
        {
          .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_MOUSE,
          .dwFlags = RIDEV_REMOVE,
        },
        {
          .usUsagePage = HID_USAGE_PAGE_GENERIC,
          .usUsage = HID_USAGE_GENERIC_KEYBOARD,
          .dwFlags = RIDEV_REMOVE,
        },
      } };

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

      auto info = registered_controllers.find(header.hDevice);

      if (info == registered_controllers.end())
      {
        return 0;
      }

      XINPUT_STATE temp{};

      auto& mappings = injector_args.args.controller_to_send_input_mappings;

      auto new_state = siege::views::get_current_state_for_handle(info->second, message.handle);

      if (new_state.dwPacketNumber == info->second.last_state.dwPacketNumber)
      {
        return 0;
      }

      auto changes = siege::views::get_changes(info->second.last_state, new_state, std::span{ info->second.buffer });

      if (changes.size() == 0)
      {
        return 0;
      }

      for (auto mapped_vkey : changes)
      {
        for (auto& mapping : mappings)
        {
          if (!mapping.from_vkey || !mapping.to_vkey)
          {
            break;
          }

          if (mapping.from_vkey != mapped_vkey.first)
          {
            continue;
          }

          if (!(mapping.to_context == siege::platform::hardware_context::global || mapping.to_context == siege::platform::hardware_context::keyboard || mapping.to_context == siege::platform::hardware_context::mouse))
          {
            continue;
          }

          if (mapped_vkey.second > 0)
          {
            simulated_inputs.emplace_back(vk_to_input(mapping.to_vkey, mapping.to_context, *device_id, input_state::down));
          }
          else
          {
            simulated_inputs.emplace_back(vk_to_input(mapping.to_vkey, mapping.to_context, *device_id, input_state::up));
          }
        }
      }


      if (!simulated_inputs.empty())
      {
        assert(::SendInput(simulated_inputs.size(), simulated_inputs.data(), sizeof(INPUT)) == simulated_inputs.size());
      }

      info->second.last_state = new_state;

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

        auto info = siege::views::controller_info_for_raw_input_device_handle(message.device_handle);

        if (info)
        {
          registered_controllers[message.device_handle] = controller_state{ *info };
        }
      }
      else if (message.code == GIDC_REMOVAL)
      {
        remove_device(message.device_handle);
        registered_mice.erase(message.device_handle);
        registered_keyboards.erase(message.device_handle);
        registered_controllers.erase(message.device_handle);
      }
      return 0;
    }

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
  };

  ::INPUT vk_to_input(WORD vkey, siege::platform::hardware_context context, std::uint32_t device_id, input_state state, std::optional<WORD> intensity)
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

  std::any bind_to_window(win32::window_ref ref, input_injector_args args)
  {
    return input_injector(std::move(ref), args);
  }

}// namespace siege
