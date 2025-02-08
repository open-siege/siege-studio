#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{};

constexpr static std::array<std::string_view, 18> verification_strings = { {
  "ADS:RES:"sv,
  "ADS:SCR:"sv,
  "BMP:INF:"sv,
  "BMP:BIN:"sv,
  "BMP:VGA:"sv,
  "BMP:AMG:"sv,
  "SCR:BIN:"sv,
  "SCR:VGA:"sv,
  "SCR:AMG:"sv,
  "PAL:VGA:"sv,
  "PAL:EGA:"sv,
  "PAL:CGA:"sv,
  "PAL:AMG:"sv,
  "REQ:REQ"sv,
  "REQ:GAD"sv,
  "chinese.fnt"sv
  "train.bmp"sv
  "tank.wld"sv
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}