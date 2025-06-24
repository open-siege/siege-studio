#include <siege/platform/win/window_impl.hpp>
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

  static std::wstring storage;
  *class_name = storage.data();

  auto stream = siege::platform::create_istream(*data);

  try
  {
    static auto this_module = win32::window_module_ref::current_module();

    if (siege::views::is_vol(*stream))
    {
      // TODO overhaul the presentation layer to use ATOMs for class names exclusively
      if (storage.empty())
      {
        auto window = this_module.CreateWindowExW(CREATESTRUCTW{
          .hwndParent = HWND_MESSAGE,
          .lpszClass = MAKEINTATOM(vol_view_atom) });

        if (window)
        {
          storage.resize(255);
          storage.resize(::GetClassNameW(*window, storage.data(), (int)storage.size()));
          ::DestroyWindow(*window);
          window->release();
        }
      }

      *class_name = storage.data();
      return S_OK;
    }

    return S_FALSE;
  }
  catch (...)
  {
    return S_FALSE;
  }
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

    static win32::hwnd_t info_instance = nullptr;

    static std::wstring module_file_name(255, '\0');
    GetModuleFileNameW(hinstDLL, module_file_name.data(), module_file_name.size());

    std::filesystem::path module_path(module_file_name.data());

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