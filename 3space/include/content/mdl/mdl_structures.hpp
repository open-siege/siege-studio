#ifndef MDL_STRUCTURES_HPP
#define MDL_STRUCTURES_HPP

#include <variant>
#include <array>
#include <string>
#include <vector>
#include "endian_arithmetic.hpp"

namespace idtech2::mdl
{
  namespace endian = studio::endian;
  using file_tag = std::array<std::byte, 4>;

  template<std::size_t Size>
  constexpr std::array<std::string_view, Size> make_keys(const char*(&&keys)[Size])
  {
    std::array<std::string_view, Size> result;
    for (auto i = 0; i < Size; i++)
    {
      result[i] = keys[i];
    }
    return result;
  }

  constexpr file_tag to_tag(const std::array<std::uint8_t, 4> values)
  {
    file_tag result{};

    for (auto i = 0u; i < values.size(); i++)
    {
      result[i] = std::byte{ values[i] };
    }
    return result;
  }

  constexpr file_tag mdl_tag = to_tag({ 'I', 'D', 'P', 'O' });

  struct vector3f
  {
    KEYS_CONSTEXPR static auto keys = make_keys({ "x", "y", "z" });
    float x;
    float y;
    float z;
  };

  static_assert(sizeof(vector3f) == sizeof(std::array<float, 3>));

  struct vertex
  {
    KEYS_CONSTEXPR static auto keys = make_keys({ "x", "y", "z", "normal" });
    std::uint8_t x;
    std::uint8_t y;
    std::uint8_t z;
    std::uint8_t normal;
  };

  struct texture_vertex
  {
    KEYS_CONSTEXPR static auto keys = make_keys({ "onSeam", "x", "y" });
    std::uint8_t on_seam;
    std::uint8_t x;
    std::uint8_t y;
  };

  struct face
  {
    KEYS_CONSTEXPR static auto keys = make_keys({ "facesFront", "vertexIndices" });
    endian::little_int32_t faces_front;
    std::array<endian::little_int32_t, 3> vertex_indices;
  };

  struct simple_frame
  {
    constexpr static auto frame_type = 0;
    vertex bounding_box_min;
    vertex bounding_box_max;
    std::array<char, 16> name;
  };

  using frame_type = endian::little_int32_t;

  struct group_frame
  {
    vertex min_position;
    vertex max_position;
  };

  struct skin
  {
    endian::little_int32_t group;
  };

  static_assert(sizeof(skin) == sizeof(endian::little_int32_t));

  struct file_header
  {
    file_tag file_tag;
    endian::little_int32_t version;
  };

  struct data_header
  {
    vector3f scale;
    vector3f translation;
    float bounding_radius;
    vector3f eye_position;

    endian::little_int32_t num_skins;
    endian::little_int32_t skin_width;
    endian::little_int32_t skin_height;

    endian::little_int32_t num_vertices;
    endian::little_int32_t num_triangles;
    endian::little_int32_t num_frames;

    endian::little_int32_t sync_type;
    endian::little_int32_t flags;
    float size;
  };
}// namespace idtech2::mdl

#endif//DARKSTARDTSCONVERTER_STRUCTURES_HPP
