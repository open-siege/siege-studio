#include <array>
#include <utility>
#include <vector>
#include <variant>
#include "shared.hpp"
#include "endian_arithmetic.hpp"
#include "content/tagged_data.hpp"

namespace studio::content::dts::three_space
{
  namespace endian = boost::endian;
  using file_tag = std::array<std::byte, 4>;

  namespace v1
  {
    constexpr file_tag grid_shape_tag = shared::to_tag<4>({ 0x01, 0x00, 0xbc, 0x02 });
    constexpr file_tag rectangle_region_tag = shared::to_tag<4>({ 0x0c, 0x00, 0x28, 0x00 });
    constexpr file_tag edge_table_tag = shared::to_tag<4>({ 0x0d, 0x00, 0x28, 0x00 });

    constexpr file_tag null_obj_tag = shared::to_tag<4>({ 0x0e, 0x00, 0x14, 0x00 });
    constexpr file_tag poly_tag = shared::to_tag<4>({ 0x01, 0x00, 0x14, 0x00 });
    constexpr file_tag solid_poly_tag = shared::to_tag<4>({ 0x02, 0x00, 0x14, 0x00 });
    constexpr file_tag texture_for_poly_tag = shared::to_tag<4>({ 0x0f, 0x00, 0x14, 0x00 });
    constexpr file_tag shaded_polygon_tag = shared::to_tag<4>({ 0x03, 0x00, 0x14, 0x00 });
    constexpr file_tag gourand_poly_tag = shared::to_tag<4>({ 0x09, 0x00, 0x14, 0x00 });
    constexpr file_tag alias_solid_poly_tag = shared::to_tag<4>({ 0x10, 0x00, 0x14, 0x00 });
    constexpr file_tag alias_shaded_poly_tag = shared::to_tag<4>({ 0x11, 0x00, 0x14, 0x00 });
    constexpr file_tag alias_gourand_poly_tag = shared::to_tag<4>({ 0x12, 0x00, 0x14, 0x00 });
    constexpr file_tag group_tag = shared::to_tag<4>({ 0x14, 0x00, 0x14, 0x00 });
    constexpr file_tag bsp_group_tag = shared::to_tag<4>({ 0x0a, 0x00, 0x14, 0x00 });
    constexpr file_tag base_part_tag = shared::to_tag<4>({ 0x05, 0x00, 0x14, 0x00 });
    constexpr file_tag bitmap_part_tag = shared::to_tag<4>({ 0x13, 0x00, 0x14, 0x00 });
    constexpr file_tag part_list_tag = shared::to_tag<4>({ 0x07, 0x00, 0x14, 0x00 });
    constexpr file_tag cell_anim_part_tag = shared::to_tag<4>({ 0x0b, 0x00, 0x14, 0x00 });
    constexpr file_tag detail_part_tag = shared::to_tag<4>({ 0x0c, 0x00, 0x14, 0x00 });
    constexpr file_tag bsp_part_tag = shared::to_tag<4>({ 0x15, 0x00, 0x14, 0x00 });
    constexpr file_tag shape_tag = shared::to_tag<4>({ 0x08, 0x00, 0x14, 0x00 });
    constexpr file_tag an_sequence_tag = shared::to_tag<4>({ 0x01, 0x00, 0x1e, 0x00 });
    constexpr file_tag an_cyclic_sequence_tag = shared::to_tag<4>({ 0x04, 0x00, 0x1e, 0x00 });
    constexpr file_tag an_anim_list_tag = shared::to_tag<4>({ 0x02, 0x00, 0x1e, 0x00 });
    constexpr file_tag an_shape_tag = shared::to_tag<4>({ 0x03, 0x00, 0x1e, 0x00 });
    constexpr file_tag dc_shape_tag = shared::to_tag<4>({ 0xf8, 0x01, 0xbc, 0x02 });
    constexpr file_tag dc_shape_instance_tag = shared::to_tag<4>({ 0xf5, 0x01, 0xbc, 0x02 });

