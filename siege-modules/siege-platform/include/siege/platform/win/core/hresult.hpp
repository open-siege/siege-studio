#ifndef HRESULT_MODULE_HPP
#define HRESULT_MODULE_HPP

#include <system_error>
#include <wtypes.h>
#include <WinDef.h>
#include <WinBase.h>

extern "C" {
HRESULT std_errc_to_hresult(std::errc code)
{
  switch (code)
  {
  case std::errc(0):
    return S_OK;
  case std::errc::connection_aborted:
    return E_ABORT;
  case std::errc::permission_denied:
    return E_ACCESSDENIED;
  case std::errc::function_not_supported:
    return E_NOTIMPL;
  case std::errc::not_supported:
    return E_NOINTERFACE;
  case std::errc::invalid_argument:
    return E_INVALIDARG;
  case std::errc::not_enough_memory:
    return E_OUTOFMEMORY;
  case std::errc::operation_in_progress:
    return E_PENDING;
  case std::errc::operation_canceled:
    return HRESULT_FROM_WIN32(ERROR_CANCELLED);
  case std::errc::no_such_device_or_address:
    return E_HANDLE;
  case std::errc::bad_message:
    return HRESULT_FROM_WIN32(ERROR_BAD_COMMAND);
  default:
    return E_FAIL;
  }
}
}

#endif