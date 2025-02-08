#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <detours.h>
#include "shared.hpp"
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"

extern "C" {
extern void(__cdecl* ConsoleEvalCdecl)(const char*) = nullptr;
extern void(__fastcall* ConsoleEvalFastcall)(const char*) = nullptr;
extern void(__stdcall* ConsoleEvalStdcall)(const char*) = nullptr;

LRESULT CALLBACK dispatch_input_to_game_console(int code, WPARAM wParam, LPARAM lParam, void (*process_movement_keydown)(MSG* message, void (*console_eval)(const char*)), void (*console_eval)(const char*));

void process_quake_movement_keydown(MSG* message, void (*console_eval)(const char*));
void process_quake_3_movement_keydown(MSG* message, void (*console_eval)(const char*));

LRESULT CALLBACK dispatch_input_to_cdecl_quake_2_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_movement_keydown, ConsoleEvalCdecl);
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

LRESULT CALLBACK dispatch_input_to_cdecl_quake_1_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_movement_keydown, do_console_eval_cdecl_quake_1);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

LRESULT CALLBACK dispatch_input_to_cdecl_quake_3_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_3_movement_keydown, do_console_eval_cdecl_quake_1);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

void do_console_eval_stdcall_quake_3(const char* text)
{
  thread_local std::string temp;
  temp.assign(text);
  temp.append(1, '\n');
  ConsoleEvalCdecl(temp.c_str());
}

LRESULT CALLBACK dispatch_input_to_stdcall_quake_3_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_3_movement_keydown, do_console_eval_stdcall_quake_3);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

void do_console_eval_fastcall(const char* text)
{
  ConsoleEvalFastcall(text);
}

LRESULT CALLBACK dispatch_input_to_fastcall_quake_2_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalFastcall)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_movement_keydown, do_console_eval_fastcall);
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

void process_quake_3_movement_keydown(MSG* message, void (*console_eval)(const char*))
{
  constexpr static std::uint16_t uint_max = -1;
  constexpr static std::uint16_t uint_half = uint_max / 2;
  if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_LEFT)
  {
    auto state = get_extended_state(message->wParam);

    std::stringstream command;
    command << "cl_run " << (state > uint_half ? "1" : "0");
    console_eval(command.str().c_str());
    console_eval("+moveleft");
  }
  else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT)
  {
    auto state = get_extended_state(message->wParam);
    std::stringstream command;
    command << "cl_run " << (state > uint_half ? "1" : "0");
    console_eval(command.str().c_str());
    console_eval("+moveright");
  }
  else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_UP)
  {
    auto state = get_extended_state(message->wParam);
    std::stringstream command;
    command << "cl_run " << (state > uint_half ? "1" : "0");
    console_eval(command.str().c_str());
    console_eval("+forward");
  }
  else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_DOWN)
  {
    auto state = get_extended_state(message->wParam);
    std::stringstream command;
    command << "cl_run " << (state > uint_half ? "1" : "0");
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

void process_quake_movement_keydown(MSG* message, void (*console_eval)(const char*))
{
  constexpr static std::uint16_t uint_max = -1;
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

LRESULT CALLBACK dispatch_input_to_game_console(int code, WPARAM wParam, LPARAM lParam, void (*process_movement_keydown)(MSG* message, void (*console_eval)(const char*)), void (*console_eval)(const char*))
{
  if (code == HC_ACTION && wParam == PM_REMOVE)
  {
    auto* message = (MSG*)lParam;

    if (message->message == WM_KEYDOWN)
    {
      process_movement_keydown(message, console_eval);
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
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}
}