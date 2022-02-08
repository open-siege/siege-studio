#include "content/fnt/font.hpp"

namespace studio::content::fnt
{
  namespace endian = boost::endian;
  constexpr auto pft_tag = shared::to_tag<4>({ 'P', 'F', 'O', 'N' });

  struct phoenix_font_header
  {
    std::array<endian::little_int32_t, 2> unknown;
    endian::little_int32_t char_count;
    endian::little_int32_t letter_width;
    endian::little_int32_t letter_height;
    std::array<endian::little_int32_t, 3> unknown2;
    endian::little_int16_t unknown3;
    endian::little_int16_t width_scale;
    endian::little_int16_t unknown4;
    endian::little_int16_t height_scale;
    endian::little_int32_t spacing;
  };

  struct phoenix_char_info
  {
    endian::little_uint8_t bitmap_index;
    endian::little_uint8_t x;
    endian::little_uint8_t y;
    endian::little_uint8_t width;
    endian::little_uint8_t height;
    std::array<endian::little_uint8_t, 3> unknown;
  };

  bool is_phoenix_font(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> header{};
    raw_data.read(header.data(), sizeof(header));

    raw_data.seekg(-int(sizeof(header)), std::ios::cur);

    return header == pft_tag;
  }

  bmp::pbmp_data read_phoenix_font(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> file_header{};
    endian::little_uint32_t file_size{};
    endian::little_uint32_t version{};

    raw_data.read(file_header.data(), sizeof(file_header));
    raw_data.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));
    raw_data.read(reinterpret_cast<std::byte*>(&version), sizeof(version));

    if (file_header != pft_tag)
    {
      throw std::invalid_argument("File data is not PFT based. Offset is " + std::to_string(raw_data.tellg()));
    }

    // TODO figure out what to do with all the font data. Will be useful later.
    // Don't have as much time to work on things, so can't go too crazy with UI changes.
    phoenix_font_header header{};

    raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

    endian::little_int32_t size;
    raw_data.read(reinterpret_cast<std::byte*>(&size), sizeof(size));

    endian::little_int32_t offset;
    raw_data.read(reinterpret_cast<std::byte*>(&offset), sizeof(offset));

    raw_data.seekg(offset * sizeof(endian::little_int16_t));

    std::vector<endian::little_int16_t> chars(size);
    raw_data.read(reinterpret_cast<std::byte*>(chars.data()), chars.size() * sizeof(endian::little_int16_t));

    std::vector<phoenix_char_info> info(header.char_count);
    raw_data.read(reinterpret_cast<std::byte*>(info.data()), info.size() * sizeof(phoenix_char_info));

    return bmp::get_pbmp_data(raw_data);
  }
}