    struct base_part;
    struct bsp_part;
    struct group;
    struct bsp_group;
    struct part_list;
    struct shape;
    struct poly;
    struct gouraud_poly;
    struct shaded_poly;
    struct solid_poly;
    struct texture_for_poly;
    struct cell_anim_part;
    struct detail_part;
    struct bitmap_part;

    using shape_item = std::variant<base_part, part_list, bsp_part, cell_anim_part, detail_part, bitmap_part,
      group, bsp_group, gouraud_poly, shaded_poly, solid_poly, texture_for_poly,poly, shape>;

    using shape_reader_map = tagged_item_map<shape_item>::tagged_item_reader_map;

    struct vector3s
    {
      endian::little_int16_t x;
      endian::little_int16_t y;
      endian::little_int16_t z;
    };

    struct base_part
    {
      endian::little_int16_t transform;
      endian::little_int16_t id_number;
      endian::little_int16_t radius;
      vector3s center;
    };

    struct group
    {
      base_part base;
      endian::little_uint16_t index_count;
      endian::little_uint16_t point_count;
      endian::little_uint16_t color_count;
      endian::little_uint16_t item_count;

      std::vector<endian::little_uint16_t> indexes;
      std::vector<vector3s> points;
      std::vector<std::array<std::byte, 4>> colors;
      std::vector<shape_item> items;
    };

    struct bsp_group
    {
      struct bsp_group_node
      {
        endian::little_int32_t coeff;
        endian::little_int16_t poly;
        endian::little_int16_t front;
        endian::little_int16_t back;
      };

      group base;
      endian::little_uint16_t node_count;
      std::vector<bsp_group_node> nodes;
    };

    struct bitmap_part
    {
      base_part base;
      endian::little_uint16_t bmp_tag;
      std::byte x_offset;
      std::byte y_offset;
    };

    struct poly
    {
      endian::little_uint16_t normal;
      endian::little_uint16_t center;
      endian::little_uint16_t vertex_count;
      endian::little_uint16_t vertex_list;
    };

    struct solid_poly
    {
      poly base;
      endian::little_uint16_t colors;
    };

    struct gouraud_poly
    {
      solid_poly base;
      endian::little_uint16_t normal_list;
    };

    struct texture_for_poly
    {
      solid_poly base;
    };

    struct shaded_poly
    {
      solid_poly base;
    };

    struct part_list
    {
      base_part base;
      endian::little_uint16_t part_count;
      std::vector<shape_item> parts;
    };

    struct shape
    {
      part_list base;
      endian::little_uint16_t transform_list_count;
      endian::little_uint16_t sequence_list_count;
      std::vector<endian::little_int32_t> sequence_list;
      std::vector<endian::little_int32_t> transform_list;
    };

    struct detail_part
    {
      part_list base;
      endian::little_uint16_t detail_count;
      std::vector<endian::little_int16_t> details;
    };

    struct cell_anim_part
    {
      part_list base;
      endian::little_int16_t anim_sequence;
    };

    struct bsp_part
    {
      struct bsp_node
      {
        vector3s normal;
        endian::little_int32_t coeff;
        endian::little_int16_t front;
        endian::little_int16_t back;
      };

      part_list base;
      endian::little_uint16_t node_count;
      std::vector<bsp_node> nodes;
      std::vector<endian::little_uint16_t> transforms;
    };

    struct an_sequence
    {
      struct frame
      {
        endian::little_int32_t tick;
        endian::little_int32_t first_transition;
        endian::little_int32_t transition_count;
      };

      endian::little_int16_t tick;
      endian::little_int16_t priority;
      endian::little_int16_t gm;
      endian::little_int16_t frame_count;
      std::vector<frame> frames;
      endian::little_uint16_t part_list_count;
      std::vector<shape_item> part_list;
      std::vector<std::vector<endian::little_int32_t>> transform_index_list;
    };

