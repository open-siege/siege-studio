#include <siege/platform/win/desktop/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{};

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "WING COMMANDER ACADEMY"sv,
  "academy.tim"sv,
  "wca.cfg"sv,
  "joy.cfg"sv,
  "gamedat"sv,
  "objects.vga"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}