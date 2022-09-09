// Using https://github.com/microsoft/Detours/wiki/Using-Detours as a starting point

#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <bitset>
#include <array>
#include <map>
#include <atomic>
#include <cstdlib>
#include <windows.h>
#include <detours.h>
#include "Hook/Game.hpp"
#include "Hook/GameConsole.hpp"
#include "Registry.hpp"
#include "System.hpp"
#include "Input/winmm.hpp"

static std::atomic_bool game_extended = false;

void runExtender() noexcept
{
  const auto* mode = std::getenv("DARKSTAR_MODE");

  if (!mode)
  {
    return;
  }

  if (mode != std::string_view{"starsiege"})
  {
    return;
  }

  try
  {
    std::ofstream file{ "test.log" };
    file << "Hello there!" << std::endl;

    auto game = Hook::Game::currentInstance();

    if (!game->isLoaded())
    {
      return;
    }

    auto console = game->getConsole();

    struct local_consumer : Core::ExternalConsoleConsumer
    {
      void APICALL doWriteLine(Core::GameConsole*, const char* consoleLine) override
      {
        std::ofstream file{ "test.log", std::ios::app };
        file << "Consumer message: " << consoleLine << '\n';
      }
    };

    console->addConsumer(new local_consumer());

    file << "init game runtime: " << game.get() << std::endl;
    file << "init console: " << console.get() << std::endl;
    file << console->echoRange(std::array<std::string, 1>{ "Hello world from echo in C++ with array" }) << std::endl;
    file << console->echoRange(std::vector<std::string>{ "Hello world from echo in C++ with vector" }) << std::endl;
    file << console->dbecho(std::array<std::string, 2>{ "1", "Hello world from dbecho in C++ with array" }) << std::endl;
    file << console->dbecho(std::vector<std::string>{ "1", "Hello world from dbecho in C++ with vector" }) << std::endl;
    file << console->strcat(std::array<std::string, 1>{ "Hello world from strcat in C++ with array" }) << std::endl;
    file << console->strcat(std::vector<std::string>{ "Hello world from strcat in C++ with vector" }) << std::endl;
    file << console->eval("echo(\"Hello world from eval in C++\");");

    std::ofstream newScript{ "test-script.cs" };
    newScript << "echo(\"Hello world from exec in C++ from test-script.cs\");" << std::endl;
    newScript.close();
    file << console->exec("test-script.cs");

    file << "Floor of 1.5: " << console->floor("1.5") << std::endl;
    file << "console.exportFunctions: " << console->exportFunctions("*", "exportFunctions.cs", "False") << std::endl;
    file << "console.exportVariables: " << console->exportVariables("*", "exportVariables.cs", "False") << std::endl;
    file << "Sqrt of 144: " << console->sqrt("144") << std::endl;

    auto plugins = game->getPlugins();

    file << "Number of plugins inside of game: " << plugins.size() << " "
         << plugins.capacity() << " "
         << std::endl;

    file << "raw console: " << console->getRaw() << std::endl;
    // TODO this is off in 1004. Need to find the correct offsets.
    // file << "raw plugin console: " << plugins[0]->console << std::endl;
  }
  catch (const std::exception& ex)
  {
    std::ofstream file{ "darkstar-hook-errors.log", std::ios_base::app };
    file << ex.what() << std::endl;
  }
}

static std::ofstream file{ "detours.log" };

static auto* TrueCreateFileA = CreateFileA;
static auto* TrueSetWindowsHookExA = SetWindowsHookExA;
static auto* TrueGetLogicalDrives = GetLogicalDrives;
static auto* TrueGetDriveTypeA = GetDriveTypeA;
static auto* TrueGetVolumeInformationA = GetVolumeInformationA;
static auto* TrueAllocConsole = AllocConsole;

static std::string VirtualDriveLetter;
constexpr static std::string_view StarsiegeDisc1 = "STARSIEGE1";
constexpr static std::string_view StarsiegeDisc2 = "STARSIEGE2";
constexpr static std::string_view StarsiegeBetaDisc = "STARSIEGE";


BOOL WINAPI WrappedAllocConsole()
{
  if (!game_extended)
  {
    runExtender();
    game_extended = true;
  }

  return TrueAllocConsole();
}

HHOOK WINAPI WrappedSetWindowsHookExA(
  int idHook,
  HOOKPROC lpfn,
  HINSTANCE hmod,
  DWORD dwThreadId)
{
  if (!game_extended)
  {
    runExtender();
    game_extended = true;
  }

  if (dwThreadId == 0)
  {
    dwThreadId = GetCurrentThreadId();
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

HANDLE WINAPI WrappedCreateFileA(
  LPCSTR lpFileName,
  DWORD dwDesiredAccess,
  DWORD dwShareMode,
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  DWORD dwCreationDisposition,
  DWORD dwFlagsAndAttributes,
  HANDLE hTemplateFile)
{
  file << "Opening " << lpFileName << '\n';

  return TrueCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

auto DetourAttach(std::pair<void**, void*>& value)
{
  return DetourAttach(value.first, value.second);
}

auto DetourDetach(std::pair<void**, void*>& value)
{
  return DetourDetach(value.first, value.second);
}

static std::array<std::pair<void**, void*>, 6> functions{ { { &(void*&)TrueCreateFileA, WrappedCreateFileA },
  { &(void*&)TrueSetWindowsHookExA, WrappedSetWindowsHookExA },
  { &(void*&)TrueGetLogicalDrives, WrappedGetLogicalDrives },
  { &(void*&)TrueGetDriveTypeA, WrappedGetDriveTypeA },
  { &(void*&)TrueGetVolumeInformationA, WrappedGetVolumeInformationA },
  { &(void*&)TrueAllocConsole, WrappedAllocConsole } } };

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
  if (DetourIsHelperProcess())
  {
    file << "Detour helper process\n";
    return TRUE;
  }

  static auto RegistryFunctions = GetRegistryDetours();
  static auto WinmmFunctions = winmm::GetWinmmDetours();
  static auto SystemFunctions = core::GetSystemDetours();

  if (dwReason == DLL_PROCESS_ATTACH)
  {
    file << "Attaching to dll\n";
    DetourRestoreAfterWith();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    std::for_each(functions.begin(), functions.end(), DetourAttach);
    std::for_each(RegistryFunctions.begin(), RegistryFunctions.end(), DetourAttach);
    std::for_each(WinmmFunctions.begin(), WinmmFunctions.end(), DetourAttach);
    std::for_each(SystemFunctions.begin(), SystemFunctions.end(), DetourAttach);

    DetourTransactionCommit();
  }
  else if (dwReason == DLL_PROCESS_DETACH)
  {
    file << "Detaching from dll\n";
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    std::for_each(functions.begin(), functions.end(), DetourDetach);
    std::for_each(RegistryFunctions.begin(), RegistryFunctions.end(), DetourDetach);
    std::for_each(WinmmFunctions.begin(), WinmmFunctions.end(), DetourDetach);
    std::for_each(SystemFunctions.begin(), SystemFunctions.end(), DetourDetach);
    DetourTransactionCommit();
  }

  return TRUE;
}

extern "C" {
extern void* _cdecl MS_Malloc(std::size_t size) noexcept
{
  return std::malloc(size);
}

extern void _cdecl MS_Free(void* data) noexcept
{
  std::free(data);
}

extern void* _cdecl MS_Realloc(void* data, std::size_t size) noexcept
{
  return std::realloc(data, size);
}

extern void* _cdecl MS_Calloc(std::size_t num, std::size_t size) noexcept
{
  return std::calloc(num, size);
}
}
