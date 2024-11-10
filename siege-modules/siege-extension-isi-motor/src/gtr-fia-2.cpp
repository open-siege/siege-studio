#include <siege/platform/win/desktop/window_module.hpp>
#include "shared.hpp"
#include <detours.h>


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

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "GAME.DIC"sv,
  "[FINAL_DRIVE]"sv,
  "[GEAR_RATIOS]"sv,
  "gtr2ui.mnu"sv,
  "specialfx.tec"sv,
  "GTR_ONLINE_LOBBY"sv,
  "GTR_ONLINE_LOGIN"sv,
  "GTR_ONLINE_CDKEY"sv,
  "GARAGEONLINEPAGE"sv,
  "STEX.GTR"sv,
  "Net.dll"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

HRESULT apply_prelaunch_settings(const wchar_t* exe_path_str, const siege::platform::game_command_line_args* args)
{
  return S_OK;
}
}