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

extern "C" {
using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "alias"sv, std::size_t(0x61cc48) },
  { "quit"sv, std::size_t(0x61cbec) },
  { "KQGame::PlayFromCD"sv, std::size_t(0x5b0170) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 15> function_name_ranges{ {
  { "KQMusic::delete"sv, "KQMusic::play"sv },
  { "killGame"sv, "deleteObject"sv },
  { "KQObject::setGroupsActive3DFar"sv, "KQObject::setLoc"sv },
  { "KQCamera::setHeight"sv, "KQCamera::follow"sv },
  { "KQGame::letCatchUp"sv, "profile"sv },
  { "KQConner::removeFromTrap"sv, "KQMonster::god"sv },
  { "KQEmitter::reinit"sv, "KQEmitter::setFollowObject"sv },
  { "KQMap::stopLoc"sv, "KQMap::activate"sv },
  { "Heap::findLeaks"sv, "Heap::check"sv },
  { "messageCanvasDevice"sv, "swapSurfaces"sv },
  { "newPointLight"sv, "newDirectionalLight"sv },
  { "globeLines"sv, "newSky"sv },
  { "loadInterior"sv, "loadInterior"sv },
  { "quit"sv, "alias"sv },
  { "KQSound::setMixLimit"sv, "KQSound::setSpeechVol"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 3> variable_name_ranges{ { { "NewWorld"sv, "KQPortal::ConLoc"sv },
  { "allowAltEnter"sv, "ConsoleWorld::FrameRate"sv },
  { "SetFirstPerson::WasThirdPerson"sv, "ConInv::Boots"sv } } };


static std::int32_t(__fastcall* CopyDir)(const char*, const char*, BOOL) = nullptr;

inline void set_gog_exports()
{

  CopyDir = (decltype(CopyDir))0x499520;
}

constexpr std::array<void (*)(), 1> export_functions = { {
  set_gog_exports,
} };

HRESULT get_function_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(function_name_ranges, length, data, saved);
}

HRESULT get_variable_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(variable_name_ranges, length, data, saved);
}

HRESULT executable_is_supported(_In_ const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
}

static auto* TrueOutputDebugStringA = OutputDebugStringA;

void __stdcall WrappedTrueOutputDebugStringA(const char* message)
{
  return TrueOutputDebugStringA(message);
}


LRESULT CALLBACK hook_proc(int code, WPARAM wParam, LPARAM lParam)
{
  if (code == HC_ACTION)
  {

    CopyDir(".\\data\\Barren\\8bit.vol", ".\\Barren\\8bit\\", TRUE);
    CopyDir(".\\data\\Barren\\8gui.vol", ".\\Barren\\8gui\\", TRUE);
    CopyDir(".\\data\\Barren\\kq.vol", ".\\Barren\\kq\\", TRUE);
    CopyDir(".\\data\\Barren\\resource.vol", ".\\Barren\\resource\\", TRUE);

    CopyDir(".\\data\\daventry\\8bit.vol", ".\\daventry\\8bit\\", TRUE);
    CopyDir(".\\data\\daventry\\8gui.vol", ".\\daventry\\8gui\\", TRUE);
    CopyDir(".\\data\\daventry\\kq.vol", ".\\daventry\\kq\\", TRUE);
    CopyDir(".\\data\\daventry\\resource.vol", ".\\daventry\\resource\\", TRUE);

    CopyDir(".\\data\\deadcity\\8bit.vol", ".\\deadcity\\8bit\\", TRUE);
    CopyDir(".\\data\\deadcity\\8gui.vol", ".\\deadcity\\8gui\\", TRUE);
    CopyDir(".\\data\\deadcity\\kq.vol", ".\\deadcity\\kq\\", TRUE);
    CopyDir(".\\data\\deadcity\\resource.vol", ".\\deadcity\\resource\\", TRUE);

    CopyDir(".\\data\\iceworld\\8bit.vol", ".\\iceworld\\8bit\\", TRUE);
    CopyDir(".\\data\\iceworld\\8gui.vol", ".\\iceworld\\8gui\\", TRUE);
    CopyDir(".\\data\\iceworld\\kq.vol", ".\\iceworld\\kq\\", TRUE);
    CopyDir(".\\data\\iceworld\\resource.vol", ".\\iceworld\\resource\\", TRUE);

    CopyDir(".\\data\\temple1\\8bit.vol", ".\\temple1\\8bit\\", TRUE);
    CopyDir(".\\data\\temple1\\8gui.vol", ".\\temple1\\8gui\\", TRUE);
    CopyDir(".\\data\\temple1\\kq.vol", ".\\temple1\\kq\\", TRUE);
    CopyDir(".\\data\\temple1\\resource.vol", ".\\temple1\\resource\\", TRUE);

    CopyDir(".\\data\\temple2\\8bit.vol", ".\\temple2\\8bit\\", TRUE);
    CopyDir(".\\data\\temple2\\8gui.vol", ".\\temple2\\8gui\\", TRUE);
    CopyDir(".\\data\\temple2\\kq.vol", ".\\temple2\\kq\\", TRUE);
    CopyDir(".\\data\\temple2\\resource.vol", ".\\temple2\\resource\\", TRUE);

    CopyDir(".\\data\\temple3\\8bit.vol", ".\\temple3\\8bit\\", TRUE);
    CopyDir(".\\data\\temple3\\8gui.vol", ".\\temple3\\8gui\\", TRUE);
    CopyDir(".\\data\\temple3\\kq.vol", ".\\temple3\\kq\\", TRUE);
    CopyDir(".\\data\\temple3\\resource.vol", ".\\temple3\\resource\\", TRUE);
    ExitProcess(0);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}


static std::array<std::pair<void**, void*>, 1> detour_functions{ { { &(void*&)TrueOutputDebugStringA, WrappedTrueOutputDebugStringA } } };

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
    auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

    auto value = app_module.GetProcAddress<std::uint32_t*>("DisableSiegeExtensionModule");

    if (value && *value == -1)
    {
      return TRUE;
    }
  }

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      int index = 0;
      try
      {
        auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

        bool module_is_valid = false;

        for (const auto& item : verification_strings)
        {
          win32::module_ref temp((void*)item[0].second);

          if (temp != app_module)
          {
            continue;
          }

          module_is_valid = std::all_of(item.begin(), item.end(), [](const auto& str) {
            return std::memcmp(str.first.data(), (void*)str.second, str.first.size()) == 0;
          });


          if (module_is_valid)
          {
            export_functions[index]();
            break;
          }
          index++;
        }

        if (!module_is_valid)
        {
          return FALSE;
        }

        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourAttach(func.first, func.second); });

        DetourTransactionCommit();


        SetWindowsHookExA(WH_GETMESSAGE, hook_proc, hinstDLL, ::GetCurrentThreadId());
      }
      catch (...)
      {
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());

      std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourDetach(func.first, func.second); });
      DetourTransactionCommit();
    }
  }

  return TRUE;
}
}
