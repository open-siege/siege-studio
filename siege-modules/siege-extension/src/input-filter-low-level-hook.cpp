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

extern "C" {

LRESULT CALLBACK LowLevelKeyboardProc(
  int code,
  WPARAM wParam,
  LPARAM lParam)
{
  if (code == HC_ACTION)
  {
    auto* context = (KBDLLHOOKSTRUCT*)lParam;

    bool is_correct = context->flags & LLKHF_INJECTED && context->dwExtraInfo == 266;

    if (!is_correct)
    {
      return -1;
    }
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
    static HHOOK keyboard_hook = nullptr;
    static HHOOK mouse_hook = nullptr;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      keyboard_hook = ::SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hinstDLL, 0);

      if (keyboard_hook == nullptr)
      {
        return FALSE;
      }

      mouse_hook = ::SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, hinstDLL, 0);

      if (mouse_hook == nullptr)
      {
        assert(::UnhookWindowsHookEx(keyboard_hook) == TRUE);
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      assert(::UnhookWindowsHookEx(keyboard_hook) == TRUE);
      assert(::UnhookWindowsHookEx(mouse_hook) == TRUE);
    }
  }

  return TRUE;
}
}
