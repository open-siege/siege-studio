#ifndef DARKSTARDTSCONVERTER_BITMAP_HPP
#define DARKSTARDTSCONVERTER_BITMAP_HPP

#include <vector>
#include <array>
#include <fstream>
#include <optional>
#include <siege/content/pal/palette.hpp>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/platform/drawing.hpp>

namespace siege::content::bmp
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
    std::vector<pal::colour> colours;
    std::vector<std::int32_t> indexes;
  };

  struct gdi_bitmap
  {
    std::int32_t type;
    std::int32_t width;
    std::int32_t height;
    std::int32_t stride;
    std::int16_t planes;
    std::int16_t bit_depth;
    std::byte* bytes; 
  };

  static_assert(std::is_trivially_copyable_v<gdi_bitmap>);

  struct platform_bitmap
  {
      explicit platform_bitmap(windows_bmp_data);
      explicit platform_bitmap(gdi_bitmap);
      
#if WIN32
      HBITMAP handle;

      operator HBITMAP() const
      {
        return handle;
      }
#endif

      ~platform_bitmap();
  };


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

  bool is_earthsiege_bmp(std::istream& raw_data);

  bool is_earthsiege_cursor(std::istream& raw_data);

  bool is_earthsiege_bmp_array(std::istream& raw_data);

  dbm_data read_earthsiege_bmp(std::istream& raw_data);

  dbm_data read_earthsiege_cursor(std::istream& raw_data);

  std::vector<dbm_data> read_earthsiege_bmp_array(std::istream& raw_data);

  bool is_microsoft_bmp(std::istream& raw_data);

  windows_bmp_data get_bmp_data(std::istream& raw_data, bool auto_flip = true);

  void write_bmp_data(std::ostream& raw_data, std::vector<pal::colour> colours, std::vector<std::byte> pixels, std::int32_t width, std::int32_t height, std::int32_t bit_depth, bool auto_flip = true);

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
