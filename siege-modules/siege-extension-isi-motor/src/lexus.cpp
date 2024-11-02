#include <siege/platform/win/desktop/window_module.hpp>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  if (filename == nullptr)
  {
    return E_POINTER;
  }

  if (!std::filesystem::exists(filename))
  {
    return E_INVALIDARG;
  }

  auto exe_path = std::filesystem::path(filename);
  auto parent_path = exe_path.parent_path();

  if (exe_path.stem() == "rFactorLexusISF" && exe_path.extension() == ".exe" && std::filesystem::exists(parent_path / "libmysql.dll"))
  {
    return S_OK;
  }

  return S_FALSE;
}
}