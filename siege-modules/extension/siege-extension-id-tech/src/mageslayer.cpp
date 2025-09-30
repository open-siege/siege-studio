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
#include <siege/extension/shared.hpp>
#include "id-tech-shared.hpp"


extern "C" {
using game_action = siege::platform::game_action;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+backward", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+strafeleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+straferight", u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, "+jump", u"Jump", u"Movement" },
  game_action{ game_action::analog, "+down", u"Crouch", u"Movement" },
  game_action{ game_action::digital, "+speed", u"Run", u"Movement" },
  game_action{ game_action::analog, "+turnleft", u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, "+turnright", u"Turn Right", u"Aiming" },
  game_action{ game_action::analog, "+aimup", u"Aim Up", u"Aiming" },
  game_action{ game_action::analog, "+aimdown", u"Aim Down", u"Aiming" },
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "+use", u"Use", u"Combat" },
  game_action{ game_action::digital, "previtem", u"Previous Item", u"Combat" },
  game_action{ game_action::digital, "nextitem", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "useitem", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "dropitem", u"Drop Item", u"Combat" },
  game_action{ game_action::digital, "nextweapon", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "prevweapon", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "dropweapon", u"Drop Weapon", u"Combat" },
  game_action{ game_action::digital, "looktoggle", u"Look Toggle", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };
using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x4e2bac) },
  { "concmds"sv, std::size_t(0x4e2bbc) },
  { "cl_showactors"sv, std::size_t(0x4eafc4) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 9> function_name_ranges{ {
  { "-mAim"sv, "looktoggle"sv },
  { "-look"sv, "+yawpos"sv },
  { "-6DOF"sv, "+use"sv },
  { "suicide"sv, "impulse"sv },
  { "-lowercamera"sv, "+raisecamera"sv },
  { "-speed"sv, "+strafeleft"sv },
  { "A_LoadPosMarkFile"sv, "A_MoveActor"sv },
  { "vp_rendertorches"sv, "vp_enginecmd"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 4> variable_name_ranges{ { { "a_acceleration"sv, "a_friction"sv },
  { "cl_showactors"sv, "cl_TimeStampVel"sv },
  { "joyStrafe"sv, "in_ForwardSpeed"sv },
  { "vp_enginetest"sv, "vp_mipdist"sv } } };

std::errc get_function_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(function_name_ranges, length, data, saved);
}

std::errc get_variable_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(variable_name_ranges, length, data, saved);
}

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
}

static std::string VirtualDriveLetter;
constexpr static std::string_view MageslayerDisc = "MAGESLAY";
static auto* TrueGetLogicalDrives = GetLogicalDrives;
static auto* TrueGetDriveTypeA = GetDriveTypeA;
static auto* TrueGetVolumeInformationA = GetVolumeInformationA;
static auto* TrueGetLogicalDriveStringsA = GetLogicalDriveStringsA;

DWORD WINAPI WrappedGetLogicalDrives()
{
  auto result = TrueGetLogicalDrives();
  std::bitset<sizeof(DWORD) * 8> bits(result);

  int driveLetter = static_cast<int>('A');

  for (auto i = 2; i < bits.size(); ++i)
  {
    if (bits[i] == false)
    {
      driveLetter += i;
      bits[i] = true;

      if (VirtualDriveLetter.empty())
      {
        VirtualDriveLetter = static_cast<char>(driveLetter) + std::string(":\\");
      }
      break;
    }
  }


  return bits.to_ulong();
}

DWORD WINAPI WrappedGetLogicalDriveStringsA(DWORD nBufferLength, LPSTR lpBuffer)
{
  // the game doesn't call GetLogicalDrives, so we should call it ourselves
  static auto drive_types = WrappedGetLogicalDrives();

  if (!VirtualDriveLetter.empty())
  {
    std::string drives;
    drives.resize(TrueGetLogicalDriveStringsA(0, drives.data()));
    TrueGetLogicalDriveStringsA((DWORD)drives.size(), drives.data());

    drives.push_back('\0');
    drives.append(VirtualDriveLetter);

    DWORD size = drives.size() < nBufferLength ? (DWORD)drives.size() : nBufferLength;
    std::memcpy(lpBuffer, drives.data(), size);
    return size;
  }

  return TrueGetLogicalDriveStringsA(nBufferLength, lpBuffer);
}

UINT WINAPI WrappedGetDriveTypeA(LPCSTR lpRootPathName)
{
  if (lpRootPathName && !VirtualDriveLetter.empty() && VirtualDriveLetter[0] == lpRootPathName[0])
  {
    return DRIVE_CDROM;
  }

  return TrueGetDriveTypeA(lpRootPathName);
}

BOOL WINAPI WrappedGetVolumeInformationA(
  LPCSTR lpRootPathName,
  LPSTR lpVolumeNameBuffer,
  DWORD nVolumeNameSize,
  LPDWORD lpVolumeSerialNumber,
  LPDWORD lpMaximumComponentLength,
  LPDWORD lpFileSystemFlags,
  LPSTR lpFileSystemNameBuffer,
  DWORD nFileSystemNameSize)
{
  if (lpRootPathName && lpRootPathName[0] == VirtualDriveLetter[0])
  {
    std::vector<char> data(nVolumeNameSize, '\0');
    std::copy(MageslayerDisc.begin(), MageslayerDisc.end(), data.begin());
    std::copy(data.begin(), data.end(), lpVolumeNameBuffer);
    return TRUE;
  }

  return TrueGetVolumeInformationA(lpRootPathName,
    lpVolumeNameBuffer,
    nVolumeNameSize,
    lpVolumeSerialNumber,
    lpMaximumComponentLength,
    lpFileSystemFlags,
    lpFileSystemNameBuffer,
    nFileSystemNameSize);
}

static std::array<std::pair<void**, void*>, 4> detour_functions{ {
  { &(void*&)TrueGetLogicalDrives, WrappedGetLogicalDrives },
  { &(void*&)TrueGetLogicalDriveStringsA, WrappedGetLogicalDriveStringsA },
  { &(void*&)TrueGetDriveTypeA, WrappedGetDriveTypeA },
  { &(void*&)TrueGetVolumeInformationA, WrappedGetVolumeInformationA },
} };

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
      try
      {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourAttach(func.first, func.second); });

        DetourTransactionCommit();
      }
      catch (...)
      {
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
    }
  }

  return TRUE;
}
}
