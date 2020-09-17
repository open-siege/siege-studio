#include <iostream>
#include <iterator>
#include <algorithm>
#include <execution>
#include <bitset>
#include <map>
#include <memory_resource>
#include "json_boost.hpp"
#include "complex_serializer.hpp"
#include "shared.hpp"
#include "dts_io.hpp"
//#include "dts_json_formatting.hpp"

namespace fs = std::filesystem;
namespace dts = darkstar::dts;


struct shape_renderer
{
  virtual void update_node(std::string_view node_name) = 0;
  virtual void update_object(std::string_view object_name) = 0;
  virtual void new_face(std::optional<std::size_t> num_vertices = std::nullopt) = 0;
  virtual void emit_vertex(const darkstar::dts::vector3f& vertex) = 0;
  virtual void emit_texture_vertex(const darkstar::dts::mesh::v1::texture_vertex& vertex) = 0;

  virtual ~shape_renderer() = default;

  shape_renderer() = delete;
  shape_renderer(const shape_renderer&) = delete;
  shape_renderer(shape_renderer&&) = delete;
};


void render_dts(const dts::shape::v7::shape& some_shape, shape_renderer& renderer)
{
  //std::visit([&](const auto& item)
  //    {
  std::vector<std::byte> buffer(1024, std::byte{ 0 });
  std::pmr::monotonic_buffer_resource resource{ buffer.data(), buffer.size() };
  std::pmr::map<boost::endian::little_int32_t, std::pmr::set<std::int32_t>> node_indexes{ &resource };
  std::pmr::map<boost::endian::little_int32_t, std::pmr::set<std::int32_t>> object_indexes{ &resource };
  std::pmr::set<std::int32_t> excluded_node_indexes{ &resource };

  const auto& detail_level = some_shape.details[0];
  const auto root_note_index = detail_level.root_node_index;

  for (auto node = std::begin(some_shape.nodes); node != std::end(some_shape.nodes); ++node)
  {
    const auto index = static_cast<std::int32_t>(std::distance(std::begin(some_shape.nodes), node));

    if (node->parent_node_index == -1 && index != root_note_index)
    {
      excluded_node_indexes.emplace(index);
      continue;
    }

    if (excluded_node_indexes.find(node->parent_node_index) != excluded_node_indexes.end())
    {
      excluded_node_indexes.emplace(index);
      continue;
    }

    auto item = node_indexes.find(node->parent_node_index);

    if (item == node_indexes.end())
    {
      item = object_indexes.emplace(node->parent_node_index, std::pmr::set<std::int32_t>{ &resource }).first;
    }

    item->second.emplace(index);
  }

  for (auto object = std::begin(some_shape.objects); object != std::end(some_shape.objects); ++object)
  {
    if (const auto item = node_indexes.find(object->node_index);
        item != node_indexes.cend())
    {
      auto other = object_indexes.find(object->node_index);

      if (other == object_indexes.end())
      {
        other = object_indexes.emplace(object->node_index, std::pmr::set<std::int32_t>{ &resource }).first;
      }
      const auto index = static_cast<std::int32_t>(std::distance(std::begin(some_shape.objects), object));
      other->second.emplace(index);
    }
  }

  std::optional<dts::vector3f> default_scale;
  dts::vector3f default_translation = { 0, 0, 0};

  for (const auto& [parent_node_index, child_node_indexes] : node_indexes)
  {
    if (parent_node_index != -1)
    {
      const auto& parent_node = some_shape.nodes[parent_node_index];
      const std::string_view parent_node_name = some_shape.names[parent_node.name_index].data();
      renderer.update_node(parent_node_name);
      const auto& default_transform = some_shape.transforms[parent_node.default_transform_index];

      // TODO get scale for default transform
      if (!default_scale.has_value())
      {
        default_scale = default_transform.scale;
      }

      default_translation += default_transform.translation;
    }

    for (const auto child_node_index : child_node_indexes)
    {
      std::pmr::set<std::int32_t>& objects = object_indexes[child_node_index];

      for (const std::int32_t object_index : objects)
      {
        const auto& object = some_shape.objects[object_index];
        const std::string_view parent_object_name = some_shape.names[object.name_index].data();

        renderer.update_object(parent_object_name);

        //const auto& mesh = some_shape.meshes[object.mesh_index];
        const dts::mesh::v3::mesh mesh{};
        //TODO add code to get scale and origin from mesh object
        // if there is no value it is nullopt
        std::optional<dts::vector3f> mesh_scale;
        std::optional<dts::vector3f> mesh_origin;

        for (const auto& frame : mesh.frames)
        {
          //TODO add code to get scale and origin from frame object
          // if there is no value it is a default value, which should not be the case since it should
          // either exist on the mesh or the frame, or we have bad data.
          dts::vector3f scale = mesh_scale.has_value() ? mesh_scale.value() : frame.scale;

          if (default_scale.has_value())
          {
            scale *= default_scale.value();
          }

          dts::vector3f origin = (mesh_origin.has_value() ? mesh_origin.value() : frame.origin) + default_translation;

          for (const auto& face : mesh.faces)
          {
            renderer.new_face(3);
            std::array vertices{ std::cref(mesh.vertices[face.vi1]),
                                 std::cref(mesh.vertices[face.vi2]),
                                 std::cref(mesh.vertices[face.vi3]) };

            std::array texture_vertices{ std::cref(mesh.texture_vertices[face.ti1]),
                                 std::cref(mesh.texture_vertices[face.ti2]),
                                 std::cref(mesh.texture_vertices[face.ti3]) };

            for (const auto& raw_vertex : vertices)
            {
              renderer.emit_vertex(raw_vertex.get() * scale + origin);
            }

            for (const auto& raw_texture_vertex : texture_vertices)
            {
              renderer.emit_texture_vertex(raw_texture_vertex.get());
            }
          }
        }
      }
    }
  }

  //  }, some_shape);
}


int main(int argc, const char** argv)
{
  const auto files = shared::find_files(
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

      auto shape = dts::read_shape(file_name, input);

      //      std::visit([&](const auto& item) {
      //
      //        std::stringstream msg;
      //        msg << "Created " << item.nodes.size() << '\n';
      //        std::cout << msg.str();
      //      },
      //        shape);
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