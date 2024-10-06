#ifndef DARKSTARDTSCONVERTER_PALETTE_HPP
#define DARKSTARDTSCONVERTER_PALETTE_HPP

#include <vector>
#include <array>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>

#include <siege/platform/palette.hpp>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/platform/shared.hpp>

namespace siege::content::pal
{
  using colour = siege::platform::palette::colour;
  namespace endian = siege::platform;
  using file_tag = std::array<std::byte, 4>;

  enum class palette_type
  {
    unknown,
    cga,
    ega,
    vga
  };

  struct old_palette
  {
    palette_type type;
    std::vector<colour> colours;
  };

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

  struct palette
  {
    std::vector<colour> colours;
    endian::little_uint32_t index;
    endian::little_uint32_t type;
  };

  bool is_old_pal(std::istream& raw_data);
  std::vector<old_palette> get_old_pal_data(std::istream& raw_data);

  bool is_phoenix_pal(std::istream& raw_data);
  std::vector<palette> get_ppl_data(std::istream& raw_data);

  bool is_earthsiege_pal(std::istream& raw_data);
  std::vector<colour> get_earthsiege_pal(std::istream& raw_data);
}// namespace siege::content::pal

#endif// DARKSTARDTSCONVERTER_PALETTE_HPP
