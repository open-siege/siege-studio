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
#include "shared.hpp"
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"

extern "C" {
extern void(__cdecl* ConsoleEvalCdecl)(const char*) = nullptr;
extern void(__fastcall* ConsoleEvalFastcall)(const char*) = nullptr;

HRESULT update_action_intensity_for_process_using_console_command(DWORD process_id, DWORD thread_id, const char* action, float intensity)
{
  if (!action)
  {
    return E_POINTER;
  }

  GUITHREADINFO info{ .cbSize = sizeof(GUITHREADINFO) };

  if (!::GetGUIThreadInfo(thread_id, &info))
  {
    return E_INVALIDARG;
  }

  if (!info.hwndActive)
  {
    return E_INVALIDARG;
  }

  std::string_view temp_action(action);

  thread_local std::string buffer;
  buffer.clear();
  buffer.reserve(temp_action.size() + 1);
  buffer.append(intensity == INFINITY ? "+" : "-");
  buffer.append(temp_action);

  COPYDATASTRUCT data{
    .dwData = ::RegisterWindowMessageW(L"ConsoleCommand"),
    .cbData = buffer.size(),
    .lpData = buffer.data()
  };

  ::SendMessageW(info.hwndActive, WM_COPYDATA, (WPARAM)::GetActiveWindow(), (LPARAM)&data);

  return S_OK;
}

LRESULT CALLBACK dispatch_copy_data_to_fastcall_game_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (code == HC_ACTION && lParam && ConsoleEvalCdecl)
  {
    auto* message = (CWPSTRUCT*)lParam;

    if (message->message == WM_COPYDATA)
    {
      auto* data = (COPYDATASTRUCT*)message->lParam;
      if (data && data->dwData == ::RegisterWindowMessageW(L"ConsoleCommand"))
      {
        ConsoleEvalFastcall((char*)data->lpData);
      }
    }
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

WORD get_extended_state(BYTE vkey)
{
  std::array<WORD, 256>* extended_states = nullptr;
  auto window = ::GetFocus();
  if (window && ::IsWindowUnicode(window))
  {
    extended_states = (std::array<WORD, 256>*)::GetPropW(window, L"ExtendedVkStates");
  }
  else if (window)
  {
    extended_states = (std::array<WORD, 256>*)::GetPropA(window, "ExtendedVkStates");
  }

  if (extended_states)
  {
    return (*extended_states)[vkey];
  }

  return 0;
}

LRESULT CALLBACK dispatch_copy_data_to_cdecl_game_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (code == HC_ACTION && wParam == PM_REMOVE && ConsoleEvalCdecl)
  {
    auto* message = (MSG*)lParam;

    WORD vkCode = LOWORD(message->wParam);// virtual-key code

    WORD keyFlags = HIWORD(message->lParam);

    WORD scanCode = LOBYTE(keyFlags);// scan code
    BOOL isExtendedKey = (keyFlags & KF_EXTENDED) == KF_EXTENDED;// extended-key flag, 1 if scancode has 0xE0 prefix

    if (isExtendedKey)
      scanCode = MAKEWORD(scanCode, 0xE0);

    BOOL wasKeyDown = (keyFlags & KF_REPEAT) == KF_REPEAT;// previous key-state flag, 1 on autorepeat
    WORD repeatCount = LOWORD(message->lParam);// repeat count, > 0 if several keydown messages was combined into one message

    BOOL isKeyReleased = (keyFlags & KF_UP) == KF_UP;
    
    constexpr static std::uint16_t uint_max = -1;

    if (message->message == WM_KEYDOWN)
    {
      if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_LEFT)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;

        std::stringstream command;
        command << "cl_sidespeed " << (int)state;
        ConsoleEvalCdecl(command.str().c_str());
        ConsoleEvalCdecl("+moveleft");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;
        std::stringstream command;
        command << "cl_sidespeed " << (int)state;
        ConsoleEvalCdecl(command.str().c_str());
        ConsoleEvalCdecl("+moveright");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_UP)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;
        std::stringstream command;
        command << "cl_forwardspeed " << (int)state;
        ConsoleEvalCdecl(command.str().c_str());
        ConsoleEvalCdecl("+forward");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_DOWN)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;
        std::stringstream command;
        command << "cl_forwardspeed " << (int)state;
        ConsoleEvalCdecl(command.str().c_str());
        ConsoleEvalCdecl("+back");
      }

      if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
        std::stringstream command;
        command << "cl_pitchspeed " << (int)state;
        ConsoleEvalCdecl(command.str().c_str());
        ConsoleEvalCdecl("+left");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
        std::stringstream command;
        command << "cl_pitchspeed " << (int)state;
        ConsoleEvalCdecl(command.str().c_str());
        ConsoleEvalCdecl("+right");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_UP)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
        std::stringstream command;
        command << "cl_yawspeed " << (int)state;
        ConsoleEvalCdecl(command.str().c_str());
        ConsoleEvalCdecl("+lookup");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
        std::stringstream command;
        command << "cl_yawspeed " << (int)state;
        ConsoleEvalCdecl(command.str().c_str());
        ConsoleEvalCdecl("+lookdown");
      }
    }
    else if (message->message == WM_KEYUP)
    {
      if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_LEFT)
      {
        ConsoleEvalCdecl("-moveleft");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT)
      {
        ConsoleEvalCdecl("-moveright");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_UP)
      {
        ConsoleEvalCdecl("-forward");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_DOWN)
      {
        ConsoleEvalCdecl("-back");
      }

      if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
      {
        ConsoleEvalCdecl("-left");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT)
      {
        ConsoleEvalCdecl("-right");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_UP)
      {
        ConsoleEvalCdecl("-lookup");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN)
      {
        ConsoleEvalCdecl("-lookdown");
      }
    }

  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}
}