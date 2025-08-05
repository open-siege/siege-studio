#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{};

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "JEDIPATH"sv,
  "IMUSE"sv,
  "DARK.GOB"sv,
  "jedi.cfg"sv,
  "jedi.lvl"sv,
  "jedisfx.lfd"sv,
  "SECTOR"sv,
  "OBJECT_MASK"sv,
  "BAD DEFAULT.3DO"sv,
  "default.fme"sv,
  "default.wax"sv,
} };


std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}