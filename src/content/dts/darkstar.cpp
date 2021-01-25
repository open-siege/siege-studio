#include "darkstar.hpp"
#include "content/binary_io.hpp"

namespace studio::content::dts::darkstar
{
  bool is_darkstar_dts(std::basic_istream<std::byte>& stream)
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

  tag_header read_object_header(std::basic_istream<std::byte>& stream)
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
  void read_meshes(ShapeType& shape, std::size_t num_meshes, std::basic_istream<std::byte>& stream)
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

  material_list_variant read_material_list(const tag_header& object_header, std::basic_istream<std::byte>& stream)
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
        read_vector<v2::material>(stream, main_header.num_materials * main_header.num_details)
      };
    }

    if (object_header.version == 3)
    {
      auto main_header = read<v2::header>(stream);

      return v3::material_list{
        main_header,
        read_vector<v3::material>(stream, main_header.num_materials * main_header.num_details)
      };
    }

    if (object_header.version == 4)
    {
      auto main_header = read<v2::header>(stream);

      return v4::material_list{
        main_header,
        read_vector<v4::material>(stream, main_header.num_materials * main_header.num_details)
      };
    }

    throw std::invalid_argument("The material list version provided is not supported: " + std::to_string(object_header.version));
  }

  template<typename ShapeType>
  void read_materials(ShapeType& shape, std::basic_istream<std::byte>& stream)
  {
    if (auto has_material_list = read<shape::v2::has_material_list_flag>(stream); has_material_list == 1)
    {
      auto object_header = read_object_header(stream);

      shape.material_list = read_material_list(object_header, stream);
    }
  }

  template<typename ShapeType>
  ShapeType read_shape_impl(std::basic_istream<std::byte>& stream)
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


  shape_variant read_shape(std::basic_istream<std::byte>& stream, std::optional<tag_header> file_header)
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


  shape_or_material_list read_shape(std::basic_istream<std::byte>& stream)
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
  void write_header(std::basic_ostream<std::byte>& stream, const RootType& root)
  {
    constexpr static auto empty = std::byte{ '\0' };
    boost::endian::little_uint32_t size_in_bytes{};
    write(stream, pers_tag);
    // This is a placeholder for the real size which will come later
    write(stream, size_in_bytes);

    constexpr std::string_view type_name = std::remove_reference_t<decltype(root)>::type_name;
    const boost::endian::little_int16_t type_size = static_cast<std::uint16_t>(type_name.size());

    write(stream, type_size);
    write(stream, type_name.data(), type_size);

    if constexpr (type_name.size() < 16)
    {
      write(stream, empty);
    }

    const boost::endian::little_int32_t version = std::remove_reference_t<decltype(root)>::version;
    write(stream, version);
  }

  void write_size(std::basic_ostream<std::byte>& stream, std::optional<std::uint32_t> start_offset = std::nullopt)
  {
    boost::endian::little_uint32_t size_in_bytes{};

    start_offset = start_offset.has_value() ? start_offset.value() + static_cast<std::uint32_t>(pers_tag.size()) : static_cast<std::uint32_t>(pers_tag.size());
    std::uint32_t end_offset = static_cast<std::uint32_t>(stream.tellp());

    // sort out the size we reserved earlier
    size_in_bytes = end_offset - start_offset.value() - sizeof(size_in_bytes);
    stream.seekp(start_offset.value(), std::ios_base::beg);

    write(stream, size_in_bytes);

    stream.seekp(end_offset, std::ios_base::beg);
  }
}