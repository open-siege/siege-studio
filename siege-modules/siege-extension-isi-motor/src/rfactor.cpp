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

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "ui.dic"sv,
  "[FINAL_DRIVE]"sv,
  "[GEAR_RATIOS]"sv,
  "MPLOBBY_MINI_CHAT"sv,
  "specialfx.cpp"sv,
  "RFACTOR_LOGO.TGA"sv,
  "rFactor"sv,
  "PITANIMS.MAS"sv,
  "MONITORPAGE"sv,
  "LOADINGSCREENPAGE"sv,
  "Global\\rFactor_APP_MUTEX_"sv,
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