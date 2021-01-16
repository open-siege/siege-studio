// Using https://github.com/microsoft/Detours/wiki/Using-Detours as a starting point

#include <windows.h>
#include <detours.h>
#include <fstream>

extern "C" void detours_init(){}

static std::ofstream file{ "detours.log" };

static auto* TrueCreateFileA = CreateFileA;

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

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueCreateFileA, WrappedCreateFileA);
        DetourTransactionCommit();
    } else if (dwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueCreateFileA, WrappedCreateFileA);
        DetourTransactionCommit();
    }

    return TRUE;
}