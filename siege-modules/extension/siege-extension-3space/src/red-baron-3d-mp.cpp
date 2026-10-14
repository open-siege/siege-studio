import std;
import siege.extension.shared;
import siege.platform.shared;

extern "C" {
using namespace std::literals;

constexpr static std::array<std::string_view, 5> verification_strings = { {
  "RedBaronClass"sv,
  "Multiplayer"sv,
  "Baronmp.exe"sv,
  "baron.dml"sv,
  "MPString2.dat"sv,
} };

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}
