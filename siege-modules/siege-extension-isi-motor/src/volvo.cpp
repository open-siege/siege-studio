#include <siege/platform/win/desktop/window_module.hpp>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

constexpr static std::array<std::string_view, 10> verification_strings = { {
  "options.dic"sv,
  "[FINAL_DRIVE]"sv,
  "[GEAR_RATIOS]"sv,
  "RATE_TIMETRIAL"sv,
  "specialfx.tec"sv,
  "GTR_ONLINE_LOBBY"sv,
  "GTR_ONLINE_GAMELIST"sv,
  "ONLINECARSETUPS"sv,
  "VolvoIntro.bik"sv,
  "STEX"sv,
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