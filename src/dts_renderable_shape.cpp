//
// Created by Matthew on 2020/10/05.
//

#include "dts_renderable_shape.hpp"
#include <map>
#include <variant>
#include <optional>
#include <iostream>
#include <glm/gtx/quaternion.hpp>

template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

std::tuple<darkstar::dts::vector3f, darkstar::dts::quaternion4f, darkstar::dts::vector3f> get_translation(const darkstar::dts::shape::v2::transform& transform)
{
  return std::make_tuple(transform.translation * transform.scale, darkstar::dts::to_float(transform.rotation), transform.scale);
}

std::tuple<darkstar::dts::vector3f, darkstar::dts::quaternion4f, darkstar::dts::vector3f> get_translation(const darkstar::dts::shape::v7::transform& transform)
{
  return std::make_tuple(transform.translation * transform.scale, darkstar::dts::to_float(transform.rotation), transform.scale);
}

std::tuple<darkstar::dts::vector3f, darkstar::dts::quaternion4f, darkstar::dts::vector3f> get_translation(const darkstar::dts::shape::v8::transform& transform)
{
  return std::make_tuple(transform.translation, darkstar::dts::to_float(transform.rotation), darkstar::dts::vector3f{ 1.0f, 1.0f, 1.0f });
}

instance_info get_instance(const darkstar::dts::shape_variant& shape, std::size_t detail_level_index, const std::vector<sequence_info>& sequences)
{
  return std::visit([&](const auto& local_shape) {
    instance_info info{};

    const auto& detail_level = local_shape.details[detail_level_index];
    const auto root_note_index = detail_level.root_node_index;

    std::list<std::int32_t> valid_nodes;

    valid_nodes.emplace_back(info.object_indexes.emplace(root_note_index, std::pmr::set<std::int32_t>{}).first->first);

    for (const auto parent_index : valid_nodes)
    {
      const auto [location, added] = info.node_indexes.emplace(parent_index, instance_info::transform_set{});

      auto transform_index = local_shape.nodes[parent_index].default_transform_index;

      for (auto& sequence : sequences)
      {
        if (sequence.enabled)
        {
          for (auto& sub_sequence : sequence.sub_sequences)
          {
            if (sub_sequence.enabled && parent_index == sub_sequence.node_index)
            {
              const auto& key_frame = local_shape.keyframes[sub_sequence.first_key_frame_index + sub_sequence.frame_index];
              transform_index = key_frame.transform_index;
              break;
            }
          }
        }
      }

      location->second.emplace_back(get_translation(local_shape.transforms[transform_index]));

      for (auto other_node = std::begin(local_shape.nodes); other_node != std::end(local_shape.nodes); ++other_node)
      {
        if (other_node->parent_node_index == parent_index)
        {
          //const std::string_view data = local_shape.names[other_node->name_index].data();
          const auto index = static_cast<std::int32_t>(std::distance(std::begin(local_shape.nodes), other_node));

          auto [iterator, node_added] = info.object_indexes.emplace(index, std::pmr::set<std::int32_t>{});

          if (node_added)
          {
            valid_nodes.emplace_back(iterator->first);
          }

          for (const auto& other_transform : info.node_indexes[parent_index])
          {
            const auto [child_location, child_added] = info.node_indexes.emplace(index, instance_info::transform_set{});
            child_location->second.emplace_back(other_transform);
          }
        }
      }
    }

    for (auto object = std::begin(local_shape.objects); object != std::end(local_shape.objects); ++object)
    {
      if (const auto item = info.object_indexes.find(object->node_index);
          item != info.object_indexes.cend())
      {
        const auto index = static_cast<std::int32_t>(std::distance(std::begin(local_shape.objects), object));
        item->second.emplace(index);
      }
    }

    return info;
  },
    shape);
}

