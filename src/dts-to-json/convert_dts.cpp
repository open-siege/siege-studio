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

int main(int argc, const char** argv)
{
  const auto files = dts::shared::find_files(
    std::vector<std::string>(argv + 1, argv + argc),
    ".dts",
    ".DTS",
    ".dml",
    ".DML");

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

        auto sequences = nlohmann::json::object();

        for (auto& sequence : item_as_json["sequences"])
        {
          auto sequence_name = item_as_json["names"][sequence["nameIndex"].get<int>()].get<std::string>();
          sequence["name"] = sequence_name;
          sequences[sequence_name] = sequence;
        }

        auto nodes = nlohmann::json::object();

        for (auto& node : item_as_json["nodes"])
        {
          auto parent_index = node["parentNodeIndex"].get<int>();

          if (parent_index != -1)
          {
            auto& parent_node = item_as_json["nodes"][parent_index];

            auto parent_name = item_as_json["names"][parent_node["nameIndex"].get<int>()].get<std::string>();
            node["parentNodeIndex"] = parent_name;
          }
          else
          {
            node["parentNodeIndex"] = nullptr;
          }
          //const auto default_transform_index = element["defaultTransformIndex"].get<int>();

          std::string name = item_as_json["names"][node["nameIndex"].get<int>()].get<std::string>();
          node["name"] = name;


          nlohmann::json element = node;

          element["sequences"] = nlohmann::json::object();

          auto& sub_sequences = item_as_json["subSequences"];
          const auto num_sub_sequences = element["numSubSequences"].get<int>();
          const auto first_sub_sequence = element["firstSubSequence"].get<int>();
          for (auto i = first_sub_sequence; i < first_sub_sequence + num_sub_sequences; ++i)
          {
            auto& local_sub_sequence = sub_sequences[i];
            auto& sequence = item_as_json["sequences"][local_sub_sequence["sequenceIndex"].get<int>()];
            auto sequence_name = sequence["name"].get<std::string>();

            auto& key_frames = element["subSequences"][sequence_name] = nlohmann::json::array();
            const auto first_key_frame = local_sub_sequence["firstKeyFrame"].get<int>();
            const auto num_key_frames = local_sub_sequence["numKeyFrames"].get<int>();

            auto& keyframes = item_as_json["keyframes"];
            for (auto j = first_key_frame; j < first_key_frame + num_key_frames; ++j)
            {
              auto& keyframe = keyframes[j];
              keyframe["keyframeIndex"] = j;
              auto transform_index = keyframe["transformIndex"].get<int>();
              auto& transform = item_as_json["transforms"][transform_index];

              keyframe["transformation"] = transform;

              key_frames.emplace_back(keyframe);
            }
          }

          nodes.emplace(name, element);
        }

        auto objects = nlohmann::json::object();

        for (auto& object : item_as_json["objects"])
        {

          auto name = item_as_json["names"][object["nameIndex"].get<int>()].get<std::string>();
          object["name"] = name;
          //          objects[name] = element;

          if (nodes.contains(name))
          {
            auto& node = nodes[name];
            object["parentObjectIndex"] = node["parentNodeIndex"];
            object["nodeIndex"] = name;
            node["objectIndex"] = name;
          }


          nlohmann::json element = object;
          element["sequences"] = nlohmann::json::object();

          auto& sub_sequences = item_as_json["subSequences"];
          const auto num_sub_sequences = element["numSubSequences"].get<int>();
          const auto first_sub_sequence = element["firstSubSequence"].get<int>();
          for (auto i = first_sub_sequence; i < first_sub_sequence + num_sub_sequences; ++i)
          {
            auto& local_sub_sequence = sub_sequences[i];
            auto& sequence = item_as_json["sequences"][local_sub_sequence["sequenceIndex"].get<int>()];
            auto sequence_name = sequence["name"].get<std::string>();

            auto& key_frames = element["subSequences"][sequence_name] = nlohmann::json::array();
            const auto first_key_frame = local_sub_sequence["firstKeyFrame"].get<int>();
            const auto num_key_frames = local_sub_sequence["numKeyFrames"].get<int>();

            auto& keyframes = item_as_json["keyframes"];
            for (auto j = first_key_frame; j < first_key_frame + num_key_frames; ++j)
            {
              auto& keyframe = keyframes[j];
              keyframe["keyframeIndex"] = j;
              auto transform_index = keyframe["transformIndex"].get<int>();
              auto& transform = item_as_json["transforms"][transform_index];

              keyframe["transformation"] = transform;

              key_frames.emplace_back(keyframe);
            }
          }
          objects.emplace(name, element);
        }

        for (auto& element : item_as_json["details"])
        {
          auto& node = item_as_json["nodes"][element["rootNodeIndex"].get<int>()];

          element["rootNodeIndex"] = node["name"];
        }

        auto count = 0u;
        for (auto& element : item_as_json["transitions"])
        {
          const auto start_index = element["startSequenceIndex"].get<int>();
          const auto end_index = element["endSequenceIndex"].get<int>();

          element["startSequenceIndex"] = item_as_json["sequences"][start_index]["name"];
          element["endSequenceIndex"] = item_as_json["sequences"][end_index]["name"];

          std::stringstream possible_name;
          possible_name << "Trans";

          if (count < 10)
          {
            possible_name << '0' << count;
          }
          else
          {
            possible_name << count;
          }

          for (auto& name : item_as_json["names"])
          {
            if (const auto raw_name = name.get<std::string>(); raw_name.rfind(possible_name.str(), 0) == 0)
            {
              element["name"] = raw_name;
            }
          }
          count++;
        }

        item_as_json["nodes"] = nodes;
        item_as_json["objects"] = objects;
        item_as_json["sequences"] = sequences;
        item_as_json.erase("transforms");
        item_as_json.erase("keyframes");
        item_as_json.erase("subSequences");
        item_as_json.erase("names");
        item_as_json.erase("header");

        auto new_file_name = file_name.string() + ".json";
        {
          std::ofstream item_as_file(new_file_name, std::ios::trunc);
          item_as_file << item_as_json.dump(4);
        }

        //new_file_name = file_name.string() + ".pack";
        {
          //std::basic_ofstream<std::uint8_t> item_as_file(new_file_name, std::ios::trunc | std::ios::binary);
          //const auto pack = nlohmann::json::to_msgpack(item_as_json);
          //item_as_file.write(&pack[0], pack.size());
        }

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