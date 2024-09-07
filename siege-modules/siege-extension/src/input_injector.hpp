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

namespace siege
{
  struct input_injector : win32::window_ref
  {
    PROCESS_INFORMATION child_process;
    win32::com::com_ptr<IDispatch> script_host;
    std::vector<INPUT> simulated_inputs;
    std::set<HANDLE> registered_controllers;
    std::set<HANDLE> regular_controllers;
    std::map<int, XINPUT_STATE> controller_state;

    std::map<std::wstring_view, DISPID> func_ids = {
      std::make_pair(std::wstring_view{ L"+forward" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"-forward" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"+back" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"-back" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"+moveleft" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"-moveleft" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"+moveright" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"-moveright" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"+left" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"-left" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"+right" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"-right" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"+lookup" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"-lookup" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"+lookdown" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"-lookdown" }, DISPID{ DISPID_UNKNOWN }),
      std::make_pair(std::wstring_view{ L"echo" }, DISPID{ DISPID_UNKNOWN }),
    };

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

    input_injector(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self), child_process{}, controller_state{}
    {
      simulated_inputs.reserve(64);
    }

    auto wm_create()
    {
      std::array<RAWINPUTDEVICE, 2> descriptors{};

      auto flags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
      descriptors[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
      descriptors[0].dwFlags = flags;
      descriptors[0].hwndTarget = *this;

      descriptors[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;
      descriptors[1].dwFlags = flags;
      descriptors[1].hwndTarget = *this;

      if (RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
      {
        auto error = GetLastError();
        DebugBreak();
        // registration failed. Call GetLastError for the cause of the error.
      }

      try
      {
        siege::platform::game_extension_module extension("siege-extension-soldier-of-fortune.dll");

        std::array<const wchar_t*, 3> args{ { L"+set",
          L"console",
          L"1" } };

        if (extension.launch_game_with_extension(L"C:\\Program Files (x86)\\GOG Galaxy\\Games\\Soldier of Fortune\\SoF.exe", args.size(), args.data(), &child_process) == S_OK && child_process.hProcess)
        {
          WaitForSingleObject(child_process.hProcess, 1000);

          if (extension.get_game_script_host(L"SoldierOfFortune", script_host.put()) == S_OK)
          {
            DISPID id = 0;

            for (auto& pair : func_ids)
            {
              win32::com::Variant name(pair.first);

              auto hresult = script_host->GetIDsOfNames(IID_NULL, &name.bstrVal, 1, LOCALE_USER_DEFAULT, &pair.second);

              if (hresult != S_OK)
              {
                DebugBreak();
              }
            }
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
      std::array<RAWINPUTDEVICE, 2> descriptors{};

      auto flags = RIDEV_REMOVE;
      descriptors[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
      descriptors[0].dwFlags = flags;

      descriptors[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
      descriptors[1].usUsage = HID_USAGE_GENERIC_JOYSTICK;
      descriptors[1].dwFlags = flags;

      if (RegisterRawInputDevices(descriptors.data(), descriptors.size(), sizeof(RAWINPUTDEVICE)) == FALSE)
      {
        auto error = GetLastError();
        DebugBreak();
        // registration failed. Call GetLastError for the cause of the error.
      }

      return 0;
    }

    void post_input(::GUITHREADINFO gui_thread)
    {
      for (auto& input : simulated_inputs)
      {
        if (input.type == INPUT_KEYBOARD)
        {
          auto vk = MapVirtualKeyW(input.ki.wScan, MAPVK_VSC_TO_VK);
          LPARAM lParam = 0;
          lParam |= (input.ki.dwFlags & KEYEVENTF_KEYUP) ? 1 << 31 : 0;// Transition state
          lParam |= (input.ki.dwFlags & KEYEVENTF_KEYUP) ? 1 << 30 : 0;// Previous key state
          lParam |= input.ki.wScan << 16;// Scan code

          WORD keyFlags = HIWORD(lParam);

          WORD scanCode = LOBYTE(keyFlags);

          if (input.ki.dwFlags & KEYEVENTF_KEYUP)
          {
            ::PostMessageW(gui_thread.hwndFocus, WM_KEYUP, vk, lParam);
          }
          else
          {

            ::PostMessageW(gui_thread.hwndFocus, WM_KEYDOWN, vk, lParam);
          }
        }
      }

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
#ifndef _DEBUG
      auto active_window = ::GetForegroundWindow();

      if (!active_window)
      {
        return 0;
      }

      DWORD process_id = 0;
      if (::GetWindowThreadProcessId(active_window, &process_id) == 0)
      {
        return 0;
      }

      if (process_id != child_process.dwProcessId)
      {
        return 0;
      }
#endif// ! DEBUG


      RAWINPUTHEADER header{};

      UINT size = sizeof(header);
      if (::GetRawInputData(message.handle, RID_HEADER, &header, &size, sizeof(header)) == (UINT)-1)
      {
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
        }

        if (temp.Gamepad.wButtons & XINPUT_GAMEPAD_B)
        {
          auto& crouch_button = simulated_inputs.emplace_back();
          crouch_button.type = INPUT_KEYBOARD;
          crouch_button.ki.dwFlags = KEYEVENTF_SCANCODE;
          crouch_button.ki.wScan = 0x001D;// left control
        }
        else if (state.second.Gamepad.wButtons & XINPUT_GAMEPAD_B)
        {
          auto& crouch_button = simulated_inputs.emplace_back();
          crouch_button.type = INPUT_KEYBOARD;
          crouch_button.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
          crouch_button.ki.wScan = 0x001D;// left control
        }


        if (temp.Gamepad.wButtons & XINPUT_GAMEPAD_A)
        {
          auto& jump_button = simulated_inputs.emplace_back();
          jump_button.type = INPUT_KEYBOARD;
          jump_button.ki.wScan = 0x0039;// space
          jump_button.ki.dwFlags = KEYEVENTF_SCANCODE;
        }
        else if (state.second.Gamepad.wButtons & XINPUT_GAMEPAD_A)
        {
          auto& jump_button = simulated_inputs.emplace_back();
          jump_button.type = INPUT_KEYBOARD;
          jump_button.ki.wScan = 0x0039;// space
          jump_button.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
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
            script_host->Invoke(func_ids[L"-lookup"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRy <= 0 && action_states[L"+lookdown"])
          {
            action_states[L"+lookdown"] = false;
            script_host->Invoke(func_ids[L"-lookdown"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRy > 0 && !action_states[L"+lookdown"])
          {
            action_states[L"+lookdown"] = true;
            script_host->Invoke(func_ids[L"+lookdown"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRy < 0 && !action_states[L"+lookup"])
          {
            action_states[L"+lookup"] = true;
            script_host->Invoke(func_ids[L"+lookup"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }
        }
        if (newRx != oldRx)
        {
          if (newRx >= 0 && action_states[L"+left"])
          {
            action_states[L"+left"] = false;
            script_host->Invoke(func_ids[L"-left"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRx <= 0 && action_states[L"+right"])
          {
            action_states[L"+right"] = false;
            script_host->Invoke(func_ids[L"-right"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRx < 0 && !action_states[L"+left"])
          {
            action_states[L"+left"] = true;
            script_host->Invoke(func_ids[L"+left"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
          }

          if (newRx > 0 && !action_states[L"+right"])
          {
            action_states[L"+right"] = true;
            script_host->Invoke(func_ids[L"+right"], IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &invoke_result, nullptr, nullptr);
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

        GUITHREADINFO gui_thread{ .cbSize = sizeof(GUITHREADINFO) };
        if (!simulated_inputs.empty() && GetGUIThreadInfo(child_process.dwThreadId, &gui_thread))
        {
          SendInput(simulated_inputs.size(), simulated_inputs.data(), sizeof(INPUT));
        }

        state.second = temp;
      }

      return 0;
    }

    auto wm_input_device_change(win32::input_device_change_message message)
    {
      if (message.code == GIDC_ARRIVAL)
      {
        registered_controllers.insert(message.device_handle);

        std::vector<wchar_t> device_buffer(255, '\0');
        UINT size = 255;

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
      return std::nullopt;
    }
  };

}// namespace siege

#endif// !SIEGE_LAUNCHER_INPUT_INJECTOR_HPP
