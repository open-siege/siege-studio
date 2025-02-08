#ifndef ID_TECH_SHARED_HPP
#define ID_TECH_SHARED_HPP

#include <siege/platform/win/window_module.hpp>

extern "C" {
LRESULT CALLBACK dispatch_input_to_cdecl_quake_2_console(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dispatch_input_to_fastcall_quake_2_console(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dispatch_input_to_cdecl_quake_1_console(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dispatch_input_to_cdecl_quake_3_console(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dispatch_input_to_stdcall_quake_3_console(int code, WPARAM wParam, LPARAM lParam);
}

namespace siege::extension
{
  static LRESULT CALLBACK DispatchInputToGameConsole(int code, WPARAM wParam, LPARAM lParam)
  {
    if (code == HC_ACTION && wParam == PM_REMOVE)
    {
      auto* message = (MSG*)lParam;

      if (message->message == WM_INPUT)
      {
        // TODO do dispatching on WM_INPUT here
      }
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  static LRESULT CALLBACK DispatchInputToSendInput(int code, WPARAM wParam, LPARAM lParam)
  {
    if (code == HC_ACTION && wParam == PM_REMOVE)
    {
      auto* message = (MSG*)lParam;

      if (message->message == WM_INPUT)
      {
        // TODO do dispatching on WM_INPUT here
      }
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
  }
}// namespace siege::extension

#endif