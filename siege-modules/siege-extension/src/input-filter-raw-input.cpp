
#include <windows.h>
#include <detours.h>
#include <hidusage.h>
#undef NDEBUG 
#include <cassert> 
#include <fstream>
#include <codecvt>
#include "input-filter.hpp"

extern "C" {

LRESULT CALLBACK CBTProc(
  int code,
  WPARAM wParam,
  LPARAM lParam)
{
  if (code == HCBT_SETFOCUS && wParam)
  {
    RAWINPUTDEVICE Rid[1];

    // Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    // Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    // Rid[0].dwFlags = RIDEV_NOLEGACY;
    // Rid[0].hwndTarget = 0;

    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    Rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    Rid[0].dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY;
    Rid[0].hwndTarget = (HWND)wParam;

    // Input Sink lets this window always receive background input.
    // Maybe HCBT_ACTIVATE, HCBT_MINMAX or HCBT_MOVESIZE might work better (or a comination of all of them).
    // However, updating the target based on focus is similar way that Windows does it except:
    // We don't update raw input when focus is lost, meaning that the last HWND will keep receiving input messages.
    // This is exactly what we want, because if we were simulating split-screen, then the game will keep receiving input.
    // The only downside is that if the Window is a child Window, then maybe the parent won't get the input.
    // Something to work on but most games tend to only have one root Window anyway.
    ::RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
  }

  return ::CallNextHookEx(nullptr, code, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(
  int code,
  WPARAM wParam,
  LPARAM lParam)
{
  if (code == HC_ACTION && wParam == PM_REMOVE)
  {
    auto* message = (MSG*)lParam;

    if (message->message == WM_INPUT)
    {
      auto* hinput = (HRAWINPUT)message->lParam;

      RAWINPUTHEADER header{};

      UINT size = sizeof(header);

      if (::GetRawInputData(hinput, RID_HEADER, &header, &size, sizeof(RAWINPUTHEADER)) > 0)
      {
        if (header.dwType == RIM_TYPEKEYBOARD || header.dwType == RIM_TYPEMOUSE)
        {
          RAWINPUT input{};
          size = sizeof(input);

          if (::GetRawInputData(hinput, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) > 0)
          {
            auto& state = siege::get_active_input_state();

            auto device_id = 0u;

            if (header.dwType == RIM_TYPEKEYBOARD)
            {
              device_id = input.data.keyboard.ExtraInformation;
            }
            else
            {
              device_id = input.data.mouse.ulExtraInformation;
            }

            auto device_iter = std::find_if(state.devices.begin(), state.devices.end(), [&](auto& device) {
              return (device.type == header.dwType || device.type == RIM_TYPEHID) && device.id == device_id && device.enabled == 1;
            });

            if (device_iter == state.devices.end())
            {
              goto next_hook;
            }

            auto group_iter = std::find_if(state.groups.begin(), state.groups.end(), [&, thread_id = ::GetCurrentThreadId()](auto& group) {
              return group.id == device_iter->group_id && group.enabled == 1 && group.thread_id == thread_id;
            });

            if (group_iter == state.groups.end())
            {
              goto next_hook;
            }

            if (header.dwType == RIM_TYPEKEYBOARD)
            {
              // TODO ::SetKeyboardState for more nosy games
              auto vk = ::MapVirtualKeyW(input.data.keyboard.MakeCode, MAPVK_VSC_TO_VK);

              LPARAM lParam = 0;
              lParam |= (input.data.keyboard.Flags & RI_KEY_BREAK) ? 1 << 31 : 0;// Transition state
              lParam |= (input.data.keyboard.Flags & RI_KEY_BREAK) ? 1 << 30 : 0;// Previous key state
              lParam |= input.data.keyboard.MakeCode << 16;// Scan code

              if (input.data.keyboard.Flags & RI_KEY_BREAK)
              {
                ::PostMessageW(message->hwnd, WM_KEYUP, vk, lParam);
              }
              else
              {
                ::PostMessageW(message->hwnd, WM_KEYDOWN, vk, lParam);
              }
            }
            else
            {
                // TODO mouse messages
            }
          }
        }
      }
    }
  }

next_hook:

  return ::CallNextHookEx(nullptr, code, wParam, lParam);
}


BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
  if (::DetourIsHelperProcess())
  {
    return TRUE;
  }

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    static HHOOK cbt_hook = nullptr;
    static HHOOK get_message_hook = nullptr;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      RAWINPUTDEVICE Rid[1];

      // Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      // Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
      // Rid[0].dwFlags = RIDEV_NOLEGACY;
      // Rid[0].hwndTarget = 0;

      Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      Rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
      Rid[0].dwFlags = RIDEV_NOLEGACY;
      Rid[0].hwndTarget = 0;

      // first init to disable all legacy input
      // then the CBT hook will handle setting the hwndTarget for
      // receiving foreground and background input.
      if (::RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
      {
        return FALSE;
      }

      cbt_hook = ::SetWindowsHookExW(WH_CBT, CBTProc, hinstDLL, ::GetCurrentThreadId());

      if (cbt_hook == nullptr)
      {
        return FALSE;
      }

      get_message_hook = ::SetWindowsHookExW(WH_GETMESSAGE, GetMsgProc, hinstDLL, ::GetCurrentThreadId());

      if (get_message_hook == nullptr)
      {
        assert(::UnhookWindowsHookEx(cbt_hook) == TRUE);
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      assert(::UnhookWindowsHookEx(cbt_hook) == TRUE);
      assert(::UnhookWindowsHookEx(get_message_hook) == TRUE);

      RAWINPUTDEVICE Rid[1];

      Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      Rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
      Rid[0].dwFlags = RIDEV_REMOVE;
      Rid[0].hwndTarget = 0;

      assert(::RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == TRUE);
    }
  }

  return TRUE;
}
}
