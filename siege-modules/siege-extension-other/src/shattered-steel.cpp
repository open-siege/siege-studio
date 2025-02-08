#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"-netgame",
  .string_settings = { { L"-netgame" } }
};

constexpr static std::array<std::string_view, 17> verification_strings = { {
  "ssteel.exe"sv,
  "ssteel.ini"sv,
  "-nocd"sv,
  "-control"sv,
  "-socket"sv,
  "-handle"sv,
  "-ipxgame"sv,
  "-netgame"sv,
  "-timeout"sv,
  "-noloop"sv,
  "-nopoll"sv,
  "-window"sv,
  "-debug"sv,
  "-half"sv,
  "-dithered"sv,
  "-no640"sv,
  "SVRDOS32.DRV"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}