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

  if (exe_path.stem() == "WILLY" && exe_path.extension() == ".EXE" && fs::exists(parent_path / "WILLY.TFA", last_error) && fs::exists(parent_path / "RESOURCE.MAP", last_error) && fs::exists(parent_path / "RESOURCE.EXE", last_error))
  {
    return std::errc{};
  }

  return std::errc::not_supported;
}