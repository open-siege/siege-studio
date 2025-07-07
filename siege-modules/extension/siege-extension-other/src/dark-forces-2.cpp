#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .string_settings = { { L"+connect", L"player_name" } },
  .ip_connect_setting = L"+connect",
  .player_name_setting = L"player_name",
};

constexpr static std::array<std::string_view, 13> verification_strings = { {
  "-path"sv,
  "-devMode"sv,
  "-dispStats"sv,
  "-windowGUI"sv,
  "-displayConfig"sv,
  "-debug"sv,
  "-verbose"sv,
  "-noHUD"sv,
  "+twinkle"sv,
  "+ssparks_saber"sv,
  "+ssparks_blood"sv,
  "+ssparks_wall"sv,
  "GAME_UBERJEDI"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}