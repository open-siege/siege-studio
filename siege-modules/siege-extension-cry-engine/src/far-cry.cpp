#include <siege/platform/win/desktop/window_module.hpp>
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

static std::array<std::string_view, 5> verification_strings = {{
  "Bin32\\FarCryConfigurator.exe"sv,
  "/Caller=FarCry",
  "FarCry.pdb", 
  "CryENGINE",
  std::string_view((const char*)u"Crytek", 8 * sizeof(char16_t)),  
}};


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}