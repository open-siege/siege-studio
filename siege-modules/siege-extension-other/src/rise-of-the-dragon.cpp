#include <siege/platform/win/desktop/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{};

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "Null poi"sv,
  "DRAGON"sv,
  "ragon.fnt"sv,
  "MEANWHILE"sv,
  "path2.ttm"sv,
  "OURCE.CFG"sv,
  ".ovl"sv,
  ":INF:"sv,
  "VOLUME.VG"sv,
  "qwerty"sv,
  "blocks"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}