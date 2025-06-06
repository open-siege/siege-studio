#include <siege/platform/win/window_module.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{};

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "Null poi"sv,
  "DRAGON"sv,
  "ragon.fnt"sv,
  "MEANWHILE"sv,
  "path2.ttm"sv,
  "OURCE.CFG"sv,
  ".ovl"sv,
  ":INF:"sv,
  "VOLUME.VG"sv,
  "qwerty"sv,
  "blocks"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  /*
   // Rise of the Dragon
  if (exe_path.stem() == "DRAGON" && exe_path.extension() == ".EXE" && 
      fs::exists(parent_path / "VOLUME.VGA", last_error) && 
      fs::exists(parent_path / "VOLUME.001", last_error) && 
      fs::exists(parent_path / "RESOURCE.CFG", last_error))
  {
    return S_OK;
  }*/
  return siege::executable_is_supported(filename, verification_strings);
}

}