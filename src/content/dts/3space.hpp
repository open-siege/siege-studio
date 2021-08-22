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

  namespace v1
  {
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
    struct an_anim_list;
    struct an_sequence;
    struct an_cyclic_sequence;
    struct an_shape;

    using shape_item = std::variant<raw_item, base_part, part_list, bsp_part, cell_anim_part, detail_part, bitmap_part, group, bsp_group, gouraud_poly,
      shaded_poly, solid_poly, texture_for_poly, poly, shape, an_sequence, an_cyclic_sequence, an_anim_list, an_shape>;

    using actual_shape_item = std::variant<shape, an_shape>;

    using group_item = std::variant<group, bsp_group>;

    using poly_item = std::variant<poly, gouraud_poly, shaded_poly, solid_poly, texture_for_poly>;

    using part_list_item = std::variant<shape, an_shape, part_list, bsp_part, cell_anim_part, detail_part>;

    // TODO update the anim list reading code to handle parsing these.
    using sequence_item = std::variant<an_sequence, an_cyclic_sequence>;

    using shape_reader_map = tagged_item_map<shape_item>::tagged_item_reader_map;

    bool is_3space_dts(std::basic_istream<std::byte>& stream);
    std::vector<shape_item> read_shapes(std::basic_istream<std::byte>& file);

    struct vector3s
    {
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "x", "y", "z" });
      endian::little_int16_t x;
      endian::little_int16_t y;
      endian::little_int16_t z;
    };

    struct base_part
    {
      constexpr static auto type_name = "base_part";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "transform", "idNumber", "radius", "center" });

      endian::little_int16_t transform;
      endian::little_int16_t id_number;
      endian::little_int16_t radius;
      vector3s center;
    };

    struct group
    {
      constexpr static auto type_name = "group";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "indexCount", "pointCount", "colorCount", "itemCount", "indexes", "points", "colors", "items" });

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
      constexpr static auto type_name = "bsp_group";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "nodeCount", "nodes" });
      struct bsp_group_node
      {
        KEYS_CONSTEXPR static auto keys = shared::make_keys({ "coeff", "poly", "front", "back" });
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
      constexpr static auto type_name = "bitmap_part";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "bmpTag", "xOffset", "yOffset" });

      base_part base;
      endian::little_uint16_t bmp_tag;
      std::byte x_offset;
      std::byte y_offset;
    };

    struct poly
    {
      constexpr static auto type_name = "poly";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "normal", "center", "vertexCount", "vertexList" });
      endian::little_uint16_t normal;
      endian::little_uint16_t center;
      endian::little_uint16_t vertex_count;
      endian::little_uint16_t vertex_list;
    };

    struct solid_poly
    {
      constexpr static auto type_name = "solid_poly";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "colors" });
      poly base;
      endian::little_uint16_t colors;
    };

    struct gouraud_poly
    {
      constexpr static auto type_name = "gouraud_poly";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "normal_list" });
      solid_poly base;
      endian::little_uint16_t normal_list;
    };

    struct texture_for_poly
    {
      constexpr static auto type_name = "texture_for_poly";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base" });
      solid_poly base;
    };

    struct shaded_poly
    {
      constexpr static auto type_name = "shaded_poly";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base" });
      solid_poly base;
    };

    struct part_list
    {
      constexpr static auto type_name = "part_list";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "partCount", "parts" });
      base_part base;
      endian::little_uint16_t part_count;
      std::vector<shape_item> parts;
    };

    struct shape
    {
      constexpr static auto type_name = "shape";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "transformListCount", "sequenceListCount", "sequences", "transforms", "extraParts" });
      part_list base;
      endian::little_uint16_t transform_list_count;
      endian::little_uint16_t sequence_list_count;
      std::vector<endian::little_int16_t> sequence_list;
      std::vector<endian::little_int16_t> transform_list;

      std::vector<shape_item> extra_parts;
    };

    struct detail_part
    {
      constexpr static auto type_name = "detail_part";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "detailCount", "details" });
      part_list base;
      endian::little_uint16_t detail_count;
      std::vector<endian::little_int16_t> details;
    };

    struct cell_anim_part
    {
      constexpr static auto type_name = "cell_anim_part";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "animSequence" });
      part_list base;
      endian::little_int16_t anim_sequence;
    };

    struct bsp_part
    {
      constexpr static auto type_name = "bsp_part";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base", "nodeCount", "nodes", "transforms" });

      struct bsp_node
      {
        KEYS_CONSTEXPR static auto keys = shared::make_keys({ "normal", "coeff", "front", "back" });
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
      constexpr static auto type_name = "an_sequence";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "tick", "priority", "gm", "frameCount", "frames", "partListCount", "parts", "transformIndexes" });

      struct frame
      {
        KEYS_CONSTEXPR static auto keys = shared::make_keys({ "tick", "firstTransition", "transitionCount" });
        endian::little_uint16_t tick;
        endian::little_uint16_t first_transition;
        endian::little_uint16_t transition_count;
      };

      endian::little_int16_t tick;
      endian::little_int16_t priority;
      endian::little_int16_t gm;
      endian::little_int16_t frame_count;
      std::vector<frame> frames;
      endian::little_uint16_t part_list_count;
      std::vector<endian::little_uint16_t> part_list;
      std::vector<endian::little_uint16_t> transform_index_list;
    };

    struct an_cyclic_sequence
    {
      constexpr static auto type_name = "an_cyclic_sequence";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base" });
      an_sequence base;
    };

    struct an_anim_list
    {
      constexpr static auto type_name = "an_anim_list";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "sequenceCount", "sequences", "transitionCount", "transitions", "transformCount", "transforms", "defaultTransformCount", "defaultTransforms", "relationsCount", "relations" });

      struct transition
      {
        KEYS_CONSTEXPR static auto keys = shared::make_keys({ "tick", "destSequence", "destFrame", "groundMovement" });
        endian::little_uint16_t tick;
        endian::little_uint16_t dest_sequence;
        endian::little_uint16_t dest_frame;
        endian::little_uint16_t ground_movement;
      };

      struct transform
      {
        KEYS_CONSTEXPR static auto keys = shared::make_keys({ "rx", "ry", "rz", "tx", "ty", "tz" });

        endian::little_uint16_t rx;
        endian::little_uint16_t ry;
        endian::little_uint16_t rz;
        endian::little_int16_t tx;
        endian::little_int16_t ty;
        endian::little_int16_t tz;
      };

      struct relation
      {
        KEYS_CONSTEXPR static auto keys = shared::make_keys({ "parent", "destination" });

        endian::little_int16_t parent;
        endian::little_int16_t destination;
      };

      endian::little_int16_t sequence_count;
      std::vector<an_sequence> sequences;

      endian::little_int16_t transition_count;
      std::vector<transition> transitions;

      endian::little_int16_t transform_count;
      std::vector<transform> transforms;

      endian::little_int16_t default_transform_count;
      std::vector<endian::little_uint16_t> default_transforms;

      endian::little_int16_t relations_count;
      std::vector<relation> relations;
    };

    struct an_shape
    {
      constexpr static auto type_name = "an_shape";
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = shared::make_keys({ "base" });
      shape base;
    };

  }// namespace v1
}// namespace studio::content::dts::three_space

#endif//INC_3SPACESTUDIO_3SPACE_HPP
