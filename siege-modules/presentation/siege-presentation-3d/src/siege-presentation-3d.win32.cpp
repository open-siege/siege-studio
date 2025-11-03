#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/hresult.hpp>
#include <siege/platform/stream.hpp>
#include "views/3d_shared.hpp"

static ATOM dts_view_atom;
static ATOM dml_view_atom;

namespace siege::views
{
  ATOM register_dts_view(win32::window_module_ref module);
  ATOM register_dml_view(win32::window_module_ref module);
}// namespace siege::views


using namespace siege::views;
using storage_info = siege::platform::storage_info;

extern "C" {
extern const std::uint32_t default_file_icon = SIID_IMAGEFILES;

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

  if (!(is_shape(*stream) || is_material(*stream)))
  {
    return S_FALSE;
  }

  if (is_shape(*stream))
  {
    static std::wstring storage;

    if (!storage.empty())
    {
      *class_name = storage.data();
      return S_OK;
    }

    auto window = this_module.CreateWindowExW(CREATESTRUCTW{
      .hwndParent = HWND_MESSAGE,
      .lpszClass = MAKEINTATOM(dts_view_atom) });

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

  if (is_material(*stream))
  {
    static std::wstring storage;

    if (!storage.empty())
    {
      *class_name = storage.data();
      return S_OK;
    }

    auto window = this_module.CreateWindowExW(CREATESTRUCTW{
      .hwndParent = HWND_MESSAGE,
      .lpszClass = MAKEINTATOM(dml_view_atom) });

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

  return S_FALSE;
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    if (lpvReserved != nullptr)
    {
      return TRUE;// do not do cleanup if process termination scenario
    }

    win32::window_module_ref this_module(hinstDLL);

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      dts_view_atom = register_dts_view(win32::window_module_ref(hinstDLL));
      dml_view_atom = register_dml_view(win32::window_module_ref(hinstDLL));
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      ::UnregisterClassW(MAKEINTATOM(dts_view_atom), hinstDLL);
      ::UnregisterClassW(MAKEINTATOM(dml_view_atom), hinstDLL);
    }
  }

  return TRUE;
}
}