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

LRESULT CALLBACK dispatch_copy_data_to_cdecl_game_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (code == HC_ACTION && lParam && ConsoleEvalCdecl)
  {
    auto* message = (CWPSTRUCT*)lParam;

    if (message->message == WM_COPYDATA)
    {
      auto* data = (COPYDATASTRUCT*)message->lParam;
      if (data && data->dwData == ::RegisterWindowMessageW(L"ConsoleCommand"))
      {
        ConsoleEvalCdecl((char*)data->lpData);
      }
    }
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}
}