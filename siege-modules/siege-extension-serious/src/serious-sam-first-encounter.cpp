#include <siege/platform/win/desktop/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"+connect",
  .string_settings = { { L"+connect" } }
};

constexpr static std::array<std::string_view, 7> verification_strings = { {
  "SeriousSam"sv,
  "serioussam"sv,
  "Data\\SeriousSam.gms"sv,
  "ETRSERIOUS"sv,
  "ETRSerious"sv,
  "Bin\\EntitiesMP.dll",
  "Bin\\Entities.dll",
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  std::error_code error{};
  if (siege::executable_is_supported(filename, verification_strings) == S_OK)
  {
    auto path = std::filesystem::path(filename);
    auto parent = path.parent_path();
    if (std::filesystem::exists(parent / "Game.dll", error) &&
        std::filesystem::exists(parent / "GameGUI.dll", error) &&
        std::filesystem::exists(parent / "Entities.dll", error))
    {
      return S_OK;
    }
  }

  return S_FALSE;
}

}