#include <siege/platform/win/window_module.hpp>
namespace fs = std::filesystem;


extern "C" HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  if (filename == nullptr)
  {
    return E_POINTER;
  }

  std::error_code last_error;

  if (!fs::exists(filename, last_error))
  {
    return E_INVALIDARG;
  }

  fs::path exe_path = filename;
  fs::path parent_path = exe_path.parent_path();

  if (exe_path.stem() == "HOC" && exe_path.extension() == ".EXE" && fs::exists(parent_path / "TANK.TBL", last_error) && fs::exists(parent_path / "VOLUME.RMF", last_error) && fs::exists(parent_path / "RESOURCE.CFG", last_error))
  {
    return S_OK;
  }

  return S_FALSE;
}