std::vector<sequence_info> dts_renderable_shape::get_sequences(const std::vector<std::size_t>& detail_level_indexes) const
{
  std::vector<sequence_info> results;

  if (shape.index() == std::variant_npos)
  {
    return results;
  }

  std::visit([&](auto& local_shape) {
    if (local_shape.sequences.empty() || local_shape.sub_sequences.empty())
    {
      return;
    }

    std::vector<sequence_info> sequences;
    results.reserve(local_shape.sequences.size());

    for (auto i = 0; i < local_shape.sequences.size(); ++i)
    {
      auto& sequence = local_shape.sequences[i];
      results.push_back({ i, local_shape.names[sequence.name_index].data(), i == 0, std::vector<sub_sequence_info>{} });
    }

    if (local_shape.details.empty())
    {
      return;
    }

    for (auto detail_level_index : detail_level_indexes)
    {
      auto instance = get_instance(shape, detail_level_index, sequences);

      for (const auto& [node_index, transforms] : instance.node_indexes)
      {
        if (node_index == -1)
        {
          continue;
        }
        const auto& node = local_shape.nodes[node_index];
        std::string node_name = local_shape.names[node.name_index].data();

        auto create_sub_info = [&](auto node_index, auto& sub_sequence) {
          sub_sequence_info info;
          info.node_index = node_index;
          info.node_name = node_name;
          info.first_key_frame_index = sub_sequence.first_key_frame_index;
          info.num_key_frames = sub_sequence.num_key_frames;
          info.enabled = sub_sequence.sequence_index == 0;

          if (sub_sequence.num_key_frames > 0)
          {
            info.min_position = local_shape.keyframes[sub_sequence.first_key_frame_index].position;
            info.max_position = local_shape.keyframes[sub_sequence.first_key_frame_index + sub_sequence.num_key_frames - 1].position;
          }
          else
          {
            info.min_position = 0;
            info.max_position = 0;
          }

          info.frame_index = 0;
          info.position = info.min_position;

          return info;
        };

        if (node.num_sub_sequences == 0)
        {
          for (auto& object : local_shape.objects)
          {
            if (object.node_index == node_index)
            {
              for (auto i = object.first_sub_sequence_index; i < object.first_sub_sequence_index + object.num_sub_sequences; ++i)
              {
                auto& sub_sequence = local_shape.sub_sequences[i];
                auto& sequence = results[sub_sequence.sequence_index];

                sequence.sub_sequences.emplace_back(create_sub_info(node_index, sub_sequence));
              }
              break;
            }
          }
        }
        else
        {
          for (auto i = node.first_sub_sequence_index; i < node.first_sub_sequence_index + node.num_sub_sequences; ++i)
          {
            auto& sub_sequence = local_shape.sub_sequences[i];
            auto& sequence = results[sub_sequence.sequence_index];

            sequence.sub_sequences.emplace_back(create_sub_info(node_index, sub_sequence));
          }
        }
      }
    }
  },
    shape);

  return results;
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

void dts_renderable_shape::render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes, const std::vector<sequence_info>& sequences) const
{

  std::visit([&](const auto& local_shape) {
    namespace dts = darkstar::dts;

    if (local_shape.details.empty())
    {
      return;
    }

    for (auto detail_level_index : detail_level_indexes)
    {

      auto instance = get_instance(shape, detail_level_index, sequences);

      for (const auto& node_item : instance.node_indexes)
      {
        auto& [node_index, transforms] = node_item;

        const auto& node = local_shape.nodes[node_index];
        const std::string_view node_name = local_shape.names[node.name_index].data();

        std::optional<std::string_view> parent_node_name;

        if (node.parent_node_index != -1)
        {
          const auto& parent_node = local_shape.nodes[node.parent_node_index];
          parent_node_name = local_shape.names[parent_node.name_index].data();
        }

        renderer.update_node(parent_node_name, node_name);

        auto& objects = instance.object_indexes[node_index];

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
                auto vertex = glm::vec4(raw_vertex.get().x, raw_vertex.get().y, raw_vertex.get().z, 1.0f);

                auto translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(mesh_origin.x, mesh_origin.y, mesh_origin.z));
                auto rotation_matrix = glm::mat4(1.0f);
                auto scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(mesh_scale.x, mesh_scale.y, mesh_scale.z));

                vertex = translation_matrix * rotation_matrix * scale_matrix * vertex;


                for (const auto& [translation, rotation, scale] : node_item.second)
                {
                  translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));
                  //rotation_matrix = glm::mat4(1.0f) * glm::toMat4(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z));
                  scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
                  vertex = translation_matrix * rotation_matrix * scale_matrix * vertex;
                }

                renderer.emit_vertex(darkstar::dts::vector3f{ vertex.x, vertex.y, vertex.z });
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