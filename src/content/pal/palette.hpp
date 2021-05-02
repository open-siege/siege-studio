#ifndef DARKSTARDTSCONVERTER_PALETTE_HPP
#define DARKSTARDTSCONVERTER_PALETTE_HPP

#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "endian_arithmetic.hpp"
#include "shared.hpp"

namespace studio::content::pal
{
  namespace endian = boost::endian;
  using file_tag = std::array<std::byte, 4>;

  struct colour
  {
    KEYS_CONSTEXPR static auto keys = shared::make_keys({ "r", "g", "b", "a" });

    std::byte red;
    std::byte green;
    std::byte blue;
    std::byte flags;
  };

  inline bool operator==(const colour& left, const colour& right)
  {
    return left.red == right.red &&
    left.green == right.green &&
    left.blue == right.blue &&
    left.flags == right.flags;
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
    return sqrt((((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8));
  }

  struct palette_header
  {
    endian::big_int16_t version;
    endian::little_int16_t colour_count;
  };

  struct palette_info
  {
    endian::little_int32_t palette_count;
    endian::little_int32_t shade_shift;
    endian::little_int32_t haze_level;
    colour haze_colour;
    std::array<std::byte, 32> allowed_matches;
  };

  struct palette
  {
    std::vector<colour> colours;
    endian::little_uint32_t index;
    endian::little_uint32_t type;
  };

  struct fixed_palette
  {
    std::array<colour, 256> colours;
    endian::little_uint32_t index;
    endian::little_uint32_t type;
  };

  bool is_microsoft_pal(std::basic_istream<std::byte>& raw_data);
  std::vector<colour> get_pal_data(std::basic_istream<std::byte>& raw_data);
  std::int32_t write_pal_data(std::basic_ostream<std::byte>& raw_data, const std::vector<colour>& colours);

  bool is_phoenix_pal(std::basic_istream<std::byte>& raw_data);

  std::vector<palette> get_ppl_data(std::basic_istream<std::byte>& raw_data);
}// namespace darkstar::pal

#endif//DARKSTARDTSCONVERTER_PALETTE_HPP
