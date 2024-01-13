#ifndef DARKSTARDTSCONVERTER_TORQUE_DTS_STRUCTURES_HPP
#define DARKSTARDTSCONVERTER_TORQUE_DTS_STRUCTURES_HPP

#include <vector>
#include <string>
#include <variant>
#include "endian_arithmetic.hpp"
#include "darkstar_structures.hpp"

namespace studio::content::dts::torque
{
  namespace endian = studio::endian;

  struct guard
  {
    endian::little_int32_t guard_32;
    endian::little_int16_t guard_16;
    std::byte guard_8;
  };

  struct bit_set
  {
    endian::little_int32_t dummy;
    endian::little_int32_t num_words;
    std::vector<endian::little_int32_t> bits;
  };

  struct sequence
  {
    endian::little_int32_t name_index;
    endian::little_uint32_t flags;
    endian::little_int32_t num_key_frames;
    float duration;
    endian::little_int32_t priority;
    endian::little_int32_t first_ground_frame;
    endian::little_int32_t num_ground_frames;
    endian::little_int32_t base_rotation_index;
    endian::little_int32_t base_translation_index;
    endian::little_int32_t base_scale_index;
    endian::little_int32_t base_object_state;
    endian::little_int32_t base_decal_state;
    endian::little_int32_t first_trigger;
    endian::little_int32_t num_triggers;
    float tool_begin;
    bit_set rotation_matters;
    bit_set translation_matters;
    bit_set scale_matters;
    bit_set decal_matters;
    bit_set ifl_matters;
    bit_set vis_matters;
    bit_set frame_matters;
    bit_set mat_frame_matters;
  };

  struct node
  {
    endian::little_int32_t name_index;
    endian::little_int32_t parent_index;
    endian::little_int32_t first_object_index;
    endian::little_int32_t first_child_index;
    endian::little_int32_t next_sibling_index;
  };

  struct object
  {
    endian::little_int32_t name_index;
    endian::little_int32_t num_meshes;
    endian::little_int32_t start_mesh_index;
    endian::little_int32_t node_index;
    endian::little_int32_t next_sibling_index;
    endian::little_int32_t first_decal_index;
  };

  struct detail
  {
    endian::little_int32_t name_index;
    endian::little_int32_t sub_shape_num;
    endian::little_int32_t object_detail_num;
    float size;
    float average_error;
    float max_error;
    endian::little_int32_t poly_count;
  };

  namespace v26
  {
    struct detail
    {
      endian::little_int32_t name_index;
      endian::little_int32_t sub_shape_num;
      endian::little_int32_t object_detail_num;
      float size;
      float average_error;
      float max_error;
      endian::little_int32_t poly_count;
      endian::little_int32_t bounding_box_dimension;
      endian::little_int32_t bounding_box_detail_level;
      endian::little_int32_t bounding_box_equator_steps;
      endian::little_int32_t bounding_box_polar_steps;
      float bounding_box_polar_angle;
      endian::little_int32_t bounding_box_include_poles;
    };
  }// namespace v26

  struct primitive
  {
    endian::little_int16_t start_index;
    endian::little_int16_t num_elements;
  };

  struct primitive_material
  {
    endian::little_uint32_t mat_index;
  };

  namespace v25
  {
    struct primitive
    {
      endian::little_int32_t start_index;
      endian::little_int32_t num_elements;
      endian::little_uint32_t mat_index;
    };
  }// namespace v25

  struct mesh_header
  {
    endian::little_int32_t type;
    guard checkpoint1;
    endian::little_int32_t num_frames;
    endian::little_int32_t num_material_frames;
    endian::little_int32_t parent_mesh;
    darkstar::dts::vector3f_pair bounds;
    darkstar::dts::vector3f center;
    float radius;
    endian::little_int32_t num_vertices;
    std::vector<darkstar::dts::vector3f> vertices;
    endian::little_int32_t num_texture_vertices;
    std::vector<std::array<float, 2>> texture_vertices;
  };

  //TODO finish this
  struct skin_mesh_details
  {
    endian::little_int32_t num_initial_vertices;
    endian::little_int32_t initial_verices;
  };

  struct sorted_mesh
  {
  };

  //TODO deal with multiple versions

  struct mesh
  {
    mesh_header header;
    //normal
    std::vector<darkstar::dts::vector3f> norms;
    std::vector<std::uint8_t> encoded_normals;

    endian::little_int32_t num_primitives;
    std::vector<primitive> primitives;

    std::vector<primitive_material> primitive_materials;

    endian::little_int32_t num_indices;

    std::vector<endian::little_int16_t> indices;

    endian::little_int32_t num_merge_indices;

    std::vector<endian::little_int16_t> merge_indices;

    endian::little_int32_t vertices_per_frame;
    endian::little_uint32_t flags;

    guard checkpoint2;
  };

  namespace v25
  {
    struct mesh
    {
      mesh_header header;
      //normal
      std::vector<darkstar::dts::vector3f> norms;
      std::vector<std::uint8_t> encoded_normals;

      endian::little_int32_t num_primitives;

      std::vector<v25::primitive> primitives;

      endian::little_int32_t num_indices;

      std::vector<endian::little_int32_t> indices;

      endian::little_int32_t num_merge_indices;

      std::vector<endian::little_int16_t> merge_indices;

      endian::little_int32_t vertices_per_frame;
      endian::little_uint32_t flags;

      guard checkpoint2;
    };
  }// namespace v26


