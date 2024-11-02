#include <siege/platform/win/desktop/window_module.hpp>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

// +connect IP:port
// +connect IP:0

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "ui.dic"sv,
  "[FINAL_DRIVE]"sv,
  "[GEAR_RATIOS]"sv,
  "MPLOBBY_MINI_CHAT"sv,
  "Superleague"sv,
  "RFACTOR_LOGO.TGA"sv,
  "rFactor"sv,
  "PITANIMS.MAS"sv,
  "MONITORPAGE"sv,
  "LOADINGSCREENPAGE"sv,
  "Global\\Superleague_APP_MUTEX_"sv 
}};


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}
#include "dll-main.hpp"

}