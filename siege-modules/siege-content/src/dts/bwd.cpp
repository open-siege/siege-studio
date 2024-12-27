#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::bwd
{
  namespace endian = siege::platform;

  constexpr auto header_tag = platform::to_tag<4>({ 'B', 'W', 'D', '\0' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'R', 'E', 'V', '\0' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'D', 'T', 'B', 'L' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'M', 'O', 'N', '\0' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'O', 'B', 'J', '\0' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'R', 'E', 'P', 'R' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'E', 'N', 'D', 'R' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'T', 'H', 'N', 'G' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'O', 'B', 'J', 'L' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'A', 'N', 'I', 'M' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'T', 'S', 'K', '\0' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'P', 'O', 'F', 'O' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'E', 'Y', 'E', 'O' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'V', 'P', 'T', 'F' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'C', 'P', 'T', 'F' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'H', 'U', 'D', 'F' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'P', 'I', 'T', 'F' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'M', 'O', 'F', 'F' });
  //constexpr auto header_tag = platform::to_tag<4>({ 'A', 'S', 'N', 'D' });

  bool is_bwd(std::istream& stream)
  {
    return false;
  }
}// namespace siege::content::bwd
