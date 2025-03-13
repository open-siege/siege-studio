#include <windows.h>

// TODO implement kernel32 wrappers for XP support
extern "C" {
BOOL SiegeGetFileInformationByHandleEx(HANDLE hFile, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInformation, DWORD dwBufferSize)
{
  return FALSE;
}

BOOL SiegeInitializeCriticalSectionEx(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount, DWORD Flags)
{
  return FALSE;
}
}