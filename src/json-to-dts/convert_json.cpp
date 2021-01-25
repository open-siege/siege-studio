#include <iostream>
#include <vector>
#include <algorithm>
#include <execution>
#include "dts_structures.hpp"
#include "content/json_boost.hpp"
#include "complex_serializer.hpp"
#include "shared.hpp"
#include "dts_io.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;

bool replace(std::string& str, const std::string& from, const std::string& to)
{
  size_t start_pos = str.rfind(from);
  if (start_pos == std::string::npos)
  {
    return false;
  }
  str.replace(start_pos, from.length(), to);
  return true;
}

int main(int argc, const char** argv)
{
  const auto files = shared::find_files(
    std::vector<std::string>(argv + 1, argv + argc),
    ".dts.json",
    ".DTS.json",
    ".dml.json",
    ".DML.json");

  std::for_each(std::execution::par_unseq, files.begin(), files.end(), [](auto&& file_name) {
    try
    {
      {
        std::stringstream msg;
        msg << "Converting " << file_name.string() << '\n';
        std::cout << msg.str();
      }

      std::ifstream test_file(file_name);
      auto fresh_shape_json = nlohmann::json::parse(test_file);
      const auto json_type_name = fresh_shape_json.at("typeName").get<std::string>();

      std::string new_file_name = file_name.string();
      replace(new_file_name, ".json", "");

      if (fs::is_regular_file(new_file_name) && !fs::is_regular_file(new_file_name + ".old"))
      {
        fs::rename(new_file_name, new_file_name + ".old");
      }

      if (json_type_name == dts::material_list::v2::material_list::type_name)
      {
        using namespace binary::io;
        const dts::material_list_variant fresh_shape = fresh_shape_json;

        std::basic_ofstream<std::byte> stream(new_file_name, std::ios::binary);

        std::visit([&](const auto& materials) {
          dts::write_header(stream, materials);
          write(stream, materials.header);
          write(stream, materials.materials);
          dts::write_size(stream);
        },
          fresh_shape);

        std::stringstream msg;
        msg << "Created " << new_file_name << '\n';
        std::cout << msg.str();
      }
      else if (json_type_name == dts::shape::v2::shape::type_name)
      {
        using namespace binary::io;
        const dts::shape_variant fresh_shape = fresh_shape_json;

        std::basic_ofstream<std::byte> stream(new_file_name, std::ios::binary);

        std::visit([&](const auto& shape) {
          dts::write_header(stream, shape);
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
              dts::write_header(stream, mesh);
              write(stream, mesh.header);
              write(stream, mesh.vertices);
              write(stream, mesh.texture_vertices);
              write(stream, mesh.faces);
              write(stream, mesh.frames);
              dts::write_size(stream, start_offset);
            },
              mesh_var);
          }

          const boost::endian::little_uint32_t has_materials = 1u;
          write(stream, has_materials);

          const auto start_offset = static_cast<std::uint32_t>(stream.tellp());
          std::visit([&](const auto& materials) {
            dts::write_header(stream, materials);
            write(stream, materials.header);
            write(stream, materials.materials);
            dts::write_size(stream, start_offset);
          },
            shape.material_list);

          dts::write_size(stream);

          std::stringstream msg;
          msg << "Created " << new_file_name << '\n';
          std::cout << msg.str();
        },
          fresh_shape);
      }
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