    struct an_anim_list
    {
      struct transition
      {
        endian::little_int32_t tick;
        endian::little_int32_t dest_sequence;
        endian::little_int32_t dest_frame;
        endian::little_int32_t ground_movement;
      };

      struct transform
      {
        endian::little_int32_t rx;
        endian::little_int32_t ry;
        endian::little_int32_t rz;
        endian::little_int32_t tx;
        endian::little_int32_t ty;
        endian::little_int32_t tz;
      };

      struct relation
      {
        endian::little_int32_t parent;
        endian::little_int32_t destination;
      };

      endian::little_int16_t sequence_count;
      std::vector<an_sequence> sequences;

      endian::little_int16_t transition_count;
      std::vector<transition> transitions;

      endian::little_int16_t transform_count;
      std::vector<transform> transforms;

      endian::little_int16_t default_transform_count;
      std::vector<endian::little_int32_t> default_transforms;

      endian::little_int16_t relations_count;
      std::vector<relation> relations;
    };

    base_part read_base_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map&)
    {
      base_part base{};
      file.read(reinterpret_cast<std::byte*>(&base.transform), sizeof(base.transform));
      file.read(reinterpret_cast<std::byte*>(&base.id_number), sizeof(base.id_number));
      file.read(reinterpret_cast<std::byte*>(&base.radius), sizeof(base.radius));
      file.read(reinterpret_cast<std::byte*>(&base.center), sizeof(base.center));

      return base;
    }

