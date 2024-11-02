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
  "GTR_ONLINE_GAMELIST"sv,
  "GTR_ONLINE_LOBBY"sv,
  "SUMMARY_MPLIST"sv,
  "GARAGEONLINEPAGE"sv,
  "RaceWTCC"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

#include "dll-main.hpp"
}