  namespace v26
  {
    struct mesh
    {
      mesh_header header;
      //v26
      endian::little_int32_t num_texture_vertices2;
      std::vector<std::array<float, 2>> texture_vertices2;
      endian::little_int32_t num_vertex_colors;
      std::vector<darkstar::dts::rgb_data> vertex_colors;

      //normal
      std::vector<darkstar::dts::vector3f> norms;
      std::vector<std::uint8_t> encoded_normals;

      endian::little_int32_t num_primitives;

      std::vector<v25::primitive> primitives;

      endian::little_int32_t num_indices;

      std::vector<endian::little_int32_t> indices;

      endian::little_int32_t num_merge_indices;

      std::vector<endian::little_int16_t> merge_indices;

      endian::little_int32_t vertices_per_frame;
      endian::little_uint32_t flags;

      guard checkpoint2;
    };
  }// namespace v26

  struct object_state
  {
    float visibility;
    endian::little_int32_t frame_index;
    endian::little_int32_t mat_frame;
  };

  struct trigger
  {
    endian::little_uint32_t state;
    float position;
  };

  struct ifl_material
  {
    endian::little_int32_t name_index;
    endian::little_int32_t material_slot;
    endian::little_int32_t first_frame_index;
    endian::little_int32_t first_frame_of_time_index;
    endian::little_int32_t num_frames;
  };

  struct decal
  {
    std::array<endian::little_int32_t, 5> dummy;
  };

  struct file_data
  {
    endian::little_int16_t dts_version_number;
    endian::little_int16_t exporter_version_number;
    endian::little_int32_t total_buffer_size;
    endian::little_int32_t offset_buffer_16;
    endian::little_int32_t offset_buffer_8;

    std::vector<endian::little_uint32_t> buffer_32;
    endian::little_uint16_t* buffer_16;
    std::byte* buffer_8;

    endian::little_int32_t num_sequences;
    std::vector<sequence> sequences;
    std::byte mat_stream_type;
    endian::little_int32_t num_materials;
    std::vector<std::string> material_names;
    std::vector<endian::little_uint32_t> material_flags;
    std::vector<endian::little_int32_t> material_reflectance_maps;
    std::vector<endian::little_int32_t> material_bump_maps;
    std::vector<endian::little_int32_t> material_detail_maps;
    std::vector<float> material_detail_scales;
    std::vector<float> material_detail_reflectance;
  };

  struct buffer_data
  {
    endian::little_int32_t num_nodes;
    endian::little_int32_t num_objects;
    endian::little_int32_t num_decals;
    endian::little_int32_t num_sub_shapes;
    endian::little_int32_t num_ifls;
    endian::little_int32_t num_node_rotations;
    endian::little_int32_t num_node_translations;
    endian::little_int32_t num_node_uniform_scales;
    endian::little_int32_t num_node_aligned_scales;
    endian::little_int32_t num_node_arb_scales;
    endian::little_int32_t num_ground_frames;
    endian::little_int32_t num_object_states;
    endian::little_int32_t num_decal_states;
    endian::little_int32_t num_triggers;
    endian::little_int32_t num_details;
    endian::little_int32_t num_meshes;
    endian::little_int32_t num_names;
    float smallest_visible_size;
    endian::little_int32_t smallest_visible_detail_level;
    guard checkpoint1;
    float radius;
    float tube_radius;
    darkstar::dts::vector3f center;
    darkstar::dts::vector3f_pair bounds;
    guard checkpoint2;
    std::vector<node> nodes;
    guard checkpoint3;
    std::vector<object> objects;
    guard checkpoint4;
    std::vector<decal> decals;
    guard checkpoint5;
    std::vector<ifl_material> ifl_materials;
    guard checkpoint6;
    std::vector<endian::little_int32_t> sub_shapes_first_node;
    std::vector<endian::little_int32_t> sub_shapes_first_object;
    std::vector<endian::little_int32_t> sub_shapes_first_decal;
    std::vector<endian::little_int32_t> sub_shapes_first_translucent_object;
    guard checkpoint7;
    std::vector<darkstar::dts::quaternion4s> default_rotations;
    std::vector<darkstar::dts::vector3f> default_translations;
    std::vector<darkstar::dts::quaternion4s> node_rotations;
    std::vector<darkstar::dts::vector3f> node_translations;
    guard checkpoint8;
    std::vector<float> node_uniform_scales;
    std::vector<darkstar::dts::vector3f> node_aligned_scales;
    std::vector<darkstar::dts::vector3f> node_arbitrary_scale_factors;
    std::vector<darkstar::dts::quaternion4s> node_arbitrary_scale_rotations;
    guard checkpoint9;
    std::vector<darkstar::dts::vector3f> ground_translations;
    std::vector<darkstar::dts::quaternion4s> ground_rotations;
    guard checkpoint10;
    std::vector<object_state> object_states;
    guard checkpoint11;
    std::vector<endian::little_int32_t> decal_states;
    guard checkpoint12;
    std::vector<trigger> triggers;
    guard checkpoint13;
    std::vector<std::variant<detail, v26::detail>> details;
    guard checkpoint14;
    std::vector<mesh> meshes;
    guard checkpoint15;
    std::vector<std::string> names;
    guard checkpoint16;
    std::vector<float> details_alpha_in;
    std::vector<float> details_alpha_out;
  };
}// namespace torque::dts

#endif//DARKSTARDTSCONVERTER_TORQUE_DTS_STRUCTURES_HPP
