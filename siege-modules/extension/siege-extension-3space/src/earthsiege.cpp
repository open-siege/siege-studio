import std;
import siege.extension.shared;
import siege.platform.shared;

extern "C" {
using namespace std::literals;

constexpr static std::array<std::string_view, 11> verification_strings = { {
  "GAME.DIC"sv,
} };

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

}
