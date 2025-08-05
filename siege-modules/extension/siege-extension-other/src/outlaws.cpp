#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .string_settings = { { L"-join", L"player_name" } },
  .ip_connect_setting = L"-join",
  .player_name_setting = L"player_name",
};

constexpr static std::array<std::string_view, 23> verification_strings = { {
  "memsafety"sv,
  "level"sv,
  "video"sv,
  "film"sv,
  "user"sv,
  "nosound"sv,
  "disext"sv,
  "diff"sv,
  "volume"sv,
  "team"sv,
  "soundlist"sv,
  "safe"sv,
  "tokenize"sv,
  "track"sv,
  "batch"sv,
  "host"sv,
  "join"sv,
  "nocheck"sv,
  "netdriver"sv,
  "lobby"sv,
  "Jedi"sv,
  "outlaws"sv,
  "OUTLAWS"sv,
} };


std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}