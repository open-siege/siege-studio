#ifndef SIEGE_WTB_HPP
#define SIEGE_WTB_HPP

#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::wtb
{
  namespace endian = siege::platform;
  // each tag must represent a type of shading or texturing

  enum polygon_type : std::uint8_t
  {
      tag1 = 0x10,
      tag2 = 0x14,
      tag3 = 0x18,
      tag4 = 0x19,
      tag5 = 0x1c,
      tag6 = 0x47,
      tag7 = 0x44,
      tag8 = 0x49,
      tag9 = 0x4c,
      tag10 = 0x4d,
      tag11 = 0x4e,
      tag12 = 0x54,
      tag13 = 0x58,
      tag14 = 0x64,
      tag15 = 0x68,
      tag16 = 0x70,
      tag17 = 0x74,
      tag18 = 0x78,
      tag19 = 0x7d,
      tag20 = 0x7f,
  };

  struct packed_vertex
  {
    endian::little_int32_t x;
    endian::little_int32_t y;
    endian::little_int32_t z;
    endian::little_uint16_t normal_index;
    endian::little_uint16_t unknown;
  };

  struct surface_header
  {
    std::byte unknown;
    polygon_type type;
    endian::little_uint16_t number_of_points;
  };

  using face_4v = std::array<endian::little_uint16_t, 4>;
  using face_7v = std::array<endian::little_uint16_t, 7>;

  struct surface
  {
    surface_header header;
    std::vector<endian::little_uint16_t> values;
  };

  struct wtb_shape
  {
    float scale;
    float translation;

    std::vector<packed_vertex> vertices;
    std::vector<surface> faces;
  };

  bool is_wtb(std::istream& stream);
  wtb_shape load_wtb(std::istream& stream);
}// namespace siege::content::wtb

#endif// !SIEGE_WTB_HPP
