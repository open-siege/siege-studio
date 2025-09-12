#ifndef SIEGE_PLATFORM_PALETTE_HPP
#define SIEGE_PLATFORM_PALETTE_HPP

#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/platform/shared.hpp>

namespace siege::platform::palette
{
  namespace endian = siege::platform;

  struct colour
  {
    KEYS_CONSTEXPR static auto keys = platform::make_keys({ "r", "g", "b", "a" });

    std::byte red;
    std::byte green;
    std::byte blue;
    std::byte flags;
  };

  struct colour_rgb
  {
    KEYS_CONSTEXPR static auto keys = platform::make_keys({ "r", "g", "b" });

    std::byte red;
    std::byte green;
    std::byte blue;
  };

  inline bool operator==(const colour& left, const colour& right)
  {
    return left.red == right.red && left.green == right.green && left.blue == right.blue && left.flags == right.flags;
  }

  inline bool operator<(const colour& left, const colour& right)
  {
    return std::tie(left.red, left.green, left.blue, left.flags) < std::tie(right.red, right.green, right.blue, right.flags);
  }

  // A big thanks to https://stackoverflow.com/questions/5392061/algorithm-to-check-similarity-of-colors and
  // https://www.compuphase.com/cmetric.htm
  inline double colour_distance(const colour& e1, const colour& e2)
  {
    long rmean = ((long)e1.red + (long)e2.red) / 2;
    long r = (long)e1.red - (long)e2.red;
    long g = (long)e1.green - (long)e2.green;
    long b = (long)e1.blue - (long)e2.blue;
    return std::sqrt((((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8));
  }

  bool is_microsoft_pal(std::istream& raw_data);
  std::vector<colour> get_pal_data(std::istream& raw_data);
  std::int32_t write_pal_data(std::ostream& raw_data, const std::vector<colour>& colours);
}// namespace siege::content::pal

#endif// SIEGE_PLATFORM_PALETTE_HPP
