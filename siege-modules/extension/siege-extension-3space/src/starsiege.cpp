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

using game_command_line_caps = siege::platform::game_command_line_caps;
using namespace std::literals;

extern "C" {
// TODO there are other ways of controlling the game. Do we still need to hook directly into the game?
#define DARKCALL __attribute__((regparm(3)))
static DARKCALL char* (*ConsoleEval)(void*, std::int32_t, std::int32_t, const char**) = nullptr;


extern auto command_line_caps = game_command_line_caps{
  .string_settings = { { L"connect" } },
  .ip_connect_setting = L"connect",
};

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 4> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 4>{ {
                                                                                                              { "cls"sv, std::size_t(0x6fb741) },
                                                                                                              { "trace"sv, std::size_t(0x6fb7af) },
                                                                                                              { "Console::logBufferEnabled"sv, std::size_t(0x6fb7b5) },
                                                                                                              { "Console::logMode"sv, std::size_t(0x6fb7fa) },
                                                                                                            } },
  std::array<std::pair<std::string_view, std::size_t>, 4>{ {
    { "cls"sv, std::size_t(0x6fe551) },
    { "trace"sv, std::size_t(0x6fe5bf) },
    { "Console::logBufferEnabled"sv, std::size_t(0x6fe5c5) },
    { "Console::logMode"sv, std::size_t(0x6fe60a) },
  } },
  std::array<std::pair<std::string_view, std::size_t>, 4>{ {
    { "cls"sv, std::size_t(0x712cf9) },
    { "trace"sv, std::size_t(0x712d67) },
    { "Console::logBufferEnabled"sv, std::size_t(0x712d6d) },
    { "Console::logMode"sv, std::size_t(0x712db2) },
  } },
  std::array<std::pair<std::string_view, std::size_t>, 4>{ {
    { "cls"sv, std::size_t(0x723169) },
    { "trace"sv, std::size_t(0x7231d7) },
    { "Console::logBufferEnabled"sv, std::size_t(0x7231dd) },
    { "Console::logMode"sv, std::size_t(0x723222) },
  } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 22> function_name_ranges{ { { "icDefaultButtonAction"sv, "icActionAllowed"sv },
  { "dataStore"sv, "dataRelease"sv },
  { "defaultWeapons"sv, "allowWeapon"sv },
  { "goto"sv, "violate"sv },
  { "initializeServer"sv, "checkDiskFreeSpace"sv },// //isEqualIP added in 1.0.0.3
  { "say"sv, "flushChannel"sv },
  { "dynDataWriteObject"sv, "FlushPilots"sv },
  //  { "ME::Create"sv, "ME::RebuildCommandMap"sv }, // added in 1.0.0.3
  { "loadObject"sv, "getNextObject"sv },
  { "HTMLOpen"sv, "HTMLOpenAndGoWin"sv },
  { "swapSurfaces"sv, "isGfxDriver"sv },
  { "newMovPlay"sv, "pauseMovie"sv },
  { "netStats"sv, "net::kick"sv },
  { "newRedbook"sv, "rbSetPlayMode"sv },
  { "newInputManager"sv, "unbind"sv },// defineKey added in 1.0.0.3
  { "simTreeCreate"sv, "simTreeRegScript"sv },
  { "newSfx"sv, "sfxGetMaxBuffers"sv },
  { "newToolWindow"sv, "saveFileAs"sv },
  { "newTerrain"sv, "lightTerrain"sv },// reCalcCRC added in 1.0.0.3
  { "GuiEditMode"sv, "windowsKeyboardDisable"sv },
  //"LSCreate" "LSEditor" available in previous versions
  // { "LS::Create"sv, "LS::parseCommands"sv }, added in 1.0.0.3
  { "ircConnect"sv, "ircEcho"sv },
  //  { "globeLines"sv, "loadSky"sv }, added in 1.0.0.3
  { "MissionRegType"sv, "missionUndoMoveRotate"sv },
  { "cls"sv, "trace"sv } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 13> variable_name_ranges{ { { "$pref::softwareTranslucency"sv, "$pref::canvasCursorTrapped"sv },
  //{ "suspended"sv, "SimGui::firstPreRender"sv }, // added in 1.0.0.3
  { "suspended"sv, "ESBasePlugin::postProduction"sv },
  { "Console::ForeRGB"sv, "Console::LastLineTimeout"sv },
  { "$pref::mipcap"sv, "$OpenGL::HSL"sv },//$OpenGL::AFK added in 1.0.0.2
  { "GFXMetrics::EmittedPolys"sv, "useLowRes3D"sv },
  { "pref::sfx2DVolume"sv, "pref::sfx3DVolume"sv },
  { "dynDataWriteObject"sv, "FlushPilots"sv },
  { "GuiEdit::GridSnapX"sv, "pref::politeGui"sv },
  { "Console::logBufferEnabled"sv, "Console::logMode"sv },
  { "DNet::ShowStats"sv, "DNet::PacketLoss"sv },
  { "GameInfo::SpawnLimit"sv, "GameInfo::TimeLimit"sv },
  { "ITRMetrics::OutsideBits"sv, "ITRMetrics::NumInteriorLinks"sv },
  { "$server::Mission"sv, "$server::TeamMassLimit"sv } } };


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

const wchar_t** format_command_line(const siege::platform::game_command_line_args* args, std::uint32_t* new_size)
{
  if (!args)
  {
    return nullptr;
  }

  if (!new_size)
  {
    return nullptr;
  }

  static std::vector<std::wstring> string_args;
  string_args.clear();

  for (auto& setting : args->string_settings)
  {
    if (!setting.name)
    {
      continue;
    }

    if (!setting.value)
    {
      continue;
    }

    if (!setting.value[0])
    {
      continue;
    }

    auto& back = string_args.emplace_back(setting.name);
    back.insert(0, 1, L'+');
    string_args.emplace_back(setting.value);
  }

  static std::vector<const wchar_t*> raw_args;
  raw_args.resize(string_args.size());
  *new_size = (std::uint32_t)string_args.size();

  std::transform(string_args.begin(), string_args.end(), raw_args.begin(), [](const std::wstring& value) {
    return value.c_str();
  });

  return raw_args.data();
}

static auto* TrueSetWindowsHookExA = SetWindowsHookExA;
static auto* TrueAllocConsole = AllocConsole;
static auto* TrueGetLogicalDrives = GetLogicalDrives;
static auto* TrueGetDriveTypeA = GetDriveTypeA;
static auto* TrueGetVolumeInformationA = GetVolumeInformationA;

static std::string VirtualDriveLetter;
constexpr static std::string_view StarsiegeDisc1 = "STARSIEGE1";
constexpr static std::string_view StarsiegeDisc2 = "STARSIEGE2";
constexpr static std::string_view StarsiegeBetaDisc = "STARSIEGE";

BOOL WINAPI WrappedAllocConsole()
{
  return TrueAllocConsole();
}

HHOOK WINAPI WrappedSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
  if (dwThreadId == 0 && idHook == WH_CBT)
  {
    dwThreadId = ::GetCurrentThreadId();
  }

  return TrueSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
}

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

UINT WINAPI WrappedGetDriveTypeA(LPCSTR lpRootPathName)
{
  if (lpRootPathName && VirtualDriveLetter == lpRootPathName)
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
  if (lpRootPathName && lpRootPathName == VirtualDriveLetter)
  {
    // TODO put current disc into environment variable
    // To be referenced when disc swapping can be implemented
    UNREFERENCED_PARAMETER(StarsiegeDisc1);
    UNREFERENCED_PARAMETER(StarsiegeBetaDisc);
    std::vector<char> data(nVolumeNameSize, '\0');
    std::copy(StarsiegeDisc2.begin(), StarsiegeDisc2.end(), data.begin());
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

static std::array<std::pair<void**, void*>, 5> detour_functions{ { { &(void*&)TrueSetWindowsHookExA, WrappedSetWindowsHookExA },
  { &(void*&)TrueGetLogicalDrives, WrappedGetLogicalDrives },
  { &(void*&)TrueGetDriveTypeA, WrappedGetDriveTypeA },
  { &(void*&)TrueGetVolumeInformationA, WrappedGetVolumeInformationA },
  { &(void*&)TrueAllocConsole, WrappedAllocConsole } } };

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
      int index = 0;
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
      DetourTransactionBegin();
      DetourUpdateThread(GetCurrentThread());

      std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourDetach(func.first, func.second); });
      DetourTransactionCommit();
    }
  }

  return TRUE;
}
}
