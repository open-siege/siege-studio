import std;
import win32.window;
namespace fs = std::filesystem;

extern "C" std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return std::errc::not_supported;
}