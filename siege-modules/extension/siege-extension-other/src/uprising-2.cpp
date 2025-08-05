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

  auto exe_path = fs::path(filename);
  auto parent_path = exe_path.parent_path();

  if (exe_path.stem() == "Uprising 2" && exe_path.extension() == ".exe" && fs::exists(parent_path / "uprising.cln", last_error) && fs::exists(parent_path / "ramlockC.VXD", last_error))
  {
    return std::errc{};
  }

  return std::errc::not_supported;
}