#ifndef DARKSTARDTSCONVERTER_BITMAP_HPP
#define DARKSTARDTSCONVERTER_BITMAP_HPP

#include <vector>
#include <array>
#include <fstream>
#include <optional>
#include <siege/content/pal/palette.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::bmp
{
  namespace endian = siege::platform;

  struct pbmp_header
  {
    endian::little_uint32_t version;
    endian::little_int32_t width;
    endian::little_int32_t height;
    endian::little_uint32_t bit_depth;
    endian::little_uint32_t flags;
  };

  struct pbmp_data
  {
    pbmp_header bmp_header;
    endian::little_uint32_t detail_levels;
    endian::little_uint32_t palette_index;
    std::vector<std::byte> pixels;
  };

  struct dba_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t file_size;
    endian::little_uint32_t count;
  };

  struct dbm_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t file_size;
    endian::little_uint16_t height;
    endian::little_uint16_t width;
    std::byte bit_depth;
    std::byte unknown0;
    std::byte unknown1;
    endian::little_uint32_t image_size;
    endian::little_uint16_t extra_data_size;
  };

  struct dbm_data
  {
    dbm_header header;
    std::vector<std::byte> pixels;
  };

  bool is_earthsiege_bmp(std::istream& raw_data);

  bool is_earthsiege_cursor(std::istream& raw_data);

  bool is_earthsiege_bmp_array(std::istream& raw_data);

  dbm_data read_earthsiege_bmp(std::istream& raw_data);

  dbm_data read_earthsiege_cursor(std::istream& raw_data);

  std::vector<dbm_data> read_earthsiege_bmp_array(std::istream& raw_data);

  bool is_phoenix_bmp(std::istream& raw_data);

  pbmp_data get_pbmp_data(std::istream& raw_data);

  void write_pbmp_data(std::ofstream& raw_data, std::int32_t width, std::int32_t height, const std::vector<pal::colour>& colours, const std::vector<std::byte>& pixels, std::optional<std::uint32_t> palette_id = std::nullopt);

  bool is_phoenix_bmp_array(std::istream& raw_data);

  std::vector<pbmp_data> get_pba_data(std::istream& raw_data);

  std::vector<std::byte> remap_bitmap(const std::vector<std::byte>& pixels,
                                         const std::vector<pal::colour>& original_colours,
                                         const std::vector<pal::colour>& other_colours,
                                         bool only_unique = false);

  std::vector<std::int32_t> remap_bitmap(const std::vector<std::int32_t>& pixels,
    const std::vector<pal::colour>& original_colours,
    const std::vector<pal::colour>& other_colours,
    bool only_unique = false);
}// namespace siege::content::bmp

#endif//DARKSTARDTSCONVERTER_BITMAP_HPP
