// MDL - Quake 1 model data
// MD2 - Quake 2 model data
// MDX - Kingpin model data
// DKM - Daikatana model data

#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::mdl
{
  namespace endian = siege::platform;

  constexpr auto mdl_tag = platform::to_tag<4>({ 'I', 'D', 'P', 'O' });
  constexpr auto md2_tag = platform::to_tag<4>({ 'I', 'D', 'P', '2' });
  constexpr auto mdx_tag = platform::to_tag<4>({ 'I', 'D', 'P', 'X' });
  constexpr auto dkm_tag = platform::to_tag<4>({ 'I', 'D', 'P', 'X' });

  struct mdl_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t version;

  };

  struct md2_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t version;
    std::array<endian::little_uint32_t, 9> size_data;
    endian::little_uint32_t texture_filename_offset;
    std::array<endian::little_uint32_t, 4> offset_data;
    endian::little_uint32_t end_of_file_offset;
  };

  struct mdx_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t version;
    std::array<endian::little_uint32_t, 11> size_data;
    endian::little_uint32_t texture_filename_offset;
    std::array<endian::little_uint32_t, 8> offset_data;
    endian::little_uint32_t end_of_file_offset;
  };

  struct dkm_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t version;
    std::array<endian::little_uint32_t, 11> size_data;
    endian::little_uint32_t texture_filename_offset;
    std::array<endian::little_uint32_t, 7> offset_data;
    endian::little_uint32_t end_of_file_offset;
  };

  bool is_mdl(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag;
    stream.read((char*)&tag, sizeof(tag));
    return tag == mdl_tag;
  }

  void load_mdl(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag;
    stream.read((char*)&tag, sizeof(tag));

  }

  bool is_md2(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag;
    stream.read((char*)&tag, sizeof(tag));
    return tag == md2_tag;
  }

  bool is_mdx(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag;
    stream.read((char*)&tag, sizeof(tag));
    return tag == md2_tag;
  }

}// namespace siege::content::mdl
// TODO complete MDL + MD2 parsers