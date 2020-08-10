#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <execution>
#include <sstream>
#include <filesystem>
#include "structures.hpp"
#include "json_boost.hpp"
#include "complex_serializer.hpp"
#include "shared.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;

std::string read_string(std::basic_ifstream<std::byte>& stream, std::size_t size)
{
  std::string dest(size, '\0');

  stream.read(reinterpret_cast<std::byte*>(&dest[0]), size);

  // There is always an embedded \0 in the
  // file if the string length is less than 16 bytes.
  if (size < 16)
  {
    stream.seekg(1, std::ios_base::cur);
  }

  return dest;
}


template<typename destination_type>
std::vector<destination_type> read_vector(std::basic_ifstream<std::byte>& stream, std::size_t size)
{
  if (size == 0)
  {
    return {};
  }

  std::vector<destination_type> dest(size);

  stream.read(reinterpret_cast<std::byte*>(&dest[0]), sizeof(destination_type) * size);

  return dest;
}


template<std::size_t size>
std::array<std::byte, size> read(std::basic_ifstream<std::byte>& stream)
{
  std::array<std::byte, size> dest{};

  stream.read(&dest[0], size);

  return dest;
}

template<typename destination_type>
destination_type read(std::basic_ifstream<std::byte>& stream)
{
  destination_type dest{};

  stream.read(reinterpret_cast<std::byte*>(&dest), sizeof(destination_type));

  return dest;
}

dts::tag_header read_object_header(std::basic_ifstream<std::byte>& stream)
{
  dts::tag_header file_header = {
    read<sizeof(dts::file_tag)>(stream),
    read<dts::file_info>(stream)
  };

  if (file_header.tag != dts::pers_tag)
  {
    std::stringstream msg;
    msg << "There was an error trying to parse a portion of the DTS/DML file at " << stream.tellg() << ". ";
    msg << "Expected " << std::string(reinterpret_cast<const char*>(dts::pers_tag.data()), dts::pers_tag.size()) << " to be present but was not found.\n";

    throw std::invalid_argument(msg.str());
  }

  file_header.class_name = read_string(stream, file_header.file_info.class_name_length);
  file_header.version = read<dts::version>(stream);

  return file_header;
}

template<typename ShapeType>
void read_meshes(ShapeType& shape, std::size_t num_meshes, std::basic_ifstream<std::byte>& stream)
{
  using namespace dts::mesh;
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


dts::material_list_variant read_material_list(const dts::tag_header& object_header, std::basic_ifstream<std::byte>& stream)
{
  using namespace dts::material_list;
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
void read_materials(ShapeType& shape, std::basic_ifstream<std::byte>& stream)
{
  if (auto has_material_list = read<dts::shape::v2::has_material_list_flag>(stream); has_material_list == 1)
  {
    auto object_header = read_object_header(stream);

    shape.material_list = read_material_list(object_header, stream);
  }
}


template<typename ShapeType>
ShapeType read_shape_impl(std::basic_ifstream<std::byte>& stream)
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

dts::shape_or_material_list read_shape(const fs::path& file_name, std::basic_ifstream<std::byte>& stream)
{
  using namespace dts::shape;
  dts::tag_header file_header = read_object_header(stream);

  if (file_header.class_name == dts::material_list::v2::material_list::type_name)
  {
    return read_material_list(file_header, stream);
  }

  if (file_header.class_name != v2::shape::type_name)
  {
    throw std::invalid_argument("The object provided is not a shape as expected.");
  }

  if (file_header.version > 8)
  {
    throw std::invalid_argument("The shape is not supported.");
  }

  if (file_header.version == 2)
  {
    return read_shape_impl<v2::shape>(stream);
  }
  else if (file_header.version == 3)
  {
    return read_shape_impl<v3::shape>(stream);
  }
  else if (file_header.version == 5)
  {
    return read_shape_impl<v5::shape>(stream);
  }
  else if (file_header.version == 6)
  {
    return read_shape_impl<v6::shape>(stream);
  }
  else if (file_header.version == 7)
  {
    return read_shape_impl<v7::shape>(stream);
  }
  else if (file_header.version == 8)
  {
    return read_shape_impl<v8::shape>(stream);
  }
  else
  {
    std::stringstream error;
    error << file_name << " is DTS version " << file_header.version << " which is currently unsupported.";
    throw std::invalid_argument(error.str());
  }
}

void read_from_json(const std::filesystem::path& file_name)
{
  auto new_file_name = file_name.string() + ".json";
  std::ifstream test_file(new_file_name);
  auto fresh_shape_json = nlohmann::json::parse(test_file);
  const dts::shape_variant fresh_shape = fresh_shape_json;
}

int main(int argc, const char** argv)
{
  const auto files = dts::shared::find_files(
    std::vector<std::string>(argv + 1, argv + argc),
    ".dts",".DTS", ".dml", ".DML");

  std::for_each(std::execution::par_unseq, files.begin(), files.end(), [](auto&& file_name) {
    try
    {
      {
        std::stringstream msg;
        msg << "Converting " << file_name.string() << '\n';
        std::cout << msg.str();
      }

      std::basic_ifstream<std::byte> input(file_name, std::ios::binary);

      auto shape = read_shape(file_name, input);

      std::visit([&](const auto& item) {
        nlohmann::ordered_json item_as_json = item;

        auto new_file_name = file_name.string() + ".json";
        std::ofstream item_as_file(new_file_name, std::ios::trunc);
        item_as_file << item_as_json.dump(4);

        std::stringstream msg;
        msg << "Created " << new_file_name << '\n';
        std::cout << msg.str();
      },
        shape);
    }
    catch (const std::exception& ex)
    {
      std::stringstream msg;
      msg << file_name << " " << ex.what() << '\n';
      std::cerr << msg.str();
    }
  });

  return 0;
}