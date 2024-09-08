#ifndef SIEGE_LAUNCHER_INPUT_INJECTOR_HPP
#define SIEGE_LAUNCHER_INPUT_INJECTOR_HPP

#include <siege/platform/win/desktop/window.hpp>
#include <siege/platform/win/core/com/client.hpp>
#include <siege/platform/extension_module.hpp>
#include <optional>
#include <array>
#include <set>
#include <map>
#include <bitset>
#include <hidusage.h>
#include <xinput.h>
#include <mmsystem.h>
#include "input-filter.hpp"

namespace siege
{
  struct input_injector_args
  {
    std::filesystem::path exe_path;
    std::filesystem::path extension_path;
    std::vector<std::wstring> command_line_args;
    std::wstring script_host;
  };

  struct input_injector : win32::window_ref
  {
    input_injector_args injector_args;
    PROCESS_INFORMATION child_process;
    std::vector<INPUT> simulated_inputs;

    std::set<HANDLE> registered_mice;
    std::set<HANDLE> registered_keyboards;
    std::set<HANDLE> registered_controllers;
    std::set<HANDLE> regular_controllers;
    std::map<int, XINPUT_STATE> controller_state;

    std::map<std::wstring_view, bool> action_states = {
      std::make_pair(std::wstring_view{ L"+forward" }, false),
      std::make_pair(std::wstring_view{ L"+back" }, false),
      std::make_pair(std::wstring_view{ L"+moveleft" }, false),
      std::make_pair(std::wstring_view{ L"+moveright" }, false),
      std::make_pair(std::wstring_view{ L"+left" }, false),
      std::make_pair(std::wstring_view{ L"+right" }, false),
      std::make_pair(std::wstring_view{ L"+lookup" }, false),
      std::make_pair(std::wstring_view{ L"+lookdown" }, false),
    };

    input_injector(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self), child_process{}, controller_state{}
    {
      simulated_inputs.reserve(64);
      injector_args = std::move(*(input_injector_args*)params.lpCreateParams);
      siege::init_active_input_state();
    }

