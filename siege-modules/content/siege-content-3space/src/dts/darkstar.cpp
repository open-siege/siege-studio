module;

#include <siege/platform/shared.hpp>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/content/3d_structures.hpp>

export module siege.content.dts;

import std;

// DTS - Dynamix ThreeSpace Shape v3 for 3Space v3 (Starsiege, Tribes, Trophy Bass 3D, Driver's Education)

namespace siege::content::dts::darkstar
{

  namespace endian = siege::platform;
  export using file_tag = std::array<std::byte, 4>;

  export constexpr file_tag pers_tag = platform::to_tag<4>({ 'P', 'E', 'R', 'S' });

  export using version = endian::little_uint32_t;

  export struct file_info
  {
    endian::little_int32_t file_length;
    endian::little_int16_t class_name_length;
  };

  export struct tag_header
  {
    file_tag tag;
    darkstar::file_info file_info;
    std::string class_name;
    darkstar::version version;
  };

  namespace mesh::v1
  {
    export struct header
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "numVerts",
        "vertsPerFrame",
        "numTextureVerts",
        "numFaces",
        "numFrames",
        "scale",
        "origin",
        "radius" });

      endian::little_int32_t num_verts;
      endian::little_int32_t verts_per_frame;
      endian::little_int32_t num_texture_verts;
      endian::little_int32_t num_faces;
      endian::little_int32_t num_frames;
      vector3f scale;
      vector3f origin;
      float radius;
    };

    export struct vertex
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "x", "y", "z", "normal" });
      std::uint8_t x;
      std::uint8_t y;
      std::uint8_t z;
      std::uint8_t normal;
    };

    static_assert(sizeof(vertex) == sizeof(std::int32_t));

    export inline vector3f operator*(const vertex& left, const vector3f& right)
    {
      vector3f result{};

      result.x = left.x * right.x;
      result.y = left.y * right.y;
      result.z = left.z * right.z;

      return result;
    }


    export inline vector3f operator+(const vertex& left, const vector3f& right)
    {
      vector3f result{};

      result.x = left.x + right.x;
      result.y = left.y + right.y;
      result.z = left.z + right.z;

      return result;
    }

    export using texture_vertex = siege::content::texture_vertex;

    export struct face
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "vi1", "ti1", "vi2", "ti2", "vi3", "ti3", "material" });
      endian::little_int32_t vi1;
      endian::little_int32_t ti1;
      endian::little_int32_t vi2;
      endian::little_int32_t ti2;
      endian::little_int32_t vi3;
      endian::little_int32_t ti3;
      endian::little_int32_t material;
    };

    export struct frame
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "firstVert" });
      endian::little_int32_t first_vert;
    };

    export struct mesh
    {
      constexpr static auto type_name = std::string_view{ "TS::CelAnimMesh" };
      constexpr static auto version = 1;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header", "vertices", "textureVertices", "faces", "frames" });

      v1::header header;
      std::vector<vertex> vertices;
      std::vector<texture_vertex> texture_vertices;
      std::vector<face> faces;
      std::vector<frame> frames;
    };
  }// namespace mesh::v1

  namespace mesh::v2
  {
    export struct header
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "numVerts",
        "vertsPerFrame",
        "numTextureVerts",
        "numFaces",
        "numFrames",
        "textureVertsPerFrame",
        "scale",
        "origin",
        "radius" });

      endian::little_int32_t num_verts;
      endian::little_int32_t verts_per_frame;
      endian::little_int32_t num_texture_verts;
      endian::little_int32_t num_faces;
      endian::little_int32_t num_frames;
      endian::little_int32_t texture_verts_per_frame;
      vector3f scale;
      vector3f origin;
      float radius;
    };

    export struct mesh
    {
      constexpr static auto type_name = v1::mesh::type_name;
      constexpr static auto version = 2;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header", "vertices", "textureVertices", "faces", "frames" });

      v2::header header;
      std::vector<v1::vertex> vertices;
      std::vector<v1::texture_vertex> texture_vertices;
      std::vector<v1::face> faces;
      std::vector<v1::frame> frames;
    };
  }// namespace mesh::v2

  namespace mesh::v3
  {
    export struct header
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "numVerts",
        "vertsPerFrame",
        "numTextureVerts",
        "numFaces",
        "numFrames",
        "textureVertsPerFrame",
        "radius" });

      endian::little_int32_t num_verts;
      endian::little_int32_t verts_per_frame;
      endian::little_int32_t num_texture_verts;
      endian::little_int32_t num_faces;
      endian::little_int32_t num_frames;
      endian::little_int32_t texture_verts_per_frame;
      float radius;
    };

    export struct frame
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "firstVert", "scale", "origin" });
      endian::little_int32_t first_vert;
      vector3f scale;
      vector3f origin;
    };

    export struct mesh
    {
      constexpr static auto type_name = v1::mesh::type_name;
      constexpr static auto version = 3;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header", "vertices", "textureVertices", "faces", "frames" });

      v3::header header;
      std::vector<v1::vertex> vertices;
      std::vector<v1::texture_vertex> texture_vertices;
      std::vector<v1::face> faces;
      std::vector<v3::frame> frames;
    };

  }// namespace mesh::v3

  namespace material_list::v2
  {
    export struct header
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "numDetails",
        "numMaterials" });
      endian::little_int32_t num_details;
      endian::little_int32_t num_materials;
    };

    export struct material
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "flags", "alpha", "index", "rgbData", "fileName" });

      endian::little_int32_t flags;
      float alpha;
      endian::little_int32_t index;
      content::rgb_data rgb_data;

      std::array<char, 32> file_name;
    };

    export struct material_list
    {
      constexpr static auto type_name = std::string_view{ "TS::MaterialList" };
      constexpr static auto version = 2;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header", "materials" });

      v2::header header;
      std::vector<material> materials;
    };
  }// namespace material_list::v2

  namespace material_list::v3
  {
    export struct material
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "flags", "alpha", "index", "rgbData", "fileName", "type", "elasticity", "friction" });

      endian::little_int32_t flags;
      float alpha;
      endian::little_int32_t index;
      content::rgb_data rgb_data;

      std::array<char, 32> file_name;

      endian::little_int32_t type;
      float elasticity;
      float friction;
    };

    export struct material_list
    {
      constexpr static auto type_name = v2::material_list::type_name;
      constexpr static auto version = 3;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header", "materials" });

      v2::header header;
      std::vector<material> materials;
    };
  }// namespace material_list::v3

  namespace material_list::v4
  {
    export struct material
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "flags", "alpha", "index", "rgbData", "fileName", "type", "elasticity", "friction", "useDefaultProperties" });

      endian::little_int32_t flags;
      float alpha;
      endian::little_int32_t index;
      content::rgb_data rgb_data;

      std::array<char, 32> file_name;

      endian::little_int32_t type;
      float elasticity;
      float friction;
      endian::little_uint32_t use_default_properties;
    };

    export struct material_list
    {
      constexpr static auto type_name = v2::material_list::type_name;
      constexpr static auto version = 4;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header", "materials" });

      v2::header header;
      std::vector<material> materials;
    };

  }// namespace material_list::v4

  export using mesh_variant = std::variant<mesh::v1::mesh, mesh::v2::mesh, mesh::v3::mesh>;

  export using material_list_variant = std::variant<material_list::v2::material_list, material_list::v3::material_list, material_list::v4::material_list>;

  namespace shape::v2
  {
    export struct header
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "numNodes",
        "numSequences",
        "numSubSequences",
        "numKeyFrames",
        "numTransforms",
        "numNames",
        "numObjects",
        "numDetails",
        "numMeshes",
        "numTransitions" });

      endian::little_int32_t num_nodes;
      endian::little_int32_t num_sequences;
      endian::little_int32_t num_sub_sequences;
      endian::little_int32_t num_key_frames;
      endian::little_int32_t num_transforms;
      endian::little_int32_t num_names;
      endian::little_int32_t num_objects;
      endian::little_int32_t num_details;
      endian::little_int32_t num_meshes;
      endian::little_int32_t num_transitions;
    };

    export struct data
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "radius", "centre" });
      float radius;
      vector3f centre;
    };

    static_assert(sizeof(data) == sizeof(std::array<float, 4>));

    export struct node
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "nameIndex", "parentNodeIndex", "numSubSequences", "firstSubSequenceIndex", "defaultTransformIndex" });
      endian::little_int32_t name_index;
      endian::little_int32_t parent_node_index;
      endian::little_int32_t num_sub_sequences;
      endian::little_int32_t first_sub_sequence_index;
      endian::little_int32_t default_transform_index;
    };

    export struct sequence
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "nameIndex",
        "cyclic",
        "duration",
        "priority" });
      endian::little_int32_t name_index;
      endian::little_int32_t cyclic;
      float duration;
      endian::little_int32_t priority;
    };

    export struct sub_sequence
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "sequenceIndex", "numKeyFrames", "firstKeyFrameIndex" });
      endian::little_int32_t sequence_index;
      endian::little_int32_t num_key_frames;
      endian::little_int32_t first_key_frame_index;
    };

    export struct keyframe
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "position", "transformIndex" });
      float position;
      endian::little_uint32_t transform_index;
    };

    export struct transform
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "rotation", "translation", "scale" });
      quaternion4f rotation;
      vector3f translation;
      vector3f scale;
    };

    export using name = std::array<char, 24>;

    export struct object
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "nameIndex", "flags", "meshIndex", "nodeIndex", "depFlags", "dep", "objectOffset", "numSubSequences", "firstSubSequenceIndex" });
      endian::little_int16_t name_index;
      endian::little_int16_t flags;
      endian::little_int32_t mesh_index;
      endian::little_int32_t node_index;
      endian::little_int32_t dep_flags;
      std::array<vector3f, 3> dep;
      vector3f object_offset;
      endian::little_int32_t num_sub_sequences;
      endian::little_int32_t first_sub_sequence_index;
    };

    export struct detail
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "rootNodeIndex", "size" });
      endian::little_int32_t root_node_index;
      float size;
    };

    export struct transition
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "startSequenceIndex",
        "endSequenceIndex",
        "startPosition",
        "endPosition",
        "duration",
        "transform" });
      endian::little_int32_t start_sequence_index;
      endian::little_int32_t end_sequence_index;
      float start_position;
      float end_position;
      float duration;
      v2::transform transform;
    };

    // TODO Put this into the actual shape for 100% coverage in the JSON file.
    export using has_material_list_flag = endian::little_int32_t;

    export struct shape
    {
      constexpr static auto type_name = std::string_view{ "TS::Shape" };
      constexpr static auto version = 2;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header",
        "data",
        "nodes",
        "sequences",
        "subSequences",
        "keyframes",
        "transforms",
        "names",
        "objects",
        "details",
        "transitions",
        "meshes",
        "materialList" });

      v2::header header;
      v2::data data;
      std::vector<node> nodes;
      std::vector<sequence> sequences;
      std::vector<sub_sequence> sub_sequences;
      std::vector<keyframe> keyframes;
      std::vector<transform> transforms;
      std::vector<name> names;
      std::vector<object> objects;
      std::vector<detail> details;
      std::vector<transition> transitions;
      std::vector<mesh_variant> meshes;

      material_list_variant material_list;
    };
  }// namespace shape::v2

  namespace shape::v3
  {
    export struct keyframe
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "position", "transformIndex", "matIndex" });
      float position;
      endian::little_uint32_t transform_index;
      endian::little_uint32_t mat_index;
    };

    export struct shape
    {
      constexpr static auto type_name = v2::shape::type_name;
      constexpr static auto version = 3;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header",
        "data",
        "nodes",
        "sequences",
        "subSequences",
        "keyframes",
        "transforms",
        "names",
        "objects",
        "details",
        "transitions",
        "meshes",
        "materialList" });

      v2::header header;
      v2::data data;
      std::vector<v2::node> nodes;
      std::vector<v2::sequence> sequences;
      std::vector<v2::sub_sequence> sub_sequences;
      std::vector<keyframe> keyframes;
      std::vector<v2::transform> transforms;
      std::vector<v2::name> names;
      std::vector<v2::object> objects;
      std::vector<v2::detail> details;
      std::vector<v2::transition> transitions;
      std::vector<mesh_variant> meshes;

      material_list_variant material_list;
    };
  }// namespace shape::v3

  namespace shape::v5
  {
    export struct header
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "numNodes",
        "numSequences",
        "numSubSequences",
        "numKeyFrames",
        "numTransforms",
        "numNames",
        "numObjects",
        "numDetails",
        "numMeshes",
        "numTransitions",
        "numFrameTriggers" });

      endian::little_int32_t num_nodes;
      endian::little_int32_t num_sequences;
      endian::little_int32_t num_sub_sequences;
      endian::little_int32_t num_key_frames;
      endian::little_int32_t num_transforms;
      endian::little_int32_t num_names;
      endian::little_int32_t num_objects;
      endian::little_int32_t num_details;
      endian::little_int32_t num_meshes;
      endian::little_int32_t num_transitions;
      endian::little_int32_t num_frame_triggers;
    };

    export struct sequence
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "nameIndex",
        "cyclic",
        "duration",
        "priority",
        "firstFrameTriggerIndex",
        "numFrameTriggers",
        "numIflSubSequences",
        "firstIflSubSequenceIndex" });
      endian::little_int32_t name_index;
      endian::little_int32_t cyclic;
      float duration;
      endian::little_int32_t priority;
      endian::little_int32_t first_frame_trigger_index;
      endian::little_int32_t num_frame_triggers;
      endian::little_int32_t num_ifl_sub_sequences;
      endian::little_int32_t first_ifl_sub_sequence_index;
    };

    export struct frame_trigger
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "position",
        "value" });
      float position;
      float value;
    };

    export struct footer
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "numDefaultMaterials" });
      endian::little_int32_t num_default_materials;
    };

    export struct shape
    {
      constexpr static auto type_name = v2::shape::type_name;
      constexpr static auto version = 5;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header",
        "data",
        "nodes",
        "sequences",
        "subSequences",
        "keyframes",
        "transforms",
        "names",
        "objects",
        "details",
        "transitions",
        "frameTriggers",
        "footer",
        "meshes",
        "materialList" });

      v5::header header;
      v2::data data;
      std::vector<v2::node> nodes;
      std::vector<sequence> sequences;
      std::vector<v2::sub_sequence> sub_sequences;
      std::vector<v3::keyframe> keyframes;
      std::vector<v2::transform> transforms;
      std::vector<v2::name> names;
      std::vector<v2::object> objects;
      std::vector<v2::detail> details;
      std::vector<v2::transition> transitions;
      std::vector<frame_trigger> frame_triggers;
      v5::footer footer;
      std::vector<mesh_variant> meshes;

      material_list_variant material_list;
    };
  }// namespace shape::v5


  namespace shape::v6
  {
    export struct footer
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "numDefaultMaterials",
        "alwaysNode" });
      endian::little_int32_t num_default_materials;
      endian::little_int32_t always_node;
    };

    export struct shape
    {
      constexpr static auto type_name = v2::shape::type_name;
      constexpr static auto version = 6;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header",
        "data",
        "nodes",
        "sequences",
        "subSequences",
        "keyframes",
        "transforms",
        "names",
        "objects",
        "details",
        "transitions",
        "frameTriggers",
        "footer",
        "meshes",
        "materialList" });

      v5::header header;
      v2::data data;
      std::vector<v2::node> nodes;
      std::vector<v5::sequence> sequences;
      std::vector<v2::sub_sequence> sub_sequences;
      std::vector<v3::keyframe> keyframes;
      std::vector<v2::transform> transforms;
      std::vector<v2::name> names;
      std::vector<v2::object> objects;
      std::vector<v2::detail> details;
      std::vector<v2::transition> transitions;
      std::vector<v5::frame_trigger> frame_triggers;
      v6::footer footer;
      std::vector<mesh_variant> meshes;

      material_list_variant material_list;
    };
  }// namespace shape::v6

  namespace shape::v7
  {
    export struct transform
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "rotation", "translation", "scale" });
      quaternion4s rotation;
      vector3f translation;
      vector3f scale;
    };

    static_assert(sizeof(transform) == sizeof(std::array<std::int32_t, 8>));

    export struct transition
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "startSequenceIndex",
        "endSequenceIndex",
        "startPosition",
        "endPosition",
        "duration",
        "rotation",
        "translation",
        "scale" });
      endian::little_int32_t start_sequence_index;
      endian::little_int32_t end_sequence_index;
      float start_position;
      float end_position;
      float duration;
      quaternion4s rotation;
      vector3f translation;
      vector3f scale;
    };

    export struct shape
    {
      constexpr static auto type_name = v2::shape::type_name;
      constexpr static auto version = 7;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header",
        "data",
        "nodes",
        "sequences",
        "subSequences",
        "keyframes",
        "transforms",
        "names",
        "objects",
        "details",
        "transitions",
        "frameTriggers",
        "footer",
        "meshes",
        "materialList" });

      v5::header header;
      v2::data data;
      std::vector<v2::node> nodes;
      std::vector<v5::sequence> sequences;
      std::vector<v2::sub_sequence> sub_sequences;
      std::vector<v3::keyframe> keyframes;
      std::vector<transform> transforms;
      std::vector<v2::name> names;
      std::vector<v2::object> objects;
      std::vector<v2::detail> details;
      std::vector<transition> transitions;
      std::vector<v5::frame_trigger> frame_triggers;
      v6::footer footer;
      std::vector<mesh_variant> meshes;

      material_list_variant material_list;
    };
  }// namespace shape::v7

  namespace shape::v8
  {
    export struct data
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "radius", "centre", "bounds" });
      float radius;
      vector3f centre;
      vector3f_pair bounds;
    };

    static_assert(sizeof(data) == sizeof(std::array<float, 10>));

    export struct node
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "nameIndex", "parentNodeIndex", "numSubSequences", "firstSubSequenceIndex", "defaultTransformIndex" });
      endian::little_int16_t name_index;
      endian::little_int16_t parent_node_index;
      endian::little_int16_t num_sub_sequences;
      endian::little_int16_t first_sub_sequence_index;
      endian::little_int16_t default_transform_index;
    };

    export struct sub_sequence
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "sequenceIndex", "numKeyFrames", "firstKeyFrameIndex" });
      endian::little_int16_t sequence_index;
      endian::little_int16_t num_key_frames;
      endian::little_int16_t first_key_frame_index;
    };


    export struct keyframe
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "position", "transformIndex", "matIndex" });
      float position;
      endian::little_uint16_t transform_index;
      endian::little_uint16_t mat_index;
    };

    export struct transform
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "rotation", "translation" });
      quaternion4s rotation;
      vector3f translation;
    };

    export struct object
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "nameIndex", "flags", "meshIndex", "nodeIndex", "objectOffset", "numSubSequences", "firstSubSequenceIndex" });
      endian::little_int16_t name_index;
      endian::little_int16_t flags;
      endian::little_int32_t mesh_index;
      endian::little_int16_t node_index;
      vector3f object_offset;
      endian::little_int16_t num_sub_sequences;
      endian::little_int16_t first_sub_sequence_index;
    };

    export struct transition
    {
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "startSequence",
        "endSequence",
        "startPosition",
        "endPosition",
        "duration",
        "transformation" });
      endian::little_int32_t start_sequence;
      endian::little_int32_t end_sequence;
      float start_position;
      float end_position;
      float duration;
      transform transformation;
    };

    export struct shape
    {
      constexpr static auto type_name = v2::shape::type_name;
      constexpr static auto version = 8;
      KEYS_CONSTEXPR static auto keys = platform::make_keys({ "header",
        "data",
        "nodes",
        "sequences",
        "subSequences",
        "keyframes",
        "transforms",
        "names",
        "objects",
        "details",
        "transitions",
        "frameTriggers",
        "footer",
        "meshes",
        "materialList" });

      v5::header header;
      v8::data data;
      std::vector<node> nodes;
      std::vector<v5::sequence> sequences;
      std::vector<sub_sequence> sub_sequences;
      std::vector<keyframe> keyframes;
      std::vector<transform> transforms;
      std::vector<v2::name> names;
      std::vector<v8::object> objects;
      std::vector<v2::detail> details;
      std::vector<v8::transition> transitions;
      std::vector<v5::frame_trigger> frame_triggers;
      v6::footer footer;
      std::vector<mesh_variant> meshes;

      material_list_variant material_list;
    };
  }// namespace shape::v8

  export using shape_variant = std::variant<shape::v2::shape, shape::v3::shape, shape::v5::shape, shape::v6::shape, shape::v7::shape, shape::v8::shape>;

  export using shape_or_material_list = std::variant<material_list_variant, shape_variant>;

  template<std::size_t size>
  std::array<std::byte, size> read(std::istream& stream)
  {
    std::array<std::byte, size> dest{};

    stream.read(reinterpret_cast<char*>(&dest[0]), size);

    return dest;
  }

  template<typename destination_type>
  destination_type read(std::istream& stream)
  {
    destination_type dest{};

    stream.read(reinterpret_cast<char*>(&dest), sizeof(destination_type));

    return dest;
  }

  template<typename destination_type>
  std::vector<destination_type> read_vector(std::istream& stream, std::size_t size)
  {
    if (size == 0)
    {
      return {};
    }

    std::vector<destination_type> dest(size);

    stream.read(reinterpret_cast<char*>(&dest[0]), sizeof(destination_type) * size);

    return dest;
  }

  inline std::string read_string(std::istream& stream, std::size_t size, std::size_t max_size = 16)
  {
    std::string dest(size, '\0');

    stream.read(reinterpret_cast<char*>(&dest[0]), size);

    // There is always an embedded \0 in the
    // file if the string length is less than 16 bytes.
    if (size < max_size)
    {
      stream.seekg(1, std::ios_base::cur);
    }

    return dest;
  }

  template<std::size_t Size>
  void write(std::ostream& stream, const std::array<std::byte, Size>& value)
  {
    stream.write(reinterpret_cast<const char*>(value.data()), Size);
  }

  template<typename ValueType>
  void write(std::ostream& stream, const ValueType& value)
  {
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
  }

  template<typename ValueType>
  void write(std::ostream& stream, const std::vector<ValueType>& values)
  {
    stream.write(reinterpret_cast<const char*>(values.data()), values.size() * sizeof(ValueType));
  }

  template<typename ValueType>
  void write(std::ostream& stream, const ValueType* value, std::size_t size)
  {
    stream.write(reinterpret_cast<const char*>(value), size);
  }

  export bool is_darkstar_dts(std::istream& stream)
  {
    auto starting_point = stream.tellg();

    tag_header file_header = {
      read<sizeof(file_tag)>(stream),
      read<file_info>(stream)
    };

    if (file_header.tag != pers_tag)
    {
      stream.seekg(starting_point, std::ios::beg);
      return false;
    }

    file_header.class_name = read_string(stream, file_header.file_info.class_name_length);

    stream.seekg(starting_point, std::ios::beg);

    return file_header.class_name == shape::v2::shape::type_name;
  }

  export bool is_darkstar_dml(std::istream& stream)
  {
    auto starting_point = stream.tellg();

    tag_header file_header = {
      read<sizeof(file_tag)>(stream),
      read<file_info>(stream)
    };

    if (file_header.tag != pers_tag)
    {
      stream.seekg(starting_point, std::ios::beg);
      return false;
    }

    file_header.class_name = read_string(stream, file_header.file_info.class_name_length);

    stream.seekg(starting_point, std::ios::beg);

    return file_header.class_name == material_list::v2::material_list::type_name;
  }

  tag_header read_object_header(std::istream& stream)
  {
    tag_header file_header = {
      read<sizeof(file_tag)>(stream),
      read<file_info>(stream)
    };

    if (file_header.tag != pers_tag)
    {
      std::stringstream msg;
      msg << "There was an error trying to parse a portion of the DTS/DML file at byte number " << stream.tellg() << ". ";
      msg << "Expected the " << std::string(reinterpret_cast<const char*>(pers_tag.data()), pers_tag.size()) << " file header to be present but it was not found.\n";

      throw std::invalid_argument(msg.str());
    }

    file_header.class_name = read_string(stream, file_header.file_info.class_name_length);
    file_header.version = read<version>(stream);

    return file_header;
  }

  template<typename ShapeType>
  void read_meshes(ShapeType& shape, std::size_t num_meshes, std::istream& stream)
  {
    using namespace mesh;
    shape.meshes.reserve(num_meshes);

    for (auto i = 0u; i < num_meshes; ++i)
    {
      auto mesh_tag_header = read_object_header(stream);

      if (mesh_tag_header.class_name != v1::mesh::type_name)
      {
        throw std::invalid_argument("The object provided is not a mesh as expected.");
      }

      if (mesh_tag_header.version > 3)
      {
        throw std::invalid_argument("The mesh version was not version 1, 2 or 3 as expected.");
      }

      if (mesh_tag_header.version == 1)
      {
        auto mesh_header = read<v1::header>(stream);

        v1::mesh mesh{
          mesh_header,
          read_vector<v1::vertex>(stream, mesh_header.num_verts),
          read_vector<v1::texture_vertex>(stream, mesh_header.num_texture_verts),
          read_vector<v1::face>(stream, mesh_header.num_faces),
          read_vector<v1::frame>(stream, mesh_header.num_frames)
        };

        shape.meshes.push_back(mesh);
      }

      if (mesh_tag_header.version == 2)
      {
        auto mesh_header = read<v2::header>(stream);

        v2::mesh mesh{
          mesh_header,
          read_vector<v1::vertex>(stream, mesh_header.num_verts),
          read_vector<v1::texture_vertex>(stream, mesh_header.num_texture_verts),
          read_vector<v1::face>(stream, mesh_header.num_faces),
          read_vector<v1::frame>(stream, mesh_header.num_frames)
        };

        shape.meshes.push_back(mesh);
      }

      if (mesh_tag_header.version == 3)
      {
        auto mesh_header = read<v3::header>(stream);

        v3::mesh mesh{
          mesh_header,
          read_vector<v1::vertex>(stream, mesh_header.num_verts),
          read_vector<v1::texture_vertex>(stream, mesh_header.num_texture_verts),
          read_vector<v1::face>(stream, mesh_header.num_faces),
          read_vector<v3::frame>(stream, mesh_header.num_frames)
        };

        shape.meshes.push_back(mesh);
      }
    }
  }

  material_list_variant read_material_list(const tag_header& object_header, std::istream& stream)
  {
    using namespace material_list;
    if (object_header.class_name != v2::material_list::type_name)
    {
      throw std::invalid_argument("The object was not a material list as expected.");
    }

    if (object_header.version == 2)
    {
      auto main_header = read<v2::header>(stream);

      return v2::material_list{
        main_header,
        read_vector<v2::material>(stream, main_header.num_materials.value() * main_header.num_details)
      };
    }

    if (object_header.version == 3)
    {
      auto main_header = read<v2::header>(stream);

      return v3::material_list{
        main_header,
        read_vector<v3::material>(stream, main_header.num_materials.value() * main_header.num_details)
      };
    }

    if (object_header.version == 4)
    {
      auto main_header = read<v2::header>(stream);

      return v4::material_list{
        main_header,
        read_vector<v4::material>(stream, main_header.num_materials.value() * main_header.num_details)
      };
    }

    throw std::invalid_argument("The material list version provided is not supported: " + to_string(object_header.version));
  }

  template<typename ShapeType>
  void read_materials(ShapeType& shape, std::istream& stream)
  {
    if (auto has_material_list = read<shape::v2::has_material_list_flag>(stream); has_material_list == 1)
    {
      auto object_header = read_object_header(stream);

      shape.material_list = read_material_list(object_header, stream);
    }
  }

  template<typename ShapeType>
  ShapeType read_shape_impl(std::istream& stream)
  {
    auto header = read<decltype(ShapeType::header)>(stream);
    ShapeType shape{
      header,
      read<decltype(ShapeType::data)>(stream),
      read_vector<typename decltype(ShapeType::nodes)::value_type>(stream, header.num_nodes),
      read_vector<typename decltype(ShapeType::sequences)::value_type>(stream, header.num_sequences),
      read_vector<typename decltype(ShapeType::sub_sequences)::value_type>(stream, header.num_sub_sequences),
      read_vector<typename decltype(ShapeType::keyframes)::value_type>(stream, header.num_key_frames),
      read_vector<typename decltype(ShapeType::transforms)::value_type>(stream, header.num_transforms),
      read_vector<typename decltype(ShapeType::names)::value_type>(stream, header.num_names),
      read_vector<typename decltype(ShapeType::objects)::value_type>(stream, header.num_objects),
      read_vector<typename decltype(ShapeType::details)::value_type>(stream, header.num_details),
      read_vector<typename decltype(ShapeType::transitions)::value_type>(stream, header.num_transitions),
    };

    if constexpr (ShapeType::version > 3)
    {
      shape.frame_triggers = read_vector<typename decltype(ShapeType::frame_triggers)::value_type>(stream, header.num_frame_triggers);
      shape.footer = read<decltype(ShapeType::footer)>(stream);
    }

    read_meshes(shape, header.num_meshes, stream);

    read_materials(shape, stream);

    return shape;
  }


  export shape_variant read_shape(std::istream& stream, std::optional<tag_header> file_header)
  {
    using namespace shape;
    file_header = !file_header.has_value() ? read_object_header(stream) : file_header;

    if (file_header->class_name != v2::shape::type_name)
    {
      throw std::invalid_argument("The object provided is not a shape as expected.");
    }

    if (file_header->version > 8)
    {
      throw std::invalid_argument("The shape is not supported.");
    }

    if (file_header->version == 2)
    {
      return read_shape_impl<v2::shape>(stream);
    }
    else if (file_header->version == 3)
    {
      return read_shape_impl<v3::shape>(stream);
    }
    else if (file_header->version == 5)
    {
      return read_shape_impl<v5::shape>(stream);
    }
    else if (file_header->version == 6)
    {
      return read_shape_impl<v6::shape>(stream);
    }
    else if (file_header->version == 7)
    {
      return read_shape_impl<v7::shape>(stream);
    }
    else if (file_header->version == 8)
    {
      return read_shape_impl<v8::shape>(stream);
    }
    else
    {
      std::stringstream error;
      error << "File is DTS version " << file_header->version << ", which is currently unsupported.";
      throw std::invalid_argument(error.str());
    }
  }


  export shape_or_material_list read_shape(std::istream& stream)
  {
    using namespace shape;
    tag_header file_header = read_object_header(stream);

    if (file_header.class_name == material_list::v2::material_list::type_name)
    {
      return read_material_list(file_header, stream);
    }

    return read_shape(stream, file_header);
  }

  template<typename RootType>
  void write_header(std::ostream& stream, const RootType& root)
  {
    constexpr static auto empty = std::byte{ '\0' };
    siege::platform::little_uint32_t size_in_bytes{};
    write(stream, pers_tag);
    // This is a placeholder for the real size which will come later
    write(stream, size_in_bytes);

    constexpr std::string_view type_name = std::remove_reference_t<decltype(root)>::type_name;
    const siege::platform::little_int16_t type_size = static_cast<std::uint16_t>(type_name.size());

    write(stream, type_size);
    write(stream, type_name.data(), type_size);

    if constexpr (type_name.size() < 16)
    {
      write(stream, empty);
    }

    const siege::platform::little_int32_t version = std::remove_reference_t<decltype(root)>::version;
    write(stream, version);
  }

  void write_size(std::ostream& stream, std::optional<std::uint32_t> start_offset = std::nullopt)
  {
    siege::platform::little_uint32_t size_in_bytes{};

    start_offset = start_offset.has_value() ? start_offset.value() + static_cast<std::uint32_t>(pers_tag.size()) : static_cast<std::uint32_t>(pers_tag.size());
    std::uint32_t end_offset = static_cast<std::uint32_t>(stream.tellp());

    // sort out the size we reserved earlier
    size_in_bytes = end_offset - start_offset.value() - sizeof(size_in_bytes);
    stream.seekp(start_offset.value(), std::ios_base::beg);

    write(stream, size_in_bytes);

    stream.seekp(end_offset, std::ios_base::beg);
  }

  export void write_material_list(std::ostream& stream, const material_list_variant& list)
  {
    std::visit([&](const auto& materials) {
      write_header(stream, materials);
      write(stream, materials.header);
      write(stream, materials.materials);
      write_size(stream);
    },
      list);
  }

  export void write_shape(std::ostream& stream, const shape_variant& fresh_shape)
  {
    std::visit([&](const auto& shape) {
      write_header(stream, shape);
      write(stream, shape.header);
      write(stream, shape.data);
      write(stream, shape.nodes);
      write(stream, shape.sequences);
      write(stream, shape.sub_sequences);
      write(stream, shape.keyframes);
      write(stream, shape.transforms);
      write(stream, shape.names);
      write(stream, shape.objects);
      write(stream, shape.details);
      write(stream, shape.transitions);

      if constexpr (std::remove_reference_t<decltype(shape)>::version > 3)
      {
        write(stream, shape.frame_triggers);
        write(stream, shape.footer);
      }

      for (const auto& mesh_var : shape.meshes)
      {
        const auto start_offset = static_cast<std::uint32_t>(stream.tellp());

        std::visit([&](const auto& mesh) {
          write_header(stream, mesh);
          write(stream, mesh.header);
          write(stream, mesh.vertices);
          write(stream, mesh.texture_vertices);
          write(stream, mesh.faces);
          write(stream, mesh.frames);
          write_size(stream, start_offset);
        },
          mesh_var);
      }

      const siege::platform::little_uint32_t has_materials = 1u;
      write(stream, has_materials);

      const auto start_offset = static_cast<std::uint32_t>(stream.tellp());
      std::visit([&](const auto& materials) {
        write_header(stream, materials);
        write(stream, materials.header);
        write(stream, materials.materials);
        write_size(stream, start_offset);
      },
        shape.material_list);

      write_size(stream);
    },
      fresh_shape);
  }
}// namespace siege::content::dts::darkstar
