#include <siege/platform/win/window_module.hpp>
#include "shared.hpp"


extern "C" {
using namespace std::literals;
using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"+connect",
  .string_settings = { { L"+connect" } }
};

// +connect IP:port
// +connect IP:0	
// HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers
// Name == full path
// value == WINXPSP3 HIGHDPIAWARE

HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  if (filename == nullptr)
  {
    return E_POINTER;
  }

  std::error_code last_error;

  if (!std::filesystem::exists(filename, last_error))
  {
    return E_INVALIDARG;
  }

  auto exe_path = std::filesystem::path(filename);
  auto parent_path = exe_path.parent_path();

  if (exe_path.stem() == "rFactorLexusISF" && exe_path.extension() == ".exe" && 
      std::filesystem::exists(parent_path / "libmysql.dll", last_error))
  {
    return S_OK;
  }

  return S_FALSE;
}

HRESULT apply_prelaunch_settings(const wchar_t* exe_path_str, const siege::platform::game_command_line_args* args)
{
  return S_OK;
}

}