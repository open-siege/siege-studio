#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{};

constexpr static std::array<std::string_view, 6> verification_strings = { {
  "X-Wing Alliance"sv,
  "use_3d_hardware1"sv,
  "use_3d_hardware2"sv,
  "lastpilot"sv,
  "Last Pilot"sv,
  "Install Path"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}