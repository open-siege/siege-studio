// TMD - PlayStation standard model format. Embedded inside of BND files from Colony Wars

#include <array>
#include <variant>
#include <any>
#include <siege/content/dts/tmd.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::tmd
{
  namespace endian = siege::platform;

  constexpr static std::uint8_t magic_number = 65;

  struct tmd_header
  {
    endian::little_uint32_t magic_number;
    endian::little_uint32_t padding;
    endian::little_uint32_t object_count;
  };

  struct tmd_object_header
  {
    endian::little_uint32_t vertex_offset;
    endian::little_uint32_t vertex_count;
    endian::little_uint32_t normal_offset;
    endian::little_uint32_t normal_count;
    endian::little_uint32_t primitive_offset;
    endian::little_uint32_t primitive_count;
    endian::little_uint32_t padding;
  };

  struct tmd_primitive_header
  {
    std::byte padding;
    std::uint8_t size;
    std::byte padding2;
    enum primitive_type : std::uint8_t
    {
      single_colour_triangle = 0x20,
      single_colour_triangle_alt = 0x22,
      gouraud_triangle = 0x24,
      single_colour_quad = 0x28,
      flat_triangle = 0x30,
      textured_triangle = 0x34,
      flat_quad = 0x38,
    } type;
  };

  struct tmd_single_colour_quad_primitive
  {
    tmd_primitive_header header;
    endian::little_uint32_t first_value;
    endian::little_uint32_t second_value;
    endian::little_uint32_t third_value;
    endian::little_uint32_t fourth_value;
  };

  struct tmd_textured_triangle_primitive
  {
    tmd_primitive_header header;
    struct
    {
      endian::little_uint16_t padding;
      std::uint8_t v_coord;
      std::uint8_t u_coord;
    } texture_coordinates[3];

    struct
    {
      endian::little_uint16_t padding;
      endian::little_uint16_t v_coord;
      endian::little_uint16_t u_coord;
    } vertex_indices[3];
  };

  using tmd_primitive = std::variant<tmd_single_colour_quad_primitive>;

  struct tmd_shape
  {
    std::vector<tmd_vertex> vertices;
    std::vector<tmd_vertex> normals;
    std::vector<tmd_primitive> primitives;
  };

  bool is_tmd(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    tmd_header main_header{};
    stream.read((char*)&main_header, sizeof(main_header));

    if (main_header.magic_number == magic_number && main_header.object_count > 0)
    {
      tmd_object_header object_header{};
      stream.read((char*)&object_header, sizeof(object_header));

      auto computed_normal_offset = object_header.vertex_offset + (sizeof(tmd_vertex) * object_header.vertex_count);
      auto computed_primitive_offset = computed_normal_offset + (sizeof(tmd_vertex) * object_header.normal_count);

      return object_header.normal_offset == computed_normal_offset && object_header.primitive_offset == computed_primitive_offset;
    }

    return false;
  }

  std::vector<std::any> load_tmd(std::istream& stream)
  {
    std::vector<std::any> results;
    platform::istream_pos_resetter resetter(stream);

    tmd_header main_header{};
    stream.read((char*)&main_header, sizeof(main_header));

    if (main_header.magic_number == magic_number && main_header.object_count > 0)
    {
      auto start_offset = (std::size_t)stream.tellg();
      
      results.reserve(main_header.object_count);

      for (auto i = 0u; i < main_header.object_count; ++i)
      {
        tmd_object_header object_header{};
        stream.read((char*)&object_header, sizeof(object_header));

        tmd_shape shape;

        shape.vertices.resize(object_header.vertex_count);
        shape.normals.resize(object_header.normal_count);
        shape.primitives.reserve(object_header.primitive_count);

        stream.seekg(start_offset + object_header.vertex_offset, std::ios::beg);
        stream.read((char*)shape.vertices.data(), sizeof(tmd_vertex) * shape.vertices.size());

        stream.seekg(start_offset + object_header.normal_offset, std::ios::beg);
        stream.read((char*)shape.normals.data(), sizeof(tmd_vertex) * shape.normals.size());

        stream.seekg(start_offset + object_header.primitive_offset, std::ios::beg);

        for (auto i = 0u; i < object_header.primitive_count; ++i)
        {
          tmd_primitive_header header{};

          stream.read((char*)&header, sizeof(header));

          if (header.type != tmd_primitive_header::single_colour_quad)
          {
            break;
          }

          tmd_single_colour_quad_primitive quad;
          quad.header = header;

          auto quad_size = sizeof(quad) - sizeof(header);

          if (header.size * 4 > quad_size)
          {
            stream.read((char*)&quad.first_value, quad_size);
            stream.seekg(header.size * 4 - quad_size, std::ios::cur);
          }
          else
          {
            stream.read((char*)&quad.first_value, header.size * 4);
          }

          shape.primitives.emplace_back(std::move(quad));
        }

        results.emplace_back(std::move(shape));
      }
    }

    return results;
  }
}