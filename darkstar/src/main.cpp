// Using https://github.com/microsoft/Detours/wiki/Using-Detours as a starting point

#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <bitset>
#include <array>
#include <map>
#include <atomic>
#include <windows.h>
#include <detours.h>
#include "Hook/Game.hpp"
#include "Hook/GameConsole.hpp"

static std::atomic_bool game_extended = false;

void runExtender() noexcept
{
    try
    {
        std::ofstream file{ "test.log" };
        file << "Hello there!" << std::endl;

        auto game = Hook::Game::currentInstance();

        auto console = game->getConsole();

        struct local_consumer : Core::ExternalConsoleConsumer
        {
            void APICALL doWriteLine(Core::GameConsole*, const char *consoleLine) override
            {
                std::ofstream file{ "test.log", std::ios::app };
                file << "Consumer message: " << consoleLine << '\n';
            }
        };

        console->addConsumer(new local_consumer());

        file << "init game runtime: " << game.get() << std::endl;
        file << "init console: " << console.get() << std::endl;
        file << console->echoRange(std::array<std::string, 1>{"Hello world from echo in C++ with array"}) << std::endl;
        file << console->echoRange(std::vector<std::string>{"Hello world from echo in C++ with vector"}) << std::endl;
        file << console->dbecho(std::array<std::string, 2>{"1", "Hello world from dbecho in C++ with array"}) << std::endl;
        file << console->dbecho(std::vector<std::string>{"1", "Hello world from dbecho in C++ with vector"}) << std::endl;
        file << console->strcat(std::array<std::string, 1>{"Hello world from strcat in C++ with array"}) << std::endl;
        file << console->strcat(std::vector<std::string>{"Hello world from strcat in C++ with vector"}) << std::endl;
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
        file << "raw plugin console: " << plugins[0]->console << std::endl;
    }
    catch (const std::exception & ex)
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

static auto* TrueRegCloseKey = RegCloseKey;
static auto* TrueRegCreateKeyExA = RegCreateKeyExA;
static auto* TrueRegDeleteKeyA = RegDeleteKeyA;
static auto* TrueRegEnumKeyExA = RegEnumKeyExA;
static auto* TrueRegEnumValueA = RegEnumValueA;
static auto* TrueRegOpenKeyExA = RegOpenKeyExA;
static auto* TrueRegQueryValueExA = RegQueryValueExA;
static auto* TrueRegSetValueExA = RegSetValueExA;

static std::string VirtualDriveLetter;
constexpr static std::string_view StarsiegeDisc1 = "STARSIEGE1";
constexpr static std::string_view StarsiegeDisc2 = "STARSIEGE2";
constexpr static std::string_view StarsiegeBetaDisc = "STARSIEGE";

constexpr static std::array<std::string_view, 3> SettingsPath = {
                                                                 "software",
                                                                 "dynamix",
                                                                 "starsiege" };

static std::map<std::string_view, HKEY> LoadedPaths;

LSTATUS APIENTRY WrappedRegCloseKey(HKEY hKey)
{
    for (auto& [Name, Key] : LoadedPaths)
    {
        if (Key == hKey)
        {
            LoadedPaths.erase(Name);
            return ERROR_SUCCESS;
        }
    }
    file << "Closing @" << hKey << '\n';
    return TrueRegCloseKey(hKey);
}

LSTATUS APIENTRY WrappedRegCreateKeyExA(
        HKEY                        hKey,
        LPCSTR                      lpSubKey,
        DWORD                       Reserved,
        LPSTR                       lpClass,
        DWORD                       dwOptions,
        REGSAM                      samDesired,
        const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        PHKEY                       phkResult,
        LPDWORD                     lpdwDisposition
)
{
    file << "Creating registry key @" << hKey << " " << lpSubKey << " @ " << lpClass << '\n';
    return TrueRegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

LSTATUS APIENTRY WrappedRegDeleteKeyA(
        HKEY   hKey,
        LPCSTR lpSubKey
)
{
    file << "Deleting registry key @" << hKey << " " << lpSubKey << '\n';
    return TrueRegDeleteKeyA(hKey, lpSubKey);
}

LSTATUS APIENTRY WrappedRegEnumKeyExA(
        HKEY      hKey,
        DWORD     dwIndex,
        LPSTR     lpName,
        LPDWORD   lpcchName,
        LPDWORD   lpReserved,
        LPSTR     lpClass,
        LPDWORD   lpcchClass,
        PFILETIME lpftLastWriteTime
)
{
    file << "Enum key @" << hKey << " " << lpName << " " << lpClass << '\n';
    return TrueRegEnumKeyExA(hKey, dwIndex, lpName, lpcchName, lpReserved, lpClass, lpcchClass, lpftLastWriteTime);
}

LSTATUS APIENTRY WrappedRegEnumValueA(HKEY    hKey,
                          DWORD   dwIndex,
                          LPSTR   lpValueName,
                          LPDWORD lpcchValueName,
                          LPDWORD lpReserved,
                          LPDWORD lpType,
                          LPBYTE  lpData,
                          LPDWORD lpcbData
)
{
    file << "Enum value @" << hKey << " " << lpValueName << '\n';
    return TrueRegEnumValueA(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);
}

LSTATUS APIENTRY WrappedRegOpenKeyExA(
        HKEY   hKey,
        LPCSTR lpSubKey,
        DWORD  ulOptions,
        REGSAM samDesired,
        PHKEY  phkResult
)
{
    std::string Temp(lpSubKey);
    std::transform(Temp.begin(), Temp.end(), Temp.begin(),
                   [](char c){ return std::tolower(c); });

    if (hKey == HKEY_LOCAL_MACHINE && Temp == SettingsPath[0])
    {
        auto alreadyOpened = LoadedPaths.find(SettingsPath[0]);

        if (alreadyOpened != LoadedPaths.end())
        {
            *phkResult = alreadyOpened->second;
            return ERROR_SUCCESS;
        }

        auto result = TrueRegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);

        if (result == ERROR_SUCCESS)
        {
            LoadedPaths.emplace(std::make_pair(SettingsPath[0], *phkResult));
        }
        return result;
    }

    for (auto& [Name, Key] : LoadedPaths)
    {
        file << "Going through cache for " << Name << '\n';
        for (auto i = 0; i < 2; ++i)
        {
            if (Key == hKey && Name == SettingsPath[i] && Temp == SettingsPath[i + 1])
            {
                auto New = LoadedPaths.emplace(std::make_pair(SettingsPath[i + 1], reinterpret_cast<HKEY>(i + 1)));

                *phkResult = New.first->second;
                return ERROR_SUCCESS;
            }
        }
    }


    file << "Opening registry key @" << hKey << " " << lpSubKey << '\n';
    return TrueRegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

LSTATUS APIENTRY WrappedRegQueryValueExA(
        HKEY    hKey,
        LPCSTR  lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE  lpData,
        LPDWORD lpcbData
)
{
    std::string Temp(lpValueName);
    std::transform(Temp.begin(), Temp.end(), Temp.begin(),
                   [](char c){ return std::tolower(c); });

    for (auto& [Name, Key] : LoadedPaths)
    {
        if (Name == SettingsPath.back() && Key == hKey && lpData && lpcbData)
        {
            if (Temp == "path")
            {
                auto CurrentDir = std::filesystem::current_path().string();
                auto Size = *lpcbData;
                Size = Size > CurrentDir.size() ? Size : CurrentDir.size();
                std::copy(CurrentDir.data(), CurrentDir.data() + Size, lpData);
                file << "Returning fake data for Path\n";
                return ERROR_SUCCESS;
            }
            else if (Temp == "installtype")
            {
                constexpr static std::string_view Type = "Full";
                std::copy(Type.data(), Type.data() + Type.size(), lpData);

                file << "Returning fake data for InstallType\n";
                return ERROR_SUCCESS;
            }
        }
    }

    file << "Querying registry key @" << hKey << " " << lpValueName << '\n';
    return TrueRegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

LSTATUS APIENTRY WrappedRegSetValueExA(
        HKEY       hKey,
        LPCSTR     lpValueName,
        DWORD      Reserved,
        DWORD      dwType,
        const BYTE *lpData,
        DWORD      cbData
)
{
    file << "Setting value @" << hKey << " " << lpValueName << '\n';
    return TrueRegSetValueExA(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

BOOL WINAPI WrappedAllocConsole()
{
    if(!game_extended)
    {
        runExtender();
        game_extended = true;
    }

    return TrueAllocConsole();
}

HHOOK WINAPI WrappedSetWindowsHookExA(
        int       idHook,
        HOOKPROC  lpfn,
        HINSTANCE hmod,
        DWORD     dwThreadId
)
{
    if(!game_extended)
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
        LPCSTR  lpRootPathName,
        LPSTR   lpVolumeNameBuffer,
        DWORD   nVolumeNameSize,
        LPDWORD lpVolumeSerialNumber,
        LPDWORD lpMaximumComponentLength,
        LPDWORD lpFileSystemFlags,
        LPSTR   lpFileSystemNameBuffer,
        DWORD   nFileSystemNameSize
)
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
        LPCSTR                lpFileName,
        DWORD                 dwDesiredAccess,
        DWORD                 dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD                 dwCreationDisposition,
        DWORD                 dwFlagsAndAttributes,
        HANDLE                hTemplateFile
)
{
    file << "Opening " << lpFileName << '\n';

    return TrueCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

static std::map<void**, void*> functions {
        {&(void*&)TrueCreateFileA, WrappedCreateFileA},
        {&(void*&)TrueSetWindowsHookExA, WrappedSetWindowsHookExA},
        {&(void*&)TrueGetLogicalDrives, WrappedGetLogicalDrives},
        {&(void*&)TrueGetDriveTypeA, WrappedGetDriveTypeA},
        {&(void*&)TrueGetVolumeInformationA, WrappedGetVolumeInformationA},
        {&(void*&)TrueAllocConsole, WrappedAllocConsole},
        {&(void*&)TrueRegCloseKey, WrappedRegCloseKey},
        {&(void*&)TrueRegCreateKeyExA, WrappedRegCreateKeyExA},
        {&(void*&)TrueRegDeleteKeyA, WrappedRegDeleteKeyA},
        {&(void*&)TrueRegEnumKeyExA, WrappedRegEnumKeyExA},
        {&(void*&)TrueRegEnumValueA, WrappedRegEnumValueA},
        {&(void*&)TrueRegOpenKeyExA, WrappedRegOpenKeyExA},
        {&(void*&)TrueRegQueryValueExA, WrappedRegQueryValueExA},
        {&(void*&)TrueRegSetValueExA, WrappedRegSetValueExA}
};

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        for (auto& [original, wrapper] : functions)
        {
            DetourAttach(original, wrapper);
        }

        DetourTransactionCommit();
    } else if (dwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        for (auto& [original, wrapper] : functions)
        {
            DetourDetach(original, wrapper);
        }
        DetourTransactionCommit();
    }

    return TRUE;
}

extern "C"
{
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