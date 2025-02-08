#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"+connect",
  .player_name_setting = L"+name",
  .string_settings = { { L"+connect", L"name" } }
};

constexpr static std::array<std::string_view, 13> verification_strings = { {
  "connect"sv,
  "name"sv,
  "team"sv,
  "skin"sv,
  "game"sv,
  "host"sv,
  "maxplayers"sv,
  "hostname"sv,
  "console"sv,
  "lobby"sv,
  "redline.cfg"sv,
  "RED6"sv,
  "Redline"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}