    auto wm_create()
    {
      std::array<RAWINPUTDEVICE, 4> descriptors{};

      auto flags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
      descriptors[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
      descriptors[0].dwFlags = flags;
      descriptors[0].hwndTarget = *this;

      descriptors[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;
      descriptors[1].dwFlags = flags;
      descriptors[1].hwndTarget = *this;

      descriptors[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[2].usUsage = HID_USAGE_GENERIC_MOUSE;
      descriptors[2].dwFlags = flags | RIDEV_NOLEGACY;
      descriptors[2].hwndTarget = *this;

      descriptors[3].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[3].usUsage = HID_USAGE_GENERIC_KEYBOARD;
      descriptors[3].dwFlags = flags | RIDEV_NOLEGACY;
      descriptors[3].hwndTarget = *this;

      if (::RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
      {
        auto error = GetLastError();
        DebugBreak();
        // registration failed. Call GetLastError for the cause of the error.
      }

      try
      {
        siege::platform::game_extension_module extension(injector_args.extension_path);

        std::vector<const wchar_t*> args(injector_args.command_line_args.size(), nullptr);
        std::transform(injector_args.command_line_args.begin(), injector_args.command_line_args.end(), args.begin(), [](auto& arg) {
          return arg.c_str();
        });

        if (extension.launch_game_with_extension(injector_args.exe_path.c_str(), args.size(), args.data(), &child_process) == S_OK && child_process.hProcess)
        {
          auto& state = siege::get_active_input_state();

          for (auto& group : state.groups)
          {
            group.process_id = child_process.dwProcessId;
            group.thread_id = child_process.dwThreadId;
          }
        }
      }
      catch (...)
      {
      }

      return 0;
    }

    auto wm_destroy()
    {
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

      return 0;
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

    auto wm_input(win32::input_message message)
    {
      DWORD exit_code = 0;
      if (::GetExitCodeProcess(child_process.hProcess, &exit_code) && exit_code != STILL_ACTIVE)
      {
        if (::EndDialog(*this, 0) || ::DestroyWindow(*this))
        {
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
        RAWINPUT input{};
        UINT size = sizeof(input);

        if (::GetRawInputData(message.handle, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) > 0)
        {
          INPUT temp{ .type = INPUT_KEYBOARD };
          temp.ki.dwExtraInfo = *device_id;
      //    temp.ki.wVk = input.data.keyboard.VKey;
          temp.ki.wScan = input.data.keyboard.MakeCode;
          temp.ki.dwFlags = KEYEVENTF_SCANCODE;

          if (input.data.keyboard.Flags & RI_KEY_BREAK)
          {
            temp.ki.dwFlags |= KEYEVENTF_KEYUP;
          }

          if (input.data.keyboard.Flags & RI_KEY_E0)
          {
            temp.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
          }

          ::SendInput(1, &temp, sizeof(temp));
        }

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

      if (controller_state.empty())
      {
        return 0;
      }

      XINPUT_STATE temp{};

      for (auto& state : controller_state)
      {
        auto result = XInputGetState(state.first, &temp);

        if (result != S_OK || state.second.dwPacketNumber == temp.dwPacketNumber)
        {
          continue;
        }

        auto [newLx, newLy] = calculate_deadzone(std::make_pair(temp.Gamepad.sThumbLX, temp.Gamepad.sThumbLY), XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        auto [oldLx, oldLy] = calculate_deadzone(std::make_pair(state.second.Gamepad.sThumbLX, state.second.Gamepad.sThumbLY), XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

        if (newLx != oldLx)
        {
          auto& button_a = simulated_inputs.emplace_back();
          button_a.type = INPUT_KEYBOARD;
          auto& button_b = simulated_inputs.emplace_back();
          button_b.type = INPUT_KEYBOARD;

          button_a.ki.dwExtraInfo = *device_id;
          button_b.ki.dwExtraInfo = *device_id;

          if (newLx == 0)
          {
            button_a.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            button_a.ki.wScan = 30;// A

            button_b.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            button_b.ki.wScan = 32;// D
          }
          else if (newLx < 0)
          {
            button_a.ki.dwFlags = KEYEVENTF_SCANCODE;
            button_a.ki.wScan = 30;
            button_b.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            button_b.ki.wScan = 32;
          }
          else if (newLx > 0)
          {
            button_a.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            button_a.ki.wScan = 30;// A
            button_b.ki.dwFlags = KEYEVENTF_SCANCODE;
            button_b.ki.wScan = 32;// D
          }
        }

        if (newLy != oldLy)
        {
          auto& button_a = simulated_inputs.emplace_back();
          button_a.type = INPUT_KEYBOARD;

          auto& button_b = simulated_inputs.emplace_back();
          button_b.type = INPUT_KEYBOARD;

          button_a.ki.dwExtraInfo = *device_id;
          button_b.ki.dwExtraInfo = *device_id;

          if (newLy == 0)
          {
            button_a.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            button_a.ki.wScan = 0x0011;// W
            button_b.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            button_b.ki.wScan = 0x001F;// S
          }
          else if (newLy < 0)
          {
            button_a.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            button_a.ki.wScan = 0x0011;// W
            button_b.ki.dwFlags = KEYEVENTF_SCANCODE;
            button_b.ki.wScan = 0x001F;// S
          }
          else if (newLy > 0)
          {
            button_a.ki.dwFlags = KEYEVENTF_SCANCODE;
            button_a.ki.wScan = 0x0011;// W
            button_b.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            button_b.ki.wScan = 0x001F;// S
          }
        }

        constexpr static auto half_size = std::numeric_limits<short>().max() / 2;

        if (newLx != oldLx || newLy != oldLy)
        {
          auto& shift_key = simulated_inputs.emplace_back();
          shift_key.type = INPUT_KEYBOARD;
          shift_key.ki.wScan = 0x002A;
          if (std::abs(newLx) > half_size || std::abs(newLy) > half_size)
          {
            shift_key.ki.dwFlags = KEYEVENTF_SCANCODE;
          }
          else
          {
            shift_key.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
          }

          shift_key.ki.dwExtraInfo = *device_id;
        }

        if (temp.Gamepad.wButtons & XINPUT_GAMEPAD_B)
        {
          auto& crouch_button = simulated_inputs.emplace_back();
          crouch_button.type = INPUT_KEYBOARD;
          crouch_button.ki.dwFlags = KEYEVENTF_SCANCODE;
          crouch_button.ki.wScan = 0x001D;// left control
          crouch_button.ki.dwExtraInfo = *device_id;
        }
        else if (state.second.Gamepad.wButtons & XINPUT_GAMEPAD_B)
        {
          auto& crouch_button = simulated_inputs.emplace_back();
          crouch_button.type = INPUT_KEYBOARD;
          crouch_button.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
          crouch_button.ki.wScan = 0x001D;// left control
          crouch_button.ki.dwExtraInfo = *device_id;
        }

        if (temp.Gamepad.wButtons & XINPUT_GAMEPAD_A)
        {
          auto& jump_button = simulated_inputs.emplace_back();
          jump_button.type = INPUT_KEYBOARD;
          jump_button.ki.wScan = 0x0039;// space
          jump_button.ki.dwFlags = KEYEVENTF_SCANCODE;
          jump_button.ki.dwExtraInfo = *device_id;
        }
        else if (state.second.Gamepad.wButtons & XINPUT_GAMEPAD_A)
        {
          auto& jump_button = simulated_inputs.emplace_back();
          jump_button.type = INPUT_KEYBOARD;
          jump_button.ki.wScan = 0x0039;// space
          jump_button.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
          jump_button.ki.dwExtraInfo = *device_id;
        }

        auto [newRx, newRy] = calculate_deadzone(std::make_pair(temp.Gamepad.sThumbRX, temp.Gamepad.sThumbRY), XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        auto [oldRx, oldRy] = calculate_deadzone(std::make_pair(state.second.Gamepad.sThumbRX, state.second.Gamepad.sThumbRY), XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

        DISPPARAMS params{};
        win32::com::Variant invoke_result;

        if (newRy != oldRy)
        {
          if (newRy >= 0 && action_states[L"+lookup"])
          {
            action_states[L"+lookup"] = false;
            // script_host->Invoke(func_ids[L"-lookup"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRy <= 0 && action_states[L"+lookdown"])
          {
            action_states[L"+lookdown"] = false;
            // script_host->Invoke(func_ids[L"-lookdown"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRy > 0 && !action_states[L"+lookdown"])
          {
            action_states[L"+lookdown"] = true;
            // script_host->Invoke(func_ids[L"+lookdown"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRy < 0 && !action_states[L"+lookup"])
          {
            action_states[L"+lookup"] = true;
            // script_host->Invoke(func_ids[L"+lookup"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }
        }
        if (newRx != oldRx)
        {
          if (newRx >= 0 && action_states[L"+left"])
          {
            action_states[L"+left"] = false;
            // script_host->Invoke(func_ids[L"-left"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRx <= 0 && action_states[L"+right"])
          {
            action_states[L"+right"] = false;
            // script_host->Invoke(func_ids[L"-right"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRx < 0 && !action_states[L"+left"])
          {
            action_states[L"+left"] = true;
            // script_host->Invoke(func_ids[L"+left"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRx > 0 && !action_states[L"+right"])
          {
            action_states[L"+right"] = true;
            //  script_host->Invoke(func_ids[L"+right"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }
        }

        if (state.second.Gamepad.bLeftTrigger != temp.Gamepad.bLeftTrigger)
        {
          auto& mouse = simulated_inputs.emplace_back();

          if (temp.Gamepad.bLeftTrigger > 127)
          {
            mouse.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
          }
          else
          {
            mouse.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
          }
        }

        if (state.second.Gamepad.bRightTrigger != temp.Gamepad.bRightTrigger)
        {
          auto& mouse = simulated_inputs.emplace_back();

          if (temp.Gamepad.bRightTrigger > 127)
          {
            mouse.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
          }
          else
          {
            mouse.mi.dwFlags = MOUSEEVENTF_LEFTUP;
          }
        }

        if (!simulated_inputs.empty())
        {
          ::SendInput(simulated_inputs.size(), simulated_inputs.data(), sizeof(INPUT));
        }

        state.second = temp;
      }

      return 0;
    }

    auto wm_input_device_change(win32::input_device_change_message message)
    {
      DWORD exit_code = 0;
      if (::GetExitCodeProcess(child_process.hProcess, &exit_code) && exit_code != STILL_ACTIVE)
      {
        if (::EndDialog(*this, 0) || ::DestroyWindow(*this))
        {
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
          if (XInputGetState(i, &temp) == S_OK)
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
          if (XInputGetState(kv.first, &kv.second) == ERROR_DEVICE_NOT_CONNECTED)
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
