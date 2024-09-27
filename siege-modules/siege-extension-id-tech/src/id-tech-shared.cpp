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

LRESULT CALLBACK dispatch_copy_data_to_game_console(int code, WPARAM wParam, LPARAM lParam, void (*console_eval)(const char*));

LRESULT CALLBACK dispatch_copy_data_to_cdecl_game_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_copy_data_to_game_console(code, wParam, lParam, ConsoleEvalCdecl);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

void do_console_eval_cdecl_quake_1(const char* text)
{
  thread_local std::string temp;
  temp.assign(text);
  temp.append(1, '\n');
  ConsoleEvalCdecl(temp.c_str());
}

LRESULT CALLBACK dispatch_copy_data_to_cdecl_quake_1_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_copy_data_to_game_console(code, wParam, lParam, do_console_eval_cdecl_quake_1);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

void do_console_eval_fastcall(const char* text)
{
  ConsoleEvalFastcall(text);
}

LRESULT CALLBACK dispatch_copy_data_to_fastcall_game_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalFastcall)
  {
    return dispatch_copy_data_to_game_console(code, wParam, lParam, do_console_eval_fastcall);
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

LRESULT CALLBACK dispatch_copy_data_to_game_console(int code, WPARAM wParam, LPARAM lParam, void (*console_eval)(const char*))
{
  if (code == HC_ACTION && wParam == PM_REMOVE)
  {
    auto* message = (MSG*)lParam;

    constexpr static std::uint16_t uint_max = -1;

    if (message->message == WM_KEYDOWN)
    {
      if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_LEFT)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;

        std::stringstream command;
        command << "cl_sidespeed " << (int)state;
        console_eval(command.str().c_str());
        console_eval("+moveleft");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;
        std::stringstream command;
        command << "cl_sidespeed " << (int)state;
        console_eval(command.str().c_str());
        console_eval("+moveright");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_UP)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;
        std::stringstream command;
        command << "cl_forwardspeed " << (int)state;
        console_eval(command.str().c_str());
        console_eval("+forward");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_DOWN)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;
        std::stringstream command;
        command << "cl_forwardspeed " << (int)state;
        console_eval(command.str().c_str());
        console_eval("+back");
      }

      if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
        std::stringstream command;
        command << "cl_yawspeed " << (int)state;
        console_eval(command.str().c_str());
        console_eval("+left");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
        std::stringstream command;
        command << "cl_yawspeed " << (int)state;
        console_eval(command.str().c_str());
        console_eval("+right");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_UP)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
        std::stringstream command;
        command << "cl_pitchspeed " << (int)state;
        console_eval(command.str().c_str());
        console_eval("+lookup");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN)
      {
        auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
        std::stringstream command;
        command << "cl_pitchspeed " << (int)state;
        console_eval(command.str().c_str());
        console_eval("+lookdown");
      }
    }
    else if (message->message == WM_KEYUP)
    {
      if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_LEFT)
      {
        console_eval("-moveleft");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT)
      {
        console_eval("-moveright");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_UP)
      {
        console_eval("-forward");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_DOWN)
      {
        console_eval("-back");
      }

      if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
      {
        console_eval("-left");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT)
      {
        console_eval("-right");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_UP)
      {
        console_eval("-lookup");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN)
      {
        console_eval("-lookdown");
      }
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
  }
}
}