#include <siege/platform/win/desktop/window_module.hpp>

extern "C" {
HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return S_FALSE;
}
}