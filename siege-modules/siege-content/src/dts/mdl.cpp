// MDL - Quake 1 model data
// MD2 - Quake 2 model data
// MDX - Kingpin model data
// DKM - Daikatana model data

#include <array>
#include <any>
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
    std::array<float, 3> scale;
    std::array<float, 3> translation;
    float other;
    std::array<endian::little_uint32_t, 3> padding;
    endian::little_uint32_t texture_count;
    endian::little_uint32_t texture_width;
    endian::little_uint32_t texture_height;
    endian::little_uint32_t vertex_and_uv_count;
    endian::little_uint32_t face_count;
    endian::little_uint32_t frame_count;
    endian::little_uint32_t padding2;
    endian::little_uint32_t padding3;
    float other2;
  };

  struct mdl_uv_coordinate
  {
    endian::little_uint32_t on_seam;
    endian::little_uint32_t u;
    endian::little_uint32_t v;
  };

  struct mdl_face
  {
    endian::little_uint32_t is_front_face;
    std::array<endian::little_uint32_t, 3> vertex_indices;
  };

  struct mdl_vertex
  {
    std::uint8_t x;
    std::uint8_t y;
    std::uint8_t z;
    std::uint8_t normal;
  };

  struct mdl_frame
  {
    mdl_vertex bounding_min;
    mdl_vertex bounding_max;
    std::array<char, 16> name;
    std::vector<mdl_vertex> vertices;
  };

  struct animated_texture
  {
    float frame_duration;
    std::vector<std::vector<std::byte>> frame_data;
  };

  using texture_data = std::variant<std::vector<std::byte>, animated_texture>;

  struct mdl_shape
  {
    endian::little_uint32_t texture_width;
    endian::little_uint32_t texture_height;
    std::vector<texture_data> textures;
    std::vector<mdl_uv_coordinate> uv_coordinates;
    std::vector<mdl_face> faces;
    std::vector<mdl_frame> frames;
  };


  bool is_mdl(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag;
    stream.read((char*)&tag, sizeof(tag));
    return tag == mdl_tag;
  }

  std::any load_mdl(std::istream& stream)
  {
    mdl_shape shape;
    platform::istream_pos_resetter resetter(stream);
    mdl_header header;
    stream.read((char*)&header, sizeof(header));

    if (header.tag != mdl_tag && header.version != 6)
    {
      return std::any{};
    }

    shape.textures.reserve(header.texture_count);
    shape.frames.reserve(header.frame_count);
    shape.uv_coordinates.resize(header.vertex_and_uv_count);
    shape.faces.resize(header.face_count);

    for (auto i = 0u; i < header.texture_count; ++i)
    {
      endian::little_uint32_t type{};
      stream.read((char*)&type, sizeof(type));

      if (type == 0)
      {
        std::vector<std::byte> image_data(header.texture_width * (std::uint32_t)header.texture_height, std::byte{});
        stream.read((char*)image_data.data(), image_data.size());
        shape.textures.emplace_back(std::move(image_data));
      }
      else
      {
        endian::little_uint32_t count{};
        stream.read((char*)&count, sizeof(count));
        float duration{};
        stream.read((char*)&duration, sizeof(duration));

        animated_texture temp;
        temp.frame_duration = duration;
        temp.frame_data.reserve(count);

        for (auto i = 0; i < count; ++i)
        {
          auto& data = temp.frame_data.emplace_back();
          data.resize(header.texture_width * (std::uint32_t)header.texture_height);
          stream.read((char*)data.data(), data.size());
        }

        shape.textures.emplace_back(std::move(temp));
      }
    }

    stream.read((char*)shape.uv_coordinates.data(), shape.uv_coordinates.size() * sizeof(mdl_uv_coordinate));
    stream.read((char*)shape.faces.data(), shape.faces.size() * sizeof(mdl_face));

    for (auto i = 0u; i < header.frame_count; ++i)
    {
      endian::little_uint32_t type{};
      stream.read((char*)&type, sizeof(type));

      if (type == 0)
      {
        auto& frame = shape.frames.emplace_back();

        stream.read((char*)&frame, sizeof(frame) - sizeof(frame.vertices));

        frame.vertices.resize(header.vertex_and_uv_count);
        stream.read((char*)frame.vertices.data(), frame.vertices.size() * sizeof(mdl_vertex));
      }
      else
      {
        DebugBreak();
      }
    }

    return shape;
  }

  struct md2_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t version;
    endian::little_uint32_t texture_width;
    endian::little_uint32_t texture_height;
    endian::little_uint32_t frame_byte_count;
    endian::little_uint32_t texture_count;
    endian::little_uint32_t vertex_per_frame_count;
    endian::little_uint32_t uv_count;
    endian::little_uint32_t face_count;
    endian::little_uint32_t unknown_count;
    endian::little_uint32_t frame_count;
    endian::little_uint32_t texture_offset;
    endian::little_uint32_t uv_offset;
    endian::little_uint32_t face_offset;
    endian::little_uint32_t frame_offset;
    endian::little_uint32_t unknown_offset;
    endian::little_uint32_t eof_offset;
  };

  struct md2_uv_coordinate
  {
    endian::little_uint16_t u;
    endian::little_uint16_t v;
  };

  struct md2_face
  {
    std::array<endian::little_uint16_t, 3> vertex_indices;
    std::array<endian::little_uint16_t, 3> uv_indices;
  };

  struct md2_frame
  {
    std::array<float, 3> scale;
    std::array<float, 3> translation;
    std::array<char, 16> name;
    std::vector<mdl_vertex> vertices;
  };

  struct md2_shape
  {
    endian::little_uint32_t texture_width;
    endian::little_uint32_t texture_height;
    std::vector<std::string> texture_filenames;
    std::vector<md2_face> faces;
    std::vector<md2_uv_coordinate> uv_coordinates;
    std::vector<md2_frame> frames;
  };


  bool is_md2(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag;
    stream.read((char*)&tag, sizeof(tag));
    return tag == md2_tag;
  }

  std::any load_md2(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);

    auto start = (std::size_t)resetter.position;

    md2_shape shape;

    md2_header header;
    stream.read((char*)&header, sizeof(header));

    if (header.tag != md2_tag && header.version != 8)
    {
      return std::any{};
    }

    shape.texture_width = header.texture_width;
    shape.texture_height = header.texture_height;

    auto frame_size = header.frame_byte_count;
    auto extra_data_size = frame_size + sizeof(std::vector<mdl_vertex>) - sizeof(md2_frame);

    if (header.vertex_per_frame_count * sizeof(mdl_vertex) != extra_data_size)
    {
      return std::any{};
    }

    shape.texture_filenames.reserve(header.texture_count);
    stream.seekg(start + header.texture_offset, std::ios::beg);
    std::array<char, 64> temp;
    for (auto i = 0u; i < header.texture_count; ++i)
    {
      stream.read(temp.data(), temp.size());
      temp[63] = 0;
      shape.texture_filenames.emplace_back(temp.data());
    }

    shape.uv_coordinates.resize(header.uv_count);
    stream.seekg(start + header.uv_offset, std::ios::beg);
    stream.read((char*)shape.uv_coordinates.data(), shape.uv_coordinates.size() * sizeof(md2_uv_coordinate));

    shape.faces.resize(header.face_count);
    stream.seekg(start + header.face_offset, std::ios::beg);
    stream.read((char*)shape.faces.data(), shape.faces.size() * sizeof(md2_face));

    shape.frames.reserve(header.frame_count);

    stream.seekg(start + header.frame_offset, std::ios::beg);

    for (auto i = 0u; i < header.frame_count; ++i)
    {
      auto& frame = shape.frames.emplace_back();
      stream.read((char*)&frame, sizeof(frame) - sizeof(std::vector<mdl_vertex>));

      frame.vertices.reserve(header.vertex_per_frame_count);
      for (auto v = 0u; v < header.vertex_per_frame_count; ++v)
      {
        auto& vertex = frame.vertices.emplace_back();
        stream.read((char*)&vertex, sizeof(vertex));
      }
    }

    return shape;
  }

  struct mdx_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t version;
    endian::little_uint32_t texture_width;
    endian::little_uint32_t texture_height;
    endian::little_uint32_t frame_byte_count;
    endian::little_uint32_t texture_count;
    endian::little_uint32_t vertex_per_frame_count;
    endian::little_uint32_t face_count;
    endian::little_uint32_t unknown_count;
    endian::little_uint32_t frame_count;
    endian::little_uint32_t padding;
    endian::little_uint32_t padding2;
    endian::little_uint32_t sub_object_count;
    endian::little_uint32_t texture_offset;
    endian::little_uint32_t face_offset;
    endian::little_uint32_t frame_offset;
    endian::little_uint32_t unknown_offset;
    endian::little_uint32_t vertex_group_offset;
    endian::little_uint32_t padding3;
    endian::little_uint32_t padding4;
    endian::little_uint32_t frame_bounding_box_offset;
  };

  struct mdx_frame : md2_frame
  {
    std::array<float, 6> bounding_box;
  };

  struct mdx_sub_object_grouping
  {
    std::vector<endian::little_uint32_t> vertex_groupings;
  };

  struct mdx_shape
  {
    endian::little_uint32_t texture_width;
    endian::little_uint32_t texture_height;
    std::vector<std::string> texture_filenames;
    std::vector<md2_face> faces;
    std::vector<mdx_frame> frames;
    std::vector<mdx_sub_object_grouping> sub_object_groupings;
  };

  bool is_mdx(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 4> tag;
    stream.read((char*)&tag, sizeof(tag));
    return tag == mdx_tag;
  }

  std::any load_mdx(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);

    auto start = (std::size_t)resetter.position;

    mdx_shape shape;

    mdx_header header;
    stream.read((char*)&header, sizeof(header));

    if (header.tag != mdx_tag && header.version != 4)
    {
      return std::any{};
    }

    shape.texture_width = header.texture_width;
    shape.texture_height = header.texture_height;

    auto frame_size = header.frame_byte_count;
    auto extra_data_size = frame_size + sizeof(std::vector<mdl_vertex>) - sizeof(md2_frame);

    if (header.vertex_per_frame_count * sizeof(mdl_vertex) != extra_data_size)
    {
      return std::any{};
    }

    shape.texture_filenames.reserve(header.texture_count);
    stream.seekg(start + header.texture_offset, std::ios::beg);
    std::array<char, 64> temp;
    for (auto i = 0u; i < header.texture_count; ++i)
    {
      stream.read(temp.data(), temp.size());
      temp[63] = 0;
      shape.texture_filenames.emplace_back(temp.data());
    }

    shape.faces.resize(header.face_count);
    stream.seekg(start + header.face_offset, std::ios::beg);
    stream.read((char*)shape.faces.data(), shape.faces.size() * sizeof(md2_face));

    shape.frames.reserve(header.frame_count);

    stream.seekg(start + header.frame_offset, std::ios::beg);

    for (auto i = 0u; i < header.frame_count; ++i)
    {
      auto& frame = shape.frames.emplace_back();
      stream.read((char*)&frame, sizeof(frame) - sizeof(frame.vertices) - sizeof(frame.bounding_box));

      frame.vertices.reserve(header.vertex_per_frame_count);
      for (auto v = 0u; v < header.vertex_per_frame_count; ++v)
      {
        auto& vertex = frame.vertices.emplace_back();
        stream.read((char*)&vertex, sizeof(vertex));
      }
    }

    stream.seekg(start + header.frame_bounding_box_offset, std::ios::beg);

    for (auto i = 0u; i < header.frame_count; ++i)
    {
      auto& frame = shape.frames[i];
      stream.read((char*)&frame.bounding_box, sizeof(frame.bounding_box));
    }

    stream.seekg(start + header.vertex_group_offset, std::ios::beg);

    shape.sub_object_groupings.reserve(header.sub_object_count);
    for (auto g = 0u; g < header.sub_object_count; ++g)
    {
      auto& grouping = shape.sub_object_groupings.emplace_back();
      grouping.vertex_groupings.resize(header.vertex_per_frame_count);
      stream.read((char*)grouping.vertex_groupings.data(), grouping.vertex_groupings.size() * sizeof(std::uint32_t));
    }

    return shape;
  }


  struct dkm_face
  {
    endian::little_uint32_t padding;
    std::array<endian::little_uint16_t, 3> vertex_indices;
    std::array<endian::little_uint16_t, 3> uv_indices;
  };

  struct dkm_header
  {
    std::array<std::byte, 4> tag;
    endian::little_uint32_t version;
    endian::little_uint32_t texture_width;
    endian::little_uint32_t texture_height;
    endian::little_uint32_t unknown;
    endian::little_uint32_t frame_byte_count;
    endian::little_uint32_t texture_count;
    endian::little_uint32_t vertex_per_frame_count;
    endian::little_uint32_t uv_count;
    endian::little_uint32_t face_count;
    endian::little_uint32_t unknown_count;
    endian::little_uint32_t frame_count;
    endian::little_uint32_t unknown_count2;
    endian::little_uint32_t texture_offset;
    endian::little_uint32_t uv_offset;
    endian::little_uint32_t face_offset;
    endian::little_uint32_t frame_offset;
    endian::little_uint32_t unknown_offset;
    endian::little_uint32_t eof_offset;
    endian::little_uint32_t unknown_count3;
    endian::little_uint32_t unknown_offset3;
  };


}// namespace siege::content::mdl