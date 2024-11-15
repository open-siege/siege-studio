#include <siege/platform/win/desktop/window_module.hpp>
#include <detours.h>
#include <filesystem>
#include <fstream>
#include "shared.hpp"


extern "C" {
using namespace std::literals;
namespace fs = std::filesystem;
using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"+connect",
  .string_settings = { { L"+connect" } }
};

// +connect IP:port
// +connect IP:0

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "GAME.DIC"sv,
  "[FINAL_DRIVE]"sv,
  "[GEAR_RATIOS]"sv,
  "GTR_ONLINE_LOBBY_ARCADE"sv,
  "specialfx.tec"sv,
  "GTR_ONLINE_LOBBY"sv,
  "GTR_ONLINE_LOGIN"sv,
  "GTR_ONLINE_CDKEY"sv,
  "GARAGEONLINEPAGE"sv,
  "STEX.GTL"sv,
  "Net.dll"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

HRESULT apply_prelaunch_settings(const wchar_t* exe_path_str, const siege::platform::game_command_line_args* args)
{
  if (exe_path_str == nullptr)
  {
    return E_POINTER;
  }

  if (args == nullptr)
  {
    return E_POINTER;
  }

  std::error_code last_error;

  auto exe_path = fs::path(exe_path_str);

  auto parent_path = exe_path.parent_path();

  if (!fs::exists(parent_path / "TG2001.DYN", last_error))
  {
    std::ofstream temp(parent_path / "TG2001.DYN", std::ios::trunc);
  }

  return S_OK;
}
}