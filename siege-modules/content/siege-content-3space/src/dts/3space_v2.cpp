// DTS - Dynamix ThreeSpace Shape v2 for 3Space v2.5 (A-10 Tank Killer 2, Red Baron 2, Pro Pilot)
import std;
import siege.platform.endian;
import siege.platform.shared;
import siege.platform.tagged_data;

namespace siege::content::dts::three_space
{
  namespace endian = siege::platform;
  using file_tag = std::array<std::byte, 4>;

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

    using shape_item = std::variant<raw_item, null_part, base_part, part_list, bsp_part, cell_anim_part, detail_part, bitmap_frame_part, mesh, shape, nu_shape, nu_bsp_part, nu_anim_mesh, nu_anim_mesh>;

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

    constexpr file_tag material_list_tag = platform::to_tag<4>({ 0x1e, 0x00, 0x14, 0x00 });
    constexpr file_tag material_tag = platform::to_tag<4>({ 0x1f, 0x00, 0x14, 0x00 });
    constexpr file_tag bsp_part_tag = platform::to_tag<4>({ 0x46, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_bsp_part_tag = platform::to_tag<4>({ 0x47, 0x00, 0x14, 0x00 });
    constexpr file_tag cell_anim_part_tag = platform::to_tag<4>({ 0x51, 0x00, 0x14, 0x00 });
    constexpr file_tag detail_part_tag = platform::to_tag<4>({ 0x5a, 0x00, 0x14, 0x00 });
    constexpr file_tag mesh_tag = platform::to_tag<4>({ 0x28, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_mesh_tag = platform::to_tag<4>({ 0x2b, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_anim_mesh_tag = platform::to_tag<4>({ 0x2a, 0x00, 0x14, 0x00 });
    constexpr file_tag part_list_tag = platform::to_tag<4>({ 0x3c, 0x00, 0x14, 0x00 });
    constexpr file_tag shape_tag = platform::to_tag<4>({ 0x64, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_shape_tag = platform::to_tag<4>({ 0x65, 0x00, 0x14, 0x00 });
    constexpr file_tag null_part_tag = platform::to_tag<4>({ 0x78, 0x00, 0x14, 0x00 });
    constexpr file_tag bitmap_frame_tag = platform::to_tag<4>({ 0x6e, 0x00, 0x14, 0x00 });

    std::vector<shape_item> read_shapes(std::istream& file)
    {
      static shape_reader_map readers = {};

      return read_children<shape_item>(file, 1, readers);
    }

    base_part read_base_part(std::istream& file, object_header& header, shape_reader_map&)
    {
      base_part base{};
      file.read(reinterpret_cast<char*>(&base.transform), sizeof(base.transform));
      file.read(reinterpret_cast<char*>(&base.id_number), sizeof(base.id_number));
      file.read(reinterpret_cast<char*>(&base.radius), sizeof(base.radius));
      file.read(reinterpret_cast<char*>(&base.center), sizeof(base.center));

      return base;
    }

    null_part read_null_part(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      null_part shape{};
      shape.base = read_base_part(file, header, readers);

      return shape;
    }

    part_list read_part_list(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      part_list parts{};
      parts.base = read_base_part(file, header, readers);
      file.read(reinterpret_cast<char*>(&parts.part_count), sizeof(parts.part_count));
      parts.parts = read_children<shape_item>(file, parts.part_count, readers);
      return parts;
    }

    material read_material(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      material shape{};

      file.read(reinterpret_cast<char*>(&shape.null), sizeof(shape.null));
      file.read(reinterpret_cast<char*>(&shape.unknown0), sizeof(shape.unknown0));
      file.read(reinterpret_cast<char*>(&shape.material_type), sizeof(shape.material_type));
      file.read(reinterpret_cast<char*>(&shape.unknown1), sizeof(shape.unknown1));
      file.read(reinterpret_cast<char*>(&shape.palette), sizeof(shape.palette));
      file.read(reinterpret_cast<char*>(&shape.rgb), sizeof(shape.rgb));
      file.read(reinterpret_cast<char*>(&shape.flags), sizeof(shape.flags));
      shape.texture = read_string(file);

      return shape;
    }

    material_list read_material_list(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      material_list shape{};

      file.read(reinterpret_cast<char*>(&shape.material_count), sizeof(shape.material_count));
      file.read(reinterpret_cast<char*>(&shape.detail_count), sizeof(shape.detail_count));

      const auto total_materials = shape.material_count.value() * shape.detail_count;

      for (auto i = 0u; i < total_materials; ++i)
      {
        object_header child_header;
        file.read(reinterpret_cast<char*>(&child_header), sizeof(child_header));
        shape.materials.emplace_back(read_material(file, child_header, readers));
      }

      return shape;
    }

    mesh read_mesh(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      mesh shape{};
      shape.base = read_base_part(file, header, readers);

      file.read(reinterpret_cast<char*>(&shape.vertex_count), sizeof(shape.vertex_count));
      file.read(reinterpret_cast<char*>(&shape.texture_vertex_count), sizeof(shape.texture_vertex_count));
      file.read(reinterpret_cast<char*>(&shape.face_count), sizeof(shape.face_count));

      shape.vertices = std::vector<mesh::vertex>(shape.vertex_count);
      file.read(reinterpret_cast<char*>(shape.vertices.data()), sizeof(mesh::vertex) * shape.vertex_count);

      using tx_vertex = decltype(shape.texture_vertices)::value_type;
      shape.texture_vertices = std::vector<tx_vertex>(shape.texture_vertex_count);
      file.read(reinterpret_cast<char*>(shape.texture_vertices.data()), sizeof(tx_vertex) * shape.texture_vertex_count);

      shape.faces = std::vector<mesh::face>(shape.face_count);
      file.read(reinterpret_cast<char*>(shape.faces.data()), sizeof(mesh::face) * shape.face_count);

      return shape;
    }

    cell_anim_part read_cell_anim_part(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      cell_anim_part shape{};
      shape.base = read_part_list(file, header, readers);

      file.read(reinterpret_cast<char*>(&shape.anim_sequence), sizeof(shape.anim_sequence));
      file.read(reinterpret_cast<char*>(&shape.cell_count), sizeof(shape.cell_count));

      file.read(reinterpret_cast<char*>(&shape.cells), sizeof(shape.cell_count));
      shape.cells = std::vector<endian::little_int32_t>(shape.cell_count);
      file.read(reinterpret_cast<char*>(shape.cells.data()), sizeof(endian::little_int32_t) * shape.cell_count);

      return shape;
    }

    bitmap_frame_part read_bitmap_frame_part(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      bitmap_frame_part shape{};
      shape.base = read_base_part(file, header, readers);

      file.read(reinterpret_cast<char*>(&shape.bmp_tag), sizeof(shape.bmp_tag));
      file.read(reinterpret_cast<char*>(&shape.x_offset), sizeof(shape.x_offset));
      file.read(reinterpret_cast<char*>(&shape.y_offset), sizeof(shape.y_offset));
      file.read(reinterpret_cast<char*>(&shape.rotation), sizeof(shape.rotation));
      file.read(reinterpret_cast<char*>(&shape.squish), sizeof(shape.squish));

      return shape;
    }

    detail_part read_detail_part(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      detail_part shape{};
      shape.base = read_part_list(file, header, readers);
      file.read(reinterpret_cast<char*>(&shape.detail_count), sizeof(shape.detail_count));

      shape.details = std::vector<endian::little_int32_t>(shape.detail_count);
      file.read(reinterpret_cast<char*>(shape.details.data()), sizeof(endian::little_int32_t) * shape.detail_count);

      return shape;
    }

    bsp_part read_bsp_part(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      bsp_part shape{};
      shape.base = read_part_list(file, header, readers);

      file.read(reinterpret_cast<char*>(&shape.node_count), sizeof(shape.node_count));

      shape.nodes = std::vector<bsp_part::bsp_node>(shape.node_count);
      file.read(reinterpret_cast<char*>(shape.nodes.data()), sizeof(bsp_part::bsp_node) * shape.node_count);

      shape.transforms = std::vector<endian::little_int32_t>(shape.node_count);
      file.read(reinterpret_cast<char*>(shape.transforms.data()), sizeof(endian::little_int32_t) * shape.node_count);

      return shape;
    }

    shape read_shape(std::istream& file, object_header& header, shape_reader_map& readers)
    {
      shape shape{};
      shape.base = read_part_list(file, header, readers);

      file.read(reinterpret_cast<char*>(&shape.sequence_count), sizeof(shape.sequence_count));
      shape.sequences = std::vector<endian::little_int32_t>(shape.sequence_count);
      file.read(reinterpret_cast<char*>(shape.sequences.data()), sizeof(endian::little_int32_t) * shape.sequence_count);
      return shape;
    }
  }// namespace v2
}// namespace siege::content::dts::three_space
