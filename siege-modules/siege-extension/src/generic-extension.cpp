#include <siege/platform/win/core/file.hpp>
#include <detours.h>

extern "C" {

HRESULT executable_is_supported(_In_ const wchar_t* filename) noexcept
{
  if (filename == nullptr)
  {
    return E_POINTER;
  }

  return (std::filesystem::exists(filename) && 
      std::filesystem::path(filename).extension() == ".exe" &&
      std::filesystem::path(filename).extension() == ".EXE") ? S_OK : S_FALSE;
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
  if (DetourIsHelperProcess())
  {
    return TRUE;
  }
}
}
