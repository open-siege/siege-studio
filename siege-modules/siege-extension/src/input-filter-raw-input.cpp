
#include <windows.h>
#include <detours.h>
#include <hidusage.h>
#undef NDEBUG
#include <cassert>
#include <fstream>
#include <algorithm>
#include <optional>
#include "input-filter.hpp"

extern "C" {

struct direct_input_hook
{
  HOOKPROC proc;
  HHOOK handle;
};

static std::optional<direct_input_hook> dinput_keyboard_hook = std::nullopt;

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

LRESULT CALLBACK LowLevelKeyboardProc(
  int code,
  WPARAM wParam,
  LPARAM lParam)
{
  if (code == HC_ACTION && lParam)
  {
    auto* info = (KBDLLHOOKSTRUCT*)lParam;

    auto& state = siege::get_active_input_state();

    auto device_id = LOWORD(info->dwExtraInfo);

    auto device_iter = std::find_if(state.devices.begin(), state.devices.end(), [&](auto& device) {
      return (device.type == RIM_TYPEKEYBOARD || device.type == RIM_TYPEHID) && device.id == device_id && device.enabled == 1;
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

    auto vk = info->vkCode ? info->vkCode : ::MapVirtualKeyW(info->scanCode, MAPVK_VSC_TO_VK);

    LPARAM lParam = 0;
    lParam |= (info->flags & LLKHF_UP) ? 1 << 31 : 0;// Transition state
    lParam |= (info->flags & LLKHF_UP) ? 1 << 30 : 0;// Previous key state
    lParam |= (info->flags & LLKHF_EXTENDED) ? 1 << 24 : 0;
    lParam |= info->scanCode << 16;// Scan code

    thread_local std::array<WORD, 256> extended_states{};
    WORD extra_state = HIWORD(info->dwExtraInfo);
    extended_states[vk] = extra_state;


    std::array<BYTE, 256> flags{};

    if (::GetKeyboardState(flags.data()) && vk)
    {
      flags[vk] |= (info->flags & LLKHF_UP) ? 0 : 1 << 7;

      ::SetKeyboardState(flags.data());
    }

    auto event = WM_KEYDOWN;

    if (info->flags & LLKHF_UP)
    {
      event = WM_KEYUP;
    }

    ::GUITHREADINFO gui_info{ .cbSize = sizeof(::GUITHREADINFO) };

    if (::GetGUIThreadInfo(::GetCurrentThreadId(), &gui_info) && gui_info.hwndFocus)
    {
      ::PostMessageW(gui_info.hwndFocus, event, vk, lParam);

      if (::IsWindowUnicode(gui_info.hwndFocus))
      {
        ::SetPropW(gui_info.hwndFocus, L"ExtendedVkStates", &extended_states);
      }
      else
      {
        ::SetPropA(gui_info.hwndFocus, "ExtendedVkStates", &extended_states);
      }
    }

    if (dinput_keyboard_hook)
    {
      KBDLLHOOKSTRUCT keyboard_event{
        .vkCode = vk,
        .scanCode = info->scanCode,
        .flags = info->flags,
        .time = info->time,
        .dwExtraInfo = info->dwExtraInfo
      };
      dinput_keyboard_hook->proc(0, wParam, (LPARAM)&keyboard_event);
    }
  }

next_hook:
  return ::CallNextHookEx(nullptr, code, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(
  int code,
  WPARAM wParam,
  LPARAM lParam)
{
  if (code == HC_ACTION)
  {
    return -1;
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
              device_id = LOWORD(input.data.keyboard.ExtraInformation);
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
              auto vk = input.data.keyboard.VKey ? input.data.keyboard.VKey : ::MapVirtualKeyW(input.data.keyboard.MakeCode, MAPVK_VSC_TO_VK);

              LPARAM lParam = 0;
              lParam |= (input.data.keyboard.Flags & RI_KEY_BREAK) ? 1 << 31 : 0;// Transition state
              lParam |= (input.data.keyboard.Flags & RI_KEY_BREAK) ? 1 << 30 : 0;// Previous key state
              lParam |= (input.data.keyboard.Flags & RI_KEY_E0) ? 1 << 24 : 0;
              lParam |= input.data.keyboard.MakeCode << 16;// Scan code

              std::array<BYTE, 256> flags{};

              thread_local std::array<WORD, 256> extended_states{};
              WORD extra_state = HIWORD(input.data.keyboard.ExtraInformation);
              extended_states[vk] = extra_state;

              if (message->hwnd)
              {
                if (::IsWindowUnicode(message->hwnd))
                {
                  ::SetPropW(message->hwnd, L"ExtendedVkStates", &extended_states);
                }
                else
                {
                  ::SetPropA(message->hwnd, "ExtendedVkStates", &extended_states);
                }
              }

              if (::GetKeyboardState(flags.data()))
              {
                flags[vk] |= (input.data.keyboard.Flags & RI_KEY_BREAK) ? 0 : 1 << 7;
                ::SetKeyboardState(flags.data());
              }

              auto event = WM_KEYDOWN;

              if (input.data.keyboard.Flags & RI_KEY_BREAK)
              {
                event = WM_KEYUP;
              }

              ::PostMessageW(message->hwnd, event, vk, lParam);

              if (dinput_keyboard_hook && input.data.keyboard.MakeCode)
              {
                KBDLLHOOKSTRUCT keyboard_event{
                  .vkCode = vk,
                  .scanCode = input.data.keyboard.MakeCode,
                  .dwExtraInfo = input.data.keyboard.ExtraInformation
                };

                if (input.data.keyboard.Flags & RI_KEY_BREAK)
                {
                  keyboard_event.flags |= LLKHF_UP;
                }

                if (input.data.keyboard.Flags & RI_KEY_E0)
                {
                  keyboard_event.flags |= LLKHF_EXTENDED;
                }
                dinput_keyboard_hook->proc(0, event, (LPARAM)&keyboard_event);
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


static auto* TrueGetAsyncKeyState = GetAsyncKeyState;
static auto* TrueSetWindowHookExW = SetWindowsHookExW;
static auto* TrueUnhookWindowsHookEx = UnhookWindowsHookEx;

static HHOOK keyboard_hook = nullptr;
static HHOOK ll_keyboard_hook = nullptr;

HHOOK WINAPI WrappedSetWindowsHookExW(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
  if (auto dinput = ::GetModuleHandleW(L"dinput.dll"); dinput && idHook == WH_KEYBOARD_LL)
  {
    if (ll_keyboard_hook)
    {
      dinput_keyboard_hook = direct_input_hook{ .proc = lpfn, .handle = ll_keyboard_hook };
      return ll_keyboard_hook;
    }

    auto result = TrueSetWindowHookExW(idHook, lpfn, hmod, dwThreadId);
    dinput_keyboard_hook = direct_input_hook{ .proc = lpfn, .handle = result };
    return result;
  }

  if (auto dinput8 = ::GetModuleHandleW(L"dinput8.dll"); dinput8 && idHook == WH_KEYBOARD_LL)
  {
    if (ll_keyboard_hook)
    {
      dinput_keyboard_hook = direct_input_hook{ .proc = lpfn, .handle = ll_keyboard_hook };
      return ll_keyboard_hook;
    }

    auto result = TrueSetWindowHookExW(idHook, lpfn, hmod, dwThreadId);
    dinput_keyboard_hook = direct_input_hook{ .proc = lpfn, .handle = result };
    return result;
  }

  return TrueSetWindowHookExW(idHook, lpfn, hmod, dwThreadId);
}

BOOL WINAPI WrappedUnhookWindowsHookEx(HHOOK handle)
{
  if (handle == ll_keyboard_hook)
  {
    ll_keyboard_hook = nullptr;
  }

  if (dinput_keyboard_hook && dinput_keyboard_hook->handle == handle)
  {
    dinput_keyboard_hook = std::nullopt;
  }

  return TrueUnhookWindowsHookEx(handle);
}

SHORT __stdcall WrappedGetAsyncKeyState(int vKey)
{
  return ::GetKeyState(vKey);
}

static std::array<std::pair<void**, void*>, 3> detour_functions{
  {
    { &(void*&)TrueGetAsyncKeyState, WrappedGetAsyncKeyState },
    { &(void*&)TrueSetWindowHookExW, WrappedSetWindowsHookExW },
    { &(void*&)TrueUnhookWindowsHookEx, WrappedUnhookWindowsHookEx },
  }
};


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
      DetourRestoreAfterWith();

      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());

      std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourAttach(func.first, func.second); });

      DetourTransactionCommit();


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
      if (::RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == TRUE)
      {
        cbt_hook = ::SetWindowsHookExW(WH_CBT, CBTProc, hinstDLL, ::GetCurrentThreadId());

        if (cbt_hook == nullptr)
        {
          Rid[0].dwFlags = RIDEV_REMOVE;
          Rid[0].hwndTarget = 0;
          assert(::RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == TRUE);
          return FALSE;
        }

        get_message_hook = ::SetWindowsHookExW(WH_GETMESSAGE, GetMsgProc, hinstDLL, ::GetCurrentThreadId());

        if (get_message_hook == nullptr)
        {
          Rid[0].dwFlags = RIDEV_REMOVE;
          Rid[0].hwndTarget = 0;
          assert(::RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == TRUE);
          assert(::UnhookWindowsHookEx(cbt_hook) == TRUE);
          return FALSE;
        }

        return TRUE;
      }
      else
      {
        ll_keyboard_hook = ::SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hinstDLL, 0);

        keyboard_hook = ::SetWindowsHookExW(WH_KEYBOARD, KeyboardProc, hinstDLL, ::GetCurrentThreadId());

        if (!(ll_keyboard_hook && keyboard_hook))
        {
          if (ll_keyboard_hook)
          {
            ::UnhookWindowsHookEx(ll_keyboard_hook);
          }

          if (keyboard_hook)
          {
            ::UnhookWindowsHookEx(keyboard_hook);
          }
          return FALSE;
        }
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());

      std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourDetach(func.first, func.second); });
      DetourTransactionCommit();

      if (cbt_hook)
      {
        assert(::UnhookWindowsHookEx(cbt_hook) == TRUE);
      }

      if (get_message_hook)
      {
        assert(::UnhookWindowsHookEx(get_message_hook) == TRUE);
      }

      if (ll_keyboard_hook)
      {
        assert(::UnhookWindowsHookEx(ll_keyboard_hook) == TRUE);
      }

      if (keyboard_hook)
      {
        assert(::UnhookWindowsHookEx(keyboard_hook) == TRUE);
      }

      RAWINPUTDEVICE Rid[1];

      Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      Rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
      Rid[0].dwFlags = RIDEV_REMOVE;
      Rid[0].hwndTarget = 0;

      ::RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
    }
  }

  return TRUE;
}
}
