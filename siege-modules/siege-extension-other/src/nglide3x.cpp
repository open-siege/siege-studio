#include <siege/platform/win/desktop/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { L"NGLIDE_BACKEND", L"NGLIDE_RESOLUTION", L"NGLIDE_ASPECT", L"NGLIDE_REFRESH", L"NGLIDE_VSYNC", L"NGLIDE_GAMMA", L"NGLIDE_SPLASH" } },
};

constexpr static std::array<std::string_view, 8> verification_strings = { { 
  "Software\\Zeus Software\\nGlide"sv,
  "glide3x.dll"sv,
  "grDepthBufferFunction"sv,
  "grFogMode"sv,
  "grDrawPoint"sv,
  "grDrawLine"sv,
  "grDrawTriangle"sv,
  "grDrawVertexArray"sv } };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}
}