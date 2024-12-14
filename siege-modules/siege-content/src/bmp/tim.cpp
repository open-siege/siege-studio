#include <siege/platform/tagged_data.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/bitmap.hpp>

namespace siege::content::tim
{
  namespace endian = siege::platform;
  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag four_bit_image = platform::to_tag<4>({ 0x10, 0x00, 0x08, 0x00 });
  constexpr file_tag eight_bit_image = platform::to_tag<4>({ 0x10, 0x00, 0x09, 0x00 });
  constexpr file_tag sixteen_bit_image = platform::to_tag<4>({ 0x10, 0x00, 0x02, 0x00 });

  struct palette_header
  {
    endian::little_uint32_t size;
    endian::little_uint16_t offset_x;
    endian::little_uint16_t offset_y;
    endian::little_uint16_t color_count;
    endian::little_uint16_t palette_count;
  };

  struct image_header
  {
    endian::little_uint32_t size;
    endian::little_uint16_t offset_x;
    endian::little_uint16_t offset_y;
    endian::little_uint16_t width;
    endian::little_uint16_t height;
  };

  struct tim_data
  {
    struct tim_palette
    {
      palette_header header;
      std::vector<endian::little_uint16_t> colours;
    };
    std::vector<tim_palette> palettes;

    image_header header;
    std::vector<endian::little_uint16_t> colours;
  };

  bool is_tim(std::istream& raw_data)
  {
    std::array<std::byte, 4> header{};
    platform::istream_pos_resetter resetter(raw_data);
    platform::read(raw_data, header.data(), sizeof(header));

    return header == four_bit_image || 
        header == eight_bit_image ||
        header == sixteen_bit_image;
  }

  tim_data get_tim_data(std::istream& raw_data)
  {
    tim_data result{};

    return result;
  }

  platform::bitmap::windows_bmp_data get_tim_data_as_bitmap(std::istream& raw_data)
  {
    tim_data temp = get_tim_data(raw_data);
    platform::bitmap::windows_bmp_data result{};

    return result;
  }



}// namespace siege::content::tim