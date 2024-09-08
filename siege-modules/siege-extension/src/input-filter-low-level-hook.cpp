#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/window_impl.hpp>
#include <detours.h>
#include "input-filter.hpp"

extern "C" {

LRESULT CALLBACK LowLevelKeyboardProc(
  int code,
  WPARAM wParam,
  LPARAM lParam)
{
  if (code == HC_ACTION)
  {
    auto* context = (KBDLLHOOKSTRUCT*)lParam;

    bool is_valid = true;// blocking everything by default would break raw input.
    // we still need real hardware events that can be filtered out later.

    if (context->flags & LLKHF_INJECTED)
    {
      is_valid = false;
      auto& state = siege::get_active_input_state();

      auto device_id = context->dwExtraInfo;

      auto device_iter = std::find_if(state.devices.begin(), state.devices.end(), [&](auto& device) {
        return (device.type == RIM_TYPEKEYBOARD || device.type == RIM_TYPEHID) && device.id == device_id && device.enabled == 1;
      });

      if (device_iter == state.devices.end())
      {
        goto rejected;
      }

      auto group_iter = std::find_if(state.groups.begin(), state.groups.end(), [&](auto& group) {
        return group.id == device_iter->group_id && group.enabled == 1;
      });

      if (group_iter == state.groups.end())
      {
        goto rejected;
      }

      GUITHREADINFO gui_info{ .cbSize = sizeof(GUITHREADINFO) };

      if (group_iter->thread_id == ::GetCurrentThreadId() && ::GetGUIThreadInfo(group_iter->thread_id, &gui_info) && gui_info.hwndActive)
      {
        auto vk = context->vkCode ? context->vkCode : ::MapVirtualKeyW(context->scanCode, MAPVK_VSC_TO_VK);

        /* if (context->scanCode == 0x002A || context->scanCode == 0x0036)
         {
           std::array<BYTE, 256> states;

           if (::GetKeyboardState(states.data()))
           {
             if (context->flags & LLKHF_UP)
             {
               states[vk] = 0;
             }
             else
             {
               states[vk] |= 0x80;
             }

             ::SetKeyboardState(states.data());
           }
         }*/

        LPARAM lParam = 0;
        lParam |= (context->flags & LLKHF_UP) ? 1 << 31 : 0;// Transition state
        lParam |= (context->flags & LLKHF_UP) ? 1 << 30 : 0;// Previous key state
        lParam |= (context->flags & LLKHF_EXTENDED ? 1 << 24 : 0);
        lParam |= context->scanCode << 16;// Scan code


        auto message = WM_KEYDOWN;

        if (context->flags & LLKHF_UP)
        {
          message = WM_KEYUP;
        }

        std::optional<WPARAM> extra_key = std::nullopt;

        if (vk == VK_LSHIFT || vk == VK_RSHIFT)
        {
          extra_key = VK_SHIFT;
        }

        if (vk == VK_LCONTROL || vk == VK_RCONTROL)
        {
          extra_key = VK_CONTROL;
        }

        if (vk == VK_LMENU || vk == VK_RMENU)
        {
          extra_key = VK_MENU;
        }

        ::SendMessageTimeoutW(gui_info.hwndActive, message, vk, lParam, SMTO_ABORTIFHUNG, 1, nullptr);

        if (extra_key)
        {
          ::SendMessageTimeoutW(gui_info.hwndActive, message, *extra_key, lParam, SMTO_ABORTIFHUNG, 1, nullptr);
        }

      }

      is_valid = true;
    }

  rejected:
    if (!is_valid)
    {
      return -1;
    }
  }

  return ::CallNextHookEx(nullptr, code, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(
  int code,
  WPARAM wParam,
  LPARAM lParam)
{
  if (code == HC_ACTION || code == HC_NOREMOVE)
  {
    return -1;
  }

  return ::CallNextHookEx(nullptr, code, wParam, lParam);
}


LRESULT CALLBACK LowLevelMouseProc(
  int code,
  WPARAM wParam,
  LPARAM lParam)
{
  if (code == HC_ACTION)
  {
    auto* context = (MSLLHOOKSTRUCT*)lParam;
  }

  return ::CallNextHookEx(nullptr, code, wParam, lParam);
}


BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
  if constexpr (sizeof(void*) != sizeof(std::uint32_t))
  {
    return TRUE;
  }

  if (DetourIsHelperProcess())
  {
    return TRUE;
  }

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    static HHOOK ll_keyboard_hook = nullptr;
    static HHOOK keyboard_hook = nullptr;
    static HHOOK mouse_hook = nullptr;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      ll_keyboard_hook = ::SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hinstDLL, 0);

      if (ll_keyboard_hook == nullptr)
      {
        return FALSE;
      }

      keyboard_hook = ::SetWindowsHookExW(WH_KEYBOARD, KeyboardProc, hinstDLL, ::GetCurrentThreadId());

      if (keyboard_hook == nullptr)
      {
        assert(::UnhookWindowsHookEx(ll_keyboard_hook) == TRUE);
        return FALSE;
      }

      mouse_hook = ::SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, hinstDLL, 0);

      if (mouse_hook == nullptr)
      {
        assert(::UnhookWindowsHookEx(ll_keyboard_hook) == TRUE);
        assert(::UnhookWindowsHookEx(keyboard_hook) == TRUE);
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      assert(::UnhookWindowsHookEx(ll_keyboard_hook) == TRUE);
      assert(::UnhookWindowsHookEx(keyboard_hook) == TRUE);
      assert(::UnhookWindowsHookEx(mouse_hook) == TRUE);
    }
  }

  return TRUE;
}
}
