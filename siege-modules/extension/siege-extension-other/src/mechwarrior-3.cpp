#include <siege/platform/win/window_module.hpp>
namespace fs = std::filesystem;

extern "C" std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return std::errc::not_supported;
}