
#include <siege/platform/win/hresult.hpp>
#include <siege/platform/presentation_module.hpp>
#include "views/vol_shared.hpp"

using namespace siege::views;
using storage_info = siege::platform::storage_info;

static ATOM vol_view_atom;

namespace siege::views
{
  ATOM register_vol_view(win32::window_module_ref module);
}

extern "C" {
extern const std::uint32_t default_file_icon = SIID_ZIPFILE;

HRESULT get_window_class_for_stream(storage_info* data, wchar_t** class_name) noexcept
{
  if (!data)
  {
    return E_INVALIDARG;
  }

  if (!class_name)
  {
    return E_INVALIDARG;
  }

  auto stream = siege::platform::create_istream(*data);

  static auto this_module = win32::window_module_ref::current_module();

  if (!siege::views::is_vol(*stream))
  {
    return S_FALSE;
  }

  static std::wstring storage;
  
  if (!storage.empty())
  {
    *class_name = storage.data();
    return S_OK;
  }

  auto window = this_module.CreateWindowExW(CREATESTRUCTW{
    .hwndParent = HWND_MESSAGE,
    .lpszClass = MAKEINTATOM(vol_view_atom) });

  if (!window)
  {
    return S_FALSE;
  }

  storage.resize(255);
  storage.resize(::GetClassNameW(*window, storage.data(), (int)storage.size()));
  ::DestroyWindow(*window);
  window->release();

  *class_name = storage.data();
  return S_OK;
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,// handle to DLL module
  DWORD fdwReason,// reason for calling function
  LPVOID lpvReserved)// reserved
{

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    if (lpvReserved != nullptr)
    {
      return TRUE;// do not do cleanup if process termination scenario
    }

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      vol_view_atom = register_vol_view(win32::window_module_ref(hinstDLL));
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      ::UnregisterClassW(MAKEINTATOM(vol_view_atom), hinstDLL);
    }
  }

  return TRUE;
}
}