#include <array>
#include <utility>
#include <vector>
#include "shared.hpp"
#include "endian_arithmetic.hpp"
#include "content/tagged_data.hpp"
#include "content/dts/3space_v2.hpp"

namespace studio::content::dts::three_space
{
  namespace endian = studio::endian;
  using file_tag = std::array<std::byte, 4>;

  namespace v2
  {
    constexpr file_tag material_list_tag = shared::to_tag<4>({ 0x1e, 0x00, 0x14, 0x00 });
    constexpr file_tag material_tag = shared::to_tag<4>({ 0x1f, 0x00, 0x14, 0x00 });
    constexpr file_tag bsp_part_tag = shared::to_tag<4>({ 0x46, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_bsp_part_tag = shared::to_tag<4>({ 0x47, 0x00, 0x14, 0x00 });
    constexpr file_tag cell_anim_part_tag = shared::to_tag<4>({ 0x51, 0x00, 0x14, 0x00 });
    constexpr file_tag detail_part_tag = shared::to_tag<4>({ 0x5a, 0x00, 0x14, 0x00 });
    constexpr file_tag mesh_tag = shared::to_tag<4>({ 0x28, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_mesh_tag = shared::to_tag<4>({ 0x2b, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_anim_mesh_tag = shared::to_tag<4>({ 0x2a, 0x00, 0x14, 0x00 });
    constexpr file_tag part_list_tag = shared::to_tag<4>({ 0x3c, 0x00, 0x14, 0x00 });
    constexpr file_tag shape_tag = shared::to_tag<4>({ 0x64, 0x00, 0x14, 0x00 });
    constexpr file_tag nu_shape_tag = shared::to_tag<4>({ 0x65, 0x00, 0x14, 0x00 });
    constexpr file_tag null_part_tag = shared::to_tag<4>({ 0x78, 0x00, 0x14, 0x00 });
    constexpr file_tag bitmap_frame_tag = shared::to_tag<4>({ 0x6e, 0x00, 0x14, 0x00 });

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

      const auto total_materials = shape.material_count * shape.detail_count;

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
}// namespace studio::content::dts::three_space
