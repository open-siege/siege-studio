#ifndef SIEGE_PLATFORM_BITMAP_HPP
#define SIEGE_PLATFORM_BITMAP_HPP

#include <vector>
#include <array>
#include <fstream>
#include <optional>
#include <siege/platform/palette.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::platform::bitmap
{
  namespace endian = siege::platform;

  struct windows_bmp_header
  {
    std::array<std::byte, 2> tag;
    endian::little_uint32_t file_size;
    endian::little_uint16_t reserved1;
    endian::little_uint16_t reserved2;
    endian::little_uint32_t offset;
  };

  struct windows_bmp_info
  {
    endian::little_uint32_t info_size;
    endian::little_int32_t width;
    endian::little_int32_t height;
    endian::little_uint16_t planes;
    endian::little_uint16_t bit_depth;
    endian::little_uint32_t compression;
    endian::little_uint32_t image_size;
    endian::little_int32_t x_pixels_per_metre;
    endian::little_int32_t y_pixels_per_metre;
    endian::little_uint32_t num_colours_used;
    endian::little_uint32_t num_important_colours;
  };

  struct windows_bmp_data
  {
    windows_bmp_header header;
    windows_bmp_info info;
    std::vector<palette::colour> colours;
    std::vector<std::int32_t> indexes;
  };

  template<typename UnitType>
  inline void invert(std::vector<UnitType>& pixels)
  {
    std::reverse(pixels.begin(), pixels.end());
  }

  template<typename UnitType>
  inline void horizontal_flip(std::vector<UnitType>& pixels, int width)
  {
    for (auto r = pixels.begin(); r != pixels.end(); r += width)
    {
      std::reverse(r, r + width);
    }
  }

  template<typename UnitType>
  inline void vertical_flip(std::vector<UnitType>& pixels, int width)
  {
    invert(pixels);
    horizontal_flip(pixels, width);
  }

  bool is_microsoft_bmp(std::istream& raw_data);

  windows_bmp_data get_bmp_data(std::istream& raw_data, bool auto_flip = true);

  struct bitmap_settings
  {
    std::vector<palette::colour> colours;
    std::int32_t width;
    std::int32_t height;
    std::int32_t bit_depth;
    bool auto_flip = true;
  };

  struct bitmap_offset_settings : bitmap_settings
  {
    std::size_t offset;
  };

  void write_bmp_data(std::ostream& raw_data, std::vector<palette::colour> colours, std::vector<std::byte> pixels, std::int32_t width, std::int32_t height, std::int32_t bit_depth, bool auto_flip = true);
}// namespace siege::platform::bitmap

#endif// SIEGE_PLATFORM_BITMAP_HPP
