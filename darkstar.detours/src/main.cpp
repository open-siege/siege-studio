// Using https://github.com/microsoft/Detours/wiki/Using-Detours as a starting point

#include <fstream>
#include <vector>
#include <algorithm>
#include <bitset>
#include <string_view>
#include <array>
#include <map>
#include <windows.h>
#include <detours.h>


extern "C" void detours_init(){}

static std::ofstream file{ "detours.log" };

static auto* TrueCreateFileA = CreateFileA;
static auto* TrueSetWindowsHookExA = SetWindowsHookExA;
static auto* TrueGetLogicalDrives = GetLogicalDrives;
static auto* TrueGetDriveTypeA = GetDriveTypeA;
static auto* TrueGetVolumeInformationA = GetVolumeInformationA;


static std::string VirtualDriveLetter;
const static std::string StarsiegeDisc1 = "STARSIEGE1";
const static std::string StarsiegeDisc2 = "STARSIEGE2";
const static std::string StarsiegeBetaDisc = "STARSIEGE";

HHOOK WINAPI WrappedSetWindowsHookExA(
        int       idHook,
        HOOKPROC  lpfn,
        HINSTANCE hmod,
        DWORD     dwThreadId
)
{
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
        {&(void*&)TrueGetVolumeInformationA, WrappedGetVolumeInformationA}
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