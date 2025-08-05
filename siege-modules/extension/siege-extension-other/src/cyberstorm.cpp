#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{};

constexpr static std::array<std::string_view, 8> verification_strings = { {
  "CSTORM.exe"sv,
  "CStorm.ini"sv,
  "CyberStorm"sv,
  "CyberStorm file"sv,
  "CyberStorm Multiplayer file"sv,
  "WinG"sv,
  "CYBRID_GAMEWIND"sv,
  "GFXCDSSurface"sv,
} };


std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}