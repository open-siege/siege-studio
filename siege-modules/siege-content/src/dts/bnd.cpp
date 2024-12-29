// BND - Colony Wars 3D shape data from Colony Wars and Colony Wars Vengeance
// Contains embedded TMD shape data and TIM texture data.

#include <array>
#include <any>
#include <siege/content/dts/tmd.hpp>
#include <siege/content/bmp/tim.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::bnd
{
  namespace endian = siege::platform;

  constexpr static auto body_tag = platform::to_tag<4>("BODY");
  constexpr static auto data_tag = platform::to_tag<4>("DATA");
  constexpr static auto tmds_tag = platform::to_tag<4>("TMDS");
  constexpr static auto vram_tag = platform::to_tag<4>("VRAM");
  constexpr static auto tims_tag = platform::to_tag<4>("TIMS");

  struct iff_tag
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t size;
  };

  struct data_section
  {
    iff_tag info;
    endian::little_uint32_t object_count;
  };

  struct tmd_section
  {
    iff_tag info;
    endian::little_uint32_t vertex_offset;
    endian::little_uint32_t vertex_count;
    endian::little_uint32_t normal_offset;
    endian::little_uint32_t normal_count;
    std::array<endian::little_uint32_t, 3> padding;
    std::array<endian::little_uint32_t, 4> primitive_sizes;
    std::array<endian::little_uint32_t, 4> padding2;
    std::array<endian::little_uint32_t, 4> primitive_offsets;
    std::array<endian::little_uint32_t, 4> primitive_end_offsets;
  };

  struct bnd_textured_triangle_primitive
  {
    struct
    {
      endian::little_uint16_t v_coord;
      endian::little_uint16_t u_coord;
    } texture_coordinates[3];

    struct
    {
      endian::little_uint16_t v_index;
      endian::little_uint16_t u_index;
    } vertex_indices[3];
  };

  struct bnd_shape
  {
    std::vector<tmd::tmd_vertex> vertices;
    std::vector<tmd::tmd_vertex> normals;
    std::vector<bnd_textured_triangle_primitive> primitives;
  };

  struct bnd_data
  {
    std::vector<bnd_shape> shapes;
    std::vector<platform::bitmap::windows_bmp_data> textures;
  };

  bool is_bnd(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);

    iff_tag body;
    iff_tag data;

    stream.read((char*)&body, sizeof(body));
    stream.read((char*)&data, sizeof(data));

    return body.tag == body_tag && data.tag == data_tag;
  }

  std::vector<std::any> load_bnd(std::istream& stream)
  {
    std::vector<std::any> results;
    platform::istream_pos_resetter resetter(stream);

    iff_tag body;

    stream.read((char*)&body, sizeof(body));

    if (body.tag == body_tag)
    {
      data_section data;

      stream.read((char*)&data, sizeof(data));

      bnd_data result;

      if (data.info.tag == data_tag)
      {
        auto data_start = (std::size_t)stream.tellg();
        auto tmd_start = data_start + sizeof(iff_tag);

        result.shapes.reserve(data.object_count);

        for (auto i = 0; i < data.object_count; ++i)
        {
          tmd_section tmd_section;
          stream.read((char*)&tmd_section, sizeof(tmd_section));

          if (tmd_section.info.tag != tmds_tag)
          {
            break;
          }

          result.shapes.emplace_back(bnd_shape{});
          auto& shape = result.shapes.back();

          shape.vertices.resize(tmd_section.vertex_count);
          shape.normals.resize(tmd_section.normal_count);

          stream.seekg(tmd_start + tmd_section.vertex_offset, std::ios::beg);
          stream.read((char*)shape.vertices.data(), sizeof(tmd::tmd_vertex) * shape.vertices.size());

          stream.seekg(tmd_start + tmd_section.normal_offset, std::ios::beg);
          stream.read((char*)shape.normals.data(), sizeof(tmd::tmd_vertex) * shape.normals.size());

          auto primitive_count = std::find_if(tmd_section.primitive_sizes.begin(), tmd_section.primitive_sizes.end(), [](auto value) { return value > 0; });

          if (primitive_count != tmd_section.primitive_sizes.end())
          {
            shape.primitives.resize(*primitive_count);
            stream.seekg(tmd_start + tmd_section.primitive_offsets[0], std::ios::beg);
            stream.read((char*)shape.primitives.data(), sizeof(bnd_textured_triangle_primitive) * shape.primitives.size());
          }

          result.shapes.emplace_back(std::move(shape));
        }
      }

      results.emplace_back(std::move(result));
    }

    return results;
  }
}// namespace siege::content::bnd