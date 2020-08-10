#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <execution>
#include <optional>
#include "structures.hpp"
#include "json_boost.hpp"
#include "complex_serializer.hpp"
#include "shared.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;

template<std::size_t Size>
void write(std::basic_ostream<std::byte>& stream, const std::array<std::byte, Size>& value)
{
  stream.write(value.data(), Size);
}

template<typename ValueType>
void write(std::basic_ostream<std::byte>& stream, const ValueType& value)
{
  stream.write(reinterpret_cast<const std::byte*>(&value), sizeof(value));
}

template<typename ValueType>
void write(std::basic_ostream<std::byte>& stream, const std::vector<ValueType>& values)
{
  stream.write(reinterpret_cast<const std::byte*>(values.data()), values.size() * sizeof(ValueType));
}

template<typename ValueType>
void write(std::basic_ostream<std::byte>& stream, const ValueType* value, std::size_t size)
{
  stream.write(reinterpret_cast<const std::byte*>(value), size);
}

template<typename RootType>
void write_header(std::basic_ostream<std::byte>& stream, const RootType& root)
{
  constexpr static auto empty = std::byte{ '\0' };
  boost::endian::little_uint32_t size_in_bytes{};
  write(stream, dts::pers_tag);
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

  start_offset = start_offset.has_value() ? start_offset.value() + dts::pers_tag.size() : dts::pers_tag.size();
  std::uint32_t end_offset = static_cast<std::uint32_t>(stream.tellp());

  // sort out the size we reserved earlier
  size_in_bytes = end_offset - start_offset.value() - sizeof(size_in_bytes);
  stream.seekp(start_offset.value(), std::ios_base::beg);

  write(stream, size_in_bytes);

  stream.seekp(end_offset, std::ios_base::beg);
}

int main(int argc, const char** argv)
{
  const auto files = dts::shared::find_files(
    std::vector<std::string>(argv + 1, argv + argc),
    ".dts.json",
    ".DTS.json" ,
    ".dml.json",
    ".DML.json");

  std::for_each(std::execution::par_unseq, files.begin(), files.end(), [](auto&& file_name) {
    try
    {
      {
        std::stringstream msg;
        msg << "Converting " << file_name << '\n';
        std::cout << msg.str();
      }

      std::ifstream test_file(file_name);
      auto fresh_shape_json = nlohmann::json::parse(test_file);
      const auto json_type_name = fresh_shape_json.at("typeName").get<std::string>();

      if (json_type_name == dts::material_list::v2::material_list::type_name)
      {
        const dts::material_list_variant fresh_shape = fresh_shape_json;
        auto new_file_name = file_name.string() + ".dml";

        std::basic_ofstream<std::byte> stream(new_file_name, std::ios::binary);

        std::visit([&](const auto& materials) {
                          write_header(stream, materials);
                          write(stream, materials.header);
                          write(stream, materials.materials);
                          write_size(stream);
                   },
                   fresh_shape);
      }
      else if (json_type_name == dts::shape::v2::shape::type_name)
      {
        const dts::shape_variant fresh_shape = fresh_shape_json;

        auto new_file_name = file_name.string() + ".dts";

        std::basic_ofstream<std::byte> stream(new_file_name, std::ios::binary);

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

          const boost::endian::little_uint32_t has_materials = 1u;
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