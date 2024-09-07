
#include <windows.h>
#include <detours.h>
#include <hidusage.h>
#include <cassert>
#include <fstream>
#include <codecvt>

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
        if (header.dwType == RIM_TYPEKEYBOARD)
        {
          RAWINPUT input{};
          size = sizeof(input);

          if (::GetRawInputData(hinput, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) > 0 && input.data.keyboard.ExtraInformation == 266)
          {
            auto vk = MapVirtualKeyW(input.data.keyboard.MakeCode, MAPVK_VSC_TO_VK);

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
        }
      }
    }
  }

  return ::CallNextHookEx(nullptr, code, wParam, lParam);
}


BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
  if (DetourIsHelperProcess())
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

      assert(RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == TRUE);
    }
  }

  return TRUE;
}
}
