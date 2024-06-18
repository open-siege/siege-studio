#ifndef BIND_CONTEXT_HPP
#define BIND_CONTEXT_HPP

#include <siege/platform/win/core/com/base.hpp>
#include <expected>

extern "C" HRESULT __stdcall GetModuleBindCtx(::IBindCtx** context) noexcept;

namespace win32::com
{
  inline std::expected<com_ptr<::IBindCtx>, HRESULT> GetModuleBindCtx() noexcept
  {
    win32::com::com_ptr<::IBindCtx> result;

    auto hresult = ::GetModuleBindCtx(result.put());

    if (hresult != S_OK)
    {
      return std::unexpected(hresult);
    }

    return result;
  }
}// namespace win32::com


#endif