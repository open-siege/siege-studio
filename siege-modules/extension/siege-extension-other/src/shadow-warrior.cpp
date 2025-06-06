#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{};

constexpr static std::array<std::string_view, 8> verification_strings = { {
  "BUILD engine"sv,
  "SHADOW WARRIOR"sv,
  "SW.CFG"sv,
  "Flash_Bomb"sv,
  "Caltrops"sv,
  "Z_Malloc"sv,
  "ASSVER"sv,
  "SCRIPT_Section"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}