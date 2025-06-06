#include <siege/platform/win/window_module.hpp>
namespace fs = std::filesystem;

extern "C" HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return S_FALSE;
}