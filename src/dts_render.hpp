#ifndef DARKSTARDTSCONVERTER_DTS_RENDER_HPP
#define DARKSTARDTSCONVERTER_DTS_RENDER_HPP

#include <iterator>
#include <algorithm>
#include <utility>
#include <optional>
#include <unordered_map>
#include <set>
#include <memory_resource>
#include "dts_structures.hpp"

struct shape_renderer
{
  virtual void update_node(std::string_view node_name) = 0;
  virtual void update_object(std::string_view object_name) = 0;
  virtual void new_face(std::size_t num_vertices) = 0;
  virtual void end_face() = 0;
  virtual void emit_vertex(const darkstar::dts::vector3f& vertex) = 0;
  virtual void emit_texture_vertex(const darkstar::dts::mesh::v1::texture_vertex& vertex) = 0;

  virtual ~shape_renderer() = default;

  shape_renderer() = default;
  shape_renderer(const shape_renderer&) = delete;
  shape_renderer(shape_renderer&&) = delete;
};


std::vector<std::string> get_detail_levels(const darkstar::dts::shape_variant& shape)
{
  return std::visit([](const auto& instance) {
    std::vector<std::string> results;
    results.reserve(instance.details.size());

    for (const auto& detail : instance.details)
    {
      const auto root_note_index = detail.root_node_index;
      const auto& node = instance.nodes[root_note_index];
      results.emplace_back(instance.names[node.name_index].data());
    }

    return results;
  },
    shape);
}

void render_dts(const darkstar::dts::shape_variant& shape_variant, shape_renderer& renderer, std::optional<std::size_t> detail_level_index = std::nullopt)
{
  std::visit([&](const auto& shape) {
    namespace dts = darkstar::dts;
    std::vector<std::byte> buffer(8192, std::byte{ 0 });
    std::pmr::monotonic_buffer_resource resource{ buffer.data(), buffer.size() };

    using transform_set = std::pmr::set<typename decltype(shape.transforms)::const_pointer>;

    detail_level_index = detail_level_index.has_value() ? detail_level_index : 0;
    const auto& detail_level = shape.details[detail_level_index.value()];
    const auto root_note_index = detail_level.root_node_index;

    std::pmr::unordered_map<std::int32_t, transform_set> node_indexes{ &resource };
    std::pmr::unordered_map<std::int32_t, std::pmr::set<std::int32_t>> object_indexes{ &resource };

    std::list<std::int32_t> valid_nodes;

    valid_nodes.emplace_back(object_indexes.emplace(root_note_index, std::pmr::set<std::int32_t>{ &resource }).first->first);

    for (const auto parent_index : valid_nodes)
    {
      const auto [location, added] = node_indexes.emplace(parent_index, transform_set{ &resource });

      auto transform_index = shape.nodes[parent_index].default_transform_index;
      const auto* transform = &shape.transforms[transform_index];
      location->second.emplace(transform);

      for (auto other_node = std::begin(shape.nodes); other_node != std::end(shape.nodes); ++other_node)
      {
        if (other_node->parent_node_index == parent_index)
        {
          const std::string_view data = shape.names[other_node->name_index].data();
          const auto index = static_cast<std::int32_t>(std::distance(std::begin(shape.nodes), other_node));

          auto [iterator, added] = object_indexes.emplace(index, std::pmr::set<std::int32_t>{ &resource });

          if (added)
          {
            valid_nodes.emplace_back(iterator->first);
          }

          for (const auto* other_transform : node_indexes[parent_index])
          {
            const auto [child_location, child_added] = node_indexes.emplace(index, transform_set{ &resource });
            child_location->second.emplace(other_transform);
          }
        }
      }
    }

    for (auto object = std::begin(shape.objects); object != std::end(shape.objects); ++object)
    {
      if (const auto item = object_indexes.find(object->node_index);
          item != object_indexes.cend())
      {
        const auto index = static_cast<std::int32_t>(std::distance(std::begin(shape.objects), object));
        item->second.emplace(index);
      }
    }

    for (const auto& [child_node_index, transforms] : node_indexes)
    {
      std::optional<dts::vector3f> default_scale;
      dts::vector3f default_translation = { 0, 0, 0 };

      for (auto transform : transforms)
      {
        if constexpr (std::remove_reference_t<decltype(shape)>::version < 8)
        {
          if (!default_scale.has_value())
          {
            default_scale = transform->scale;
          }
        }
        default_translation = default_translation + transform->translation;
      }

      const auto& child_node = shape.nodes[child_node_index];
      const std::string_view child_node_name = shape.names[child_node.name_index].data();
      renderer.update_node(child_node_name);

      auto& objects = object_indexes[child_node_index];

      for (const std::int32_t object_index : objects)
      {
        const auto& object = shape.objects[object_index];
        const std::string_view parent_object_name = shape.names[object.name_index].data();

        renderer.update_object(parent_object_name);

        std::visit([&](const auto& mesh) {
          dts::vector3f mesh_scale;
          dts::vector3f mesh_origin;

          if constexpr (std::remove_reference_t<decltype(mesh)>::version < 3)
          {
            mesh_scale = mesh.header.scale;
            mesh_origin = mesh.header.origin;
          }
          else if constexpr (std::remove_reference_t<decltype(mesh)>::version >= 3)
          {
            if (!mesh.frames.empty())
            {
              mesh_scale = mesh.frames[0].scale;
              mesh_origin = mesh.frames[0].origin;
            }
            else
            {
              mesh_scale = { 1, 1, 1 };
              mesh_origin = { 0, 0, 0 };
            }
          }

          if (default_scale.has_value())
          {
            mesh_scale = mesh_scale * default_scale.value();
          }

          mesh_origin = mesh_origin + default_translation;

          for (const auto& face : mesh.faces)
          {
            renderer.new_face(3);
            std::array vertices{ std::cref(mesh.vertices[face.vi3]),
              std::cref(mesh.vertices[face.vi2]),
              std::cref(mesh.vertices[face.vi1]) };

            std::array texture_vertices{ std::cref(mesh.texture_vertices[face.ti3]),
              std::cref(mesh.texture_vertices[face.ti2]),
              std::cref(mesh.texture_vertices[face.ti1]) };

            for (const auto& raw_vertex : vertices)
            {
              renderer.emit_vertex(raw_vertex.get() * mesh_scale + mesh_origin);
            }

            for (const auto& raw_texture_vertex : texture_vertices)
            {
              renderer.emit_texture_vertex(raw_texture_vertex.get());
            }

            renderer.end_face();
          }
        },
          shape.meshes[object.mesh_index]);
      }
    }
  },
    shape_variant);
}

#endif//DARKSTARDTSCONVERTER_DTS_RENDER_HPP
