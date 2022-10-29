#include <atomic>
#include <array>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CINTERFACE
#include <windows.h>
#include <detours.h>

#include "System.hpp"

namespace core
{

  static auto* TrueCreateProcessA = CreateProcessA;

  static std::atomic_bool should_detour = true;

  BOOL WINAPI DarkCreateProcessA(
    LPCSTR                lpApplicationName,
    LPSTR                 lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCSTR                lpCurrentDirectory,
    LPSTARTUPINFOA        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
  )
  {
    if (!should_detour)
    {
      return TrueCreateProcessA(lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation);
    }
    should_detour = false;
    auto result = DetourCreateProcessWithDllExA(lpApplicationName,
      lpCommandLine,
      lpProcessAttributes,
      lpThreadAttributes,
      bInheritHandles,
      dwCreationFlags,
      lpEnvironment,
      lpCurrentDirectory,
      lpStartupInfo,
      lpProcessInformation,
      "darkstar.dll",
      nullptr);

    should_detour = true;

    return result;
  }

  std::array<std::pair<void**, void*>, 1> GetSystemDetours()
  {
    return std::array<std::pair<void**, void*>, 1>{ {
      { &(void*&)TrueCreateProcessA, DarkCreateProcessA } }
    };
  }
}

