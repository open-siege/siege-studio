#include <siege/platform/win/desktop/window_impl.hpp>
#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/core/hresult.hpp>
#include <siege/platform/stream.hpp>
#include "views/dts_view.hpp"

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

  std::wstring empty;
  *class_name = empty.data();

  auto stream = siege::platform::create_istream(*data);

  try
  {
    static auto this_module = win32::window_module_ref::current_module();

    if (dts_controller::is_dts(*stream))
    {
      static auto window_type_name = win32::type_name<dts_view>();

      if (this_module.GetClassInfoExW(window_type_name))
      {
        *class_name = window_type_name.data();
        return S_OK;
      }
    }

    return S_FALSE;
  }
  catch (...)
  {
    return S_FALSE;
  }
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
      this_module.RegisterClassExW(win32::window_meta_class<dts_view>());
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      this_module.UnregisterClassW<dts_view>();
    }
  }

  return TRUE;
}
}