#ifndef INC_3SPACESTUDIO_3SPACE_HPP
#define INC_3SPACESTUDIO_3SPACE_HPP

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

  namespace v2
  {
    struct base_part;
    struct bsp_part;
    struct nu_bsp_part;
    struct part_list;
    struct shape;
    struct mesh;
    struct nu_shape;
    struct nu_mesh;
    struct nu_anim_mesh;
    struct cell_anim_part;
    struct detail_part;
    struct bitmap_frame_part;
    struct null_part;

    using shape_item = std::variant<raw_item, null_part, base_part, part_list, bsp_part, cell_anim_part,
      detail_part, bitmap_frame_part, mesh, shape, nu_shape, nu_bsp_part, nu_anim_mesh, nu_anim_mesh>;

    bool is_3space_dts(std::istream& stream);
    std::vector<shape_item> read_shapes(std::istream& file);

    using shape_reader_map = tagged_item_map<shape_item>::tagged_item_reader_map;

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

    struct null_part
    {
      base_part base;
    };

    struct part_list
    {
      base_part base;

      endian::little_uint32_t part_count;
      std::vector<shape_item> parts;
    };

    struct shape
    {
      part_list base;
      endian::little_uint32_t sequence_count;
      std::vector<endian::little_int32_t> sequences;
    };

    struct nu_shape
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

    struct nu_bsp_part
    {
      bsp_part base;
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
      std::string texture;
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

      base_part base;

      endian::little_uint32_t vertex_count;
      std::vector<vertex> vertices;

      endian::little_uint32_t texture_vertex_count;
      std::vector<std::pair<endian::little_uint32_t, endian::little_uint32_t>> texture_vertices;

      endian::little_uint32_t face_count;
      std::vector<face> faces;
    };

    struct nu_mesh
    {
      mesh base;
    };

    struct nu_anim_mesh
    {
      struct key_frame
      {
        endian::little_int32_t transform_index;
        vector3l coordinates;
      };

      struct vertex
      {
        endian::little_int32_t duration;
        std::vector<key_frame> key_frames;
      };

      struct face
      {
        vector3l vertices;
        vector3l texture_vertices;
        endian::little_int32_t center;
        endian::little_int32_t material;
      };

      base_part base;

      endian::little_uint32_t face_count;
      endian::little_uint32_t key_frame_count;
      endian::little_uint32_t vertex_count;
      endian::little_uint32_t texture_vertex_count;

      std::vector<std::pair<endian::little_uint32_t, endian::little_uint32_t>> texture_vertices;
      std::vector<vertex> vertices;
      std::vector<face> faces;
    };
  }// namespace v2
}// namespace studio::content::dts::three_space

#endif//INC_3SPACESTUDIO_3SPACE_HPP
