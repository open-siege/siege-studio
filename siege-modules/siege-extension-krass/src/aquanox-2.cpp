#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"+connect",
  .player_name_setting = L"player_name",
  .string_settings = { { L"+connect", L"player_name" } }
};

constexpr static std::array<std::string_view, 4> verification_strings = { {
  "AN2.dat"sv,
  "Local\\AN2Revelation"sv,
  "AN2 Launcher"sv,
  "AquaNox 2 Revelation"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}