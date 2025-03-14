#include <siege/platform/win/window.hpp>

namespace win32
{
  bool is_parent_from_system(HWND window)
  {
    if (!window)
    {
      return false;
    }

    if (window == HWND_MESSAGE)
    {
      return true;
    }

    auto module = (HMODULE)::GetClassLongPtrW(window, GCLP_HMODULE);

    if (!module)
    {
      return false;
    }

    if (module == ::GetModuleHandleW(L"comdlg32") ||
        module == ::GetModuleHandleW(L"shell32") ||
        module == ::GetModuleHandleW(L"user32") ||
        module == ::GetModuleHandleW(L"ExplorerFrame") ||
        module == ::GetModuleHandleW(L"shlwapi"))
    {
        return true;
    }

    return false;
  }
}// namespace win32