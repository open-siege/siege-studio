#include <siege/platform/win/desktop/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

constexpr static std::array<std::string_view, 5> verification_strings = { {
  "RedBaronClass"sv,
  "baron.exe"sv,
  "baron.dml"sv,
  "Debrief.dat"sv,
  "SQUADRON.dat"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}