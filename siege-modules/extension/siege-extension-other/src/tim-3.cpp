#include <siege/platform/win/window_module.hpp>
namespace fs = std::filesystem;

extern "C" std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  if (filename == nullptr)
  {
    return std::errc::bad_address;
  }

  std::error_code last_error;

  if (!fs::exists(filename, last_error))
  {
    return std::errc::invalid_argument;
  }

  fs::path exe_path = filename;
  fs::path parent_path = exe_path.parent_path();

  if (exe_path.stem() == "TIMWIN" && exe_path.extension() == ".EXE" && fs::exists(parent_path / "SOSLIBS2.DLL", last_error) && fs::exists(parent_path / "SOS32S02.DLL", last_error) && fs::exists(parent_path / "SOS9502.DLL", last_error))
  {
    return std::errc{};
  }

  return std::errc::not_supported;
}