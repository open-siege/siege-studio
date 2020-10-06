//
// Created by Matthew on 2020/10/05.
//

#include "dts_renderable_shape.hpp"
#include <map>
#include <variant>
#include <optional>


template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;


std::map<std::size_t, dts_renderable_shape::instance_info>::iterator cache_instance(const dts_renderable_shape& self, std::size_t detail_level_index)
{
  return std::visit([&](const auto& local_shape) {
    auto instance_iterator = self.instances.find(detail_level_index);

    if (instance_iterator != self.instances.end())
    {
      return instance_iterator;
    }
    const auto& [info, added] = self.instances.emplace(detail_level_index, dts_renderable_shape::instance_info{});

    const auto& detail_level = local_shape.details[detail_level_index];
    const auto root_note_index = detail_level.root_node_index;

    std::list<std::int32_t> valid_nodes;

    valid_nodes.emplace_back(info->second.object_indexes.emplace(root_note_index, std::pmr::set<std::int32_t>{}).first->first);

    for (const auto parent_index : valid_nodes)
    {
      const auto [location, added] = info->second.node_indexes.emplace(parent_index, dts_renderable_shape::transform_set{});

      auto transform_index = local_shape.nodes[parent_index].default_transform_index;
      const auto* transform = &local_shape.transforms[transform_index];
      location->second.emplace(transform);

      for (auto other_node = std::begin(local_shape.nodes); other_node != std::end(local_shape.nodes); ++other_node)
      {
        if (other_node->parent_node_index == parent_index)
        {
          //const std::string_view data = local_shape.names[other_node->name_index].data();
          const auto index = static_cast<std::int32_t>(std::distance(std::begin(local_shape.nodes), other_node));

          auto [iterator, node_added] = info->second.object_indexes.emplace(index, std::pmr::set<std::int32_t>{});

          if (node_added)
          {
            valid_nodes.emplace_back(iterator->first);
          }

          for (const auto other_transform : info->second.node_indexes[parent_index])
          {
            const auto [child_location, child_added] = info->second.node_indexes.emplace(index, dts_renderable_shape::transform_set{});
            child_location->second.emplace(other_transform);
          }
        }
      }
    }

    for (auto object = std::begin(local_shape.objects); object != std::end(local_shape.objects); ++object)
    {
      if (const auto item = info->second.object_indexes.find(object->node_index);
          item != info->second.object_indexes.cend())
      {
        const auto index = static_cast<std::int32_t>(std::distance(std::begin(local_shape.objects), object));
        item->second.emplace(index);
      }
    }

    return info;
  },
    self.shape);
}

std::vector<std::string> dts_renderable_shape::get_detail_levels() const
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

void dts_renderable_shape::render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes /*, const std::vector<sequence_info>& sequences*/) const
{
  std::visit([&](const auto& local_shape) {
    namespace dts = darkstar::dts;
    if (local_shape.details.empty())
    {
      return;
    }

    for (auto detail_level_index : detail_level_indexes)
    {
      auto instance = cache_instance(*this, detail_level_index);

      for (const auto& [node_index, transforms] : instance->second.node_indexes)
      {
        std::optional<dts::vector3f> default_scale;
        dts::vector3f default_translation = { 0, 0, 0 };
        dts::quaternion4f default_rotation = { 0, 0, 0, 1 };

        for (auto raw_transform : transforms)
        {
          auto raw_translation = std::visit([](auto& real_transform) { return real_transform->translation; }, raw_transform);
          auto scale = std::visit(overloaded{
                                    [](const dts::shape::v2::transform* real_transform) { return real_transform->scale; },
                                    [](const dts::shape::v7::transform* real_transform) { return real_transform->scale; },
                                    [](const dts::shape::v8::transform*) { return dts::vector3f{ 1, 1, 1 }; } },
            raw_transform);

          if constexpr (std::remove_reference_t<decltype(local_shape)>::version < 8)
          {
            if (!default_scale.has_value())
            {
              default_scale = scale;
            }
          }

          //auto rotation = dts::to_float(transform->rotation);

          // glm::quat glm_rotation{rotation.w, rotation.x, rotation.y, rotation.z};
          glm::vec3 translation{ raw_translation.x, raw_translation.y, raw_translation.z };

          // translation = glm::rotate(glm_rotation, translation);

          default_translation.x += translation.x;
          default_translation.y += translation.y;
          default_translation.z += translation.z;
        }

        const auto& node = local_shape.nodes[node_index];
        const std::string_view node_name = local_shape.names[node.name_index].data();

        std::optional<std::string_view> parent_node_name;

        if (node.parent_node_index != -1)
        {
          const auto& parent_node = local_shape.nodes[node.parent_node_index];
          parent_node_name = local_shape.names[parent_node.name_index].data();
        }

        renderer.update_node(parent_node_name, node_name);

        auto& objects = instance->second.object_indexes[node_index];

        for (const std::int32_t object_index : objects)
        {
          const auto& object = local_shape.objects[object_index];
          const std::string_view object_name = local_shape.names[object.name_index].data();

          renderer.update_object(node_name, object_name);

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
            local_shape.meshes[object.mesh_index]);
        }
      }
    }
  },
    shape);
}