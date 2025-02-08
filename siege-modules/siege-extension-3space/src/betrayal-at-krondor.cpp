#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

constexpr static std::array<std::string_view, 7> verification_strings = { {
  "KRONDOR.CFG"sv,
  "krondor.rmf"sv,
  "krondor.exe"sv,
  "BMP:INF:"sv,
  "BMP:BIN:"sv,
  "BMP:VGA:"sv,
  "BMP:AMG:"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}