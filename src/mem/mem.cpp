#include <windows.h>

extern "C" void detours_init();

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
    detours_init();
    return TRUE;
}