    part_list read_part_list(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      part_list parts{};
      parts.base = read_base_part(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&parts.part_count), sizeof(parts.part_count));
      parts.parts = read_children<shape_item>(file, parts.part_count, readers);
      return parts;
    }

    bsp_part read_bsp_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      bsp_part shape{};
      shape.base = read_part_list(file, header, readers);

      file.read(reinterpret_cast<std::byte*>(&shape.node_count), sizeof(shape.node_count));

      shape.nodes = std::vector<bsp_part::bsp_node>(shape.node_count);
      file.read(reinterpret_cast<std::byte*>(shape.nodes.data()), sizeof(bsp_part::bsp_node) * shape.node_count);

      shape.transforms = std::vector<endian::little_uint16_t>(shape.node_count);
      file.read(reinterpret_cast<std::byte*>(shape.transforms.data()), sizeof(endian::little_uint16_t) * shape.node_count);

      return shape;
    }

    group read_group(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      group shape{};

      shape.base = read_base_part(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.index_count), sizeof(shape.index_count));
      file.read(reinterpret_cast<std::byte*>(&shape.point_count), sizeof(shape.point_count));
      file.read(reinterpret_cast<std::byte*>(&shape.color_count), sizeof(shape.color_count));
      file.read(reinterpret_cast<std::byte*>(&shape.item_count), sizeof(shape.item_count));

      shape.indexes = std::vector<endian::little_uint16_t>(shape.index_count);
      file.read(reinterpret_cast<std::byte*>(shape.indexes.data()), sizeof(endian::little_uint16_t) * shape.index_count);

      shape.points = std::vector<vector3s>(shape.point_count);
      file.read(reinterpret_cast<std::byte*>(shape.points.data()), sizeof(vector3s) * shape.point_count);

      shape.colors = std::vector<std::array<std::byte, 4>>(shape.color_count);
      file.read(reinterpret_cast<std::byte*>(shape.colors.data()), sizeof(std::array<std::byte, 4>) * shape.color_count);

      shape.items = read_children<shape_item>(file, shape.item_count, readers);

      return shape;
    }

    bsp_group read_bsp_group(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      bsp_group shape{};

      shape.base = read_group(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.node_count), sizeof(shape.node_count));

      shape.nodes = std::vector<bsp_group::bsp_group_node>(shape.node_count);
      file.read(reinterpret_cast<std::byte*>(shape.nodes.data()), sizeof(bsp_group::bsp_group_node) * shape.node_count);

      return shape;
    }

    poly read_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      poly poly{};

      file.read(reinterpret_cast<std::byte*>(&poly.normal), sizeof(poly.normal));
      file.read(reinterpret_cast<std::byte*>(&poly.center), sizeof(poly.center));
      file.read(reinterpret_cast<std::byte*>(&poly.vertex_count), sizeof(poly.vertex_count));
      file.read(reinterpret_cast<std::byte*>(&poly.vertex_list), sizeof(poly.vertex_list));

      return poly;
    }

    solid_poly read_solid_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      solid_poly poly{};
      poly.base = read_poly(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&poly.colors), sizeof(poly.colors));

      return poly;
    }

    texture_for_poly read_texture_for_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      texture_for_poly poly{};
      poly.base = read_solid_poly(file, header, readers);

      return poly;
    }

    gouraud_poly read_gouraud_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      gouraud_poly poly{};
      poly.base = read_solid_poly(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&poly.normal_list), sizeof(poly.normal_list));

      return poly;
    }

    shaded_poly read_shaded_poly(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      shaded_poly poly{};
      poly.base = read_solid_poly(file, header, readers);

      return poly;
    }

    cell_anim_part read_cell_anim_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      cell_anim_part shape{};
      shape.base = read_part_list(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.anim_sequence), sizeof(shape.anim_sequence));
      return shape;
    }

    detail_part read_detail_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      detail_part shape{};
      shape.base = read_part_list(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.detail_count), sizeof(shape.detail_count));

      shape.details = std::vector<endian::little_int16_t>(shape.detail_count);
      file.read(reinterpret_cast<std::byte*>(shape.details.data()), sizeof(endian::little_int16_t) * shape.detail_count);

      return shape;
    }

    bitmap_part read_bitmap_part(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      bitmap_part shape{};
      shape.base = read_base_part(file, header, readers);
      file.read(reinterpret_cast<std::byte*>(&shape.bmp_tag), sizeof(shape.bmp_tag));
      file.read(reinterpret_cast<std::byte*>(&shape.x_offset), sizeof(shape.x_offset));
      file.read(reinterpret_cast<std::byte*>(&shape.y_offset), sizeof(shape.y_offset));

      return shape;
    }

    shape read_shape(std::basic_istream<std::byte>& file, object_header& header, shape_reader_map& readers)
    {
      shape shape{};
      shape.base = read_part_list(file, header, readers);
      return shape;
    }

    std::vector<shape_item> read_shapes(std::basic_istream<std::byte>& file)
    {
      static shape_reader_map readers = {
        { shape_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_shape(file, header, readers); } } },
        { bsp_part_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_bsp_part(file, header, readers); } } },
        { part_list_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_part_list(file, header, readers); } } },
        { base_part_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_base_part(file, header, readers); } } },
        { bitmap_part_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_bitmap_part(file, header, readers); } } },
        { cell_anim_part_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_cell_anim_part(file, header, readers); } } },
        { group_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_group(file, header, readers); } } },
        { bsp_group_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_bsp_group(file, header, readers); } } },
        { poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_poly(file, header, readers); } } },
        { texture_for_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_texture_for_poly(file, header, readers); } } },
        { gourand_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_gouraud_poly(file, header, readers); } } },
        { alias_gourand_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_gouraud_poly(file, header, readers); } } },
        { solid_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_solid_poly(file, header, readers); } } },
        { alias_solid_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_solid_poly(file, header, readers); } } },
        { shaded_polygon_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_shaded_poly(file, header, readers); } } },
        { alias_shaded_poly_tag, { [](auto& file, auto& header, auto& readers) -> shape_item { return read_shaded_poly(file, header, readers); } } },
      };

      return read_children<shape_item>(file, 1, readers);
    }
  }// namespace v1

  namespace v2
  {
    constexpr file_tag material_list_tag = shared::to_tag<4>({ 0x1f, 0x00, 0x14, 0x00 });
    constexpr file_tag material_tag = shared::to_tag<4>({ 0x1e, 0x00, 0x14, 0x00 });
    constexpr file_tag bsp_part_tag = shared::to_tag<4>({ 0x46, 0x00, 0x14, 0x00 });
    constexpr file_tag cell_anim_part_tag = shared::to_tag<4>({ 0x51, 0x00, 0x14, 0x00 });
    constexpr file_tag detail_part_tag = shared::to_tag<4>({ 0x5a, 0x00, 0x14, 0x00 });
    constexpr file_tag mesh_tag = shared::to_tag<4>({ 0x28, 0x00, 0x14, 0x00 });
    constexpr file_tag part_list_tag = shared::to_tag<4>({ 0x3c, 0x00, 0x14, 0x00 });
    constexpr file_tag shape_tag = shared::to_tag<4>({ 0x64, 0x00, 0x14, 0x00 });
    constexpr file_tag null_part_tag = shared::to_tag<4>({ 0x78, 0x00, 0x14, 0x00 });
    constexpr file_tag bitmap_frame_tag = shared::to_tag<4>({ 0x6e, 0x00, 0x14, 0x00 });

    struct base_part;
    struct part_list;
    struct shape;

    using shape_item = std::variant<base_part, part_list, shape>;

    using shape_items = std::vector<shape_item>;

    struct vector3l
    {
      endian::little_int32_t x;
      endian::little_int32_t y;
      endian::little_int32_t z;
    };

    struct base_part
    {
      endian::little_int32_t transform;
      endian::little_int32_t id_number;
      endian::little_int32_t radius;
      vector3l center;
    };

    struct part_list
    {
      base_part base;

      endian::little_uint32_t part_count;
      shape_items parts;
    };

    struct shape
    {
      part_list base;
      endian::little_uint32_t sequence_count;
      std::vector<endian::little_int32_t> sequences;
    };

    struct bsp_part
    {
      struct bsp_node
      {
        vector3l normal;
        endian::little_int32_t coeff;
        endian::little_int32_t front;
        endian::little_int32_t back;
      };

      part_list base;

      endian::little_uint32_t node_count;
      std::vector<bsp_node> nodes;
      std::vector<endian::little_int32_t> transforms;
    };

    struct detail_part
    {
      part_list base;
      endian::little_uint32_t detail_count;
      std::vector<endian::little_int32_t> details;
    };

    struct bitmap_frame_part
    {
      base_part base;
      endian::little_uint32_t bmp_tag;
      endian::little_int32_t x_offset;
      endian::little_int32_t y_offset;
      endian::little_int32_t rotation;
      endian::little_int32_t squish;
    };

    struct cell_anim_part
    {
      part_list base;
      endian::little_int32_t anim_sequence;
      endian::little_int32_t cell_count;
      std::vector<endian::little_int32_t> cells;
    };

    struct material
    {
      endian::little_int32_t null;
      endian::little_int32_t unknown0;
      endian::little_int32_t material_type;
      endian::little_int32_t unknown1;
      endian::little_int32_t palette;
      endian::little_int32_t rgb;
      endian::little_int32_t flags;
      std::byte texture;
    };

    struct material_list
    {
      endian::little_uint32_t material_count;
      endian::little_uint32_t detail_count;
      std::vector<material> materials;
    };

    struct mesh
    {
      struct vertex
      {
        vector3l coordinates;
        vector3l normal;
      };

      struct face
      {
        vector3l vertices;
        vector3l texture_vertices;
        endian::little_int32_t center;
        endian::little_int32_t material;
      };

      endian::little_uint32_t vertex_count;
      std::vector<vertex> vertices;

      endian::little_uint32_t texture_vertex_count;
      std::vector<std::pair<endian::little_uint32_t, endian::little_uint32_t>> texture_vertices;

      endian::little_uint32_t face_count;
      std::vector<face> faces;
    };


  }// namespace v2
}// namespace studio::content::dts::three_space