// BND - Colony Wars 3D shape data from Colony Wars and Colony Wars Vengeance
// Contains embedded TMD shape data and TIM texture data.

#include <array>
#include <any>
#include <spanstream>
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
  constexpr static auto coll_tag = platform::to_tag<4>("COLL");
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
    endian::little_uint16_t object_count;
    endian::little_uint16_t padding;
  };

  struct tmd_section
  {
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

  struct bnd_collision
  {
    std::vector<char> bytes;
    bnd_shape shape;
  };

  struct bnd_data
  {
    std::vector<bnd_shape> shapes;
    std::vector<bnd_collision> collision_data;
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
      bnd_data result;
      std::vector<char> buffer;

      for (auto i = 0u; i < 2; ++i)
      {
        data_section data;

        stream.read((char*)&data, sizeof(data));

        if (data.info.tag == data_tag)
        {
          result.shapes.reserve(data.object_count);

          std::size_t object_count = data.object_count;

          for (auto i = 0u; i < object_count; ++i)
          {
            iff_tag next_item;
            stream.read((char*)&next_item, sizeof(next_item));

            if (!(next_item.tag == tmds_tag || next_item.tag == coll_tag))
            {
              break;
            }

            buffer.resize(next_item.size);
            stream.read(buffer.data(), buffer.size());

            auto read_tmd = [](std::istream& tmd_stream, bnd_shape& shape) {
              tmd_section tmd_section;
              tmd_stream.read((char*)&tmd_section, sizeof(tmd_section));

              shape.vertices.resize(tmd_section.vertex_count);
              shape.normals.resize(tmd_section.normal_count);

              tmd_stream.seekg(tmd_section.vertex_offset, std::ios::beg);
              tmd_stream.read((char*)shape.vertices.data(), sizeof(tmd::tmd_vertex) * shape.vertices.size());

              tmd_stream.seekg(tmd_section.normal_offset, std::ios::beg);
              tmd_stream.read((char*)shape.normals.data(), sizeof(tmd::tmd_vertex) * shape.normals.size());

              auto primitive_count = std::find_if(tmd_section.primitive_sizes.begin(), tmd_section.primitive_sizes.end(), [](auto value) { return value > 0; });

              if (primitive_count != tmd_section.primitive_sizes.end())
              {
                shape.primitives.resize(*primitive_count);
                tmd_stream.seekg(tmd_section.primitive_offsets[0], std::ios::beg);
                tmd_stream.read((char*)shape.primitives.data(), sizeof(bnd_textured_triangle_primitive) * shape.primitives.size());
              }
            };

            if (next_item.tag == tmds_tag)
            {
              std::ispanstream tmd_stream{ std::span<char>(buffer) };

              auto& shape = result.shapes.emplace_back();
              read_tmd(tmd_stream, shape);
            }
            else if (next_item.tag == coll_tag)
            {
              buffer.resize(next_item.size);
              stream.read(buffer.data(), buffer.size());
              auto& collision = result.collision_data.emplace_back();

              collision.bytes = std::move(buffer);
              buffer = std::vector<char>();

              stream.read((char*)&next_item, sizeof(next_item));

              if (next_item.tag != tmds_tag)
              {
                stream.seekg(-sizeof(next_item), std::ios::cur);
                continue;
              }
              buffer.resize(next_item.size);
              stream.read(buffer.data(), buffer.size());

              std::ispanstream tmd_stream{ std::span<char>(buffer) };
              read_tmd(tmd_stream, collision.shape);
            }

            auto current_pos = stream.tellg();
            stream.read((char*)&next_item, sizeof(next_item));

            if (next_item.tag == coll_tag)
            {
              object_count++;
            }
            stream.seekg(current_pos, std::ios::beg);
          }
        }
        else if (data.info.tag == vram_tag)
        {
          stream.seekg(data.info.size - sizeof(std::uint32_t), std::ios::cur);

          for (auto i = 0u; i < data.object_count; ++i)
          {
            iff_tag next_item;
            stream.read((char*)&next_item, sizeof(next_item));
            
            if (next_item.tag != tims_tag)
            {
              break;
            }
            
            buffer.resize(next_item.size);
            stream.read(buffer.data(), buffer.size());
            std::ispanstream tim_stream{ std::span<char>(buffer) };
            result.textures.emplace_back(tim::get_tim_data_as_bitmap(tim_stream));
          }
        }
      }

      results.emplace_back(std::move(result));
    }

    return results;
  }
}// namespace siege::content::bnd