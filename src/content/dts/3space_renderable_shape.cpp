#include "3space_renderable_shape.hpp"
#include "shared.hpp"
#include <functional>

template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace studio::content::dts::three_space
{
  std::vector<sequence_info> dts_renderable_shape::get_sequences(const std::vector<std::size_t>& detail_level_indexes) const
  {
    static_assert(std::is_constructible_v<v1::actual_shape_item, shared::Exactly<v1::an_shape>> == true, "Could not create shape item from an shape");
    std::vector<sequence_info> results;

    v1::shape* shape_ref = nullptr;

    auto shape_val = studio::shared::variant_cast_opt<v1::actual_shape_item>(shape);

    if (!shape_val.has_value())
    {
      return results;
    }

    std::visit(overloaded{
                 [&](v1::shape& arg) { shape_ref = &arg; },
                 [&](v1::an_shape& arg) { shape_ref = &arg.base; } },
      shape_val.value());

    std::vector<v1::an_anim_list> anim_lists = studio::shared::transform_variants<v1::an_anim_list>(shape_ref->extra_parts);

    if (anim_lists.empty())
    {
      return results;
    }

    const auto& anim_list = anim_lists.front();

    results.reserve(anim_list.sequence_count);

    const auto& default_transforms = anim_list.default_transforms;

    for (auto i = 0u; i < anim_list.sequences.size(); ++i)
    {
      const auto& seq = anim_list.sequences[i];

      if (default_transforms.size() < seq.part_list_count)
      {
        continue;
      }

      const auto index = std::int32_t(i);

      std::vector<sub_sequence_info> sub_sequences;
      sub_sequences.reserve(seq.part_list.size());

      for (auto x = 0u; x < seq.part_list.size(); ++x)
      {
        sub_sequence_info temp;
        temp.node_index = std::int32_t(seq.part_list[x]);
        temp.node_name = std::to_string(seq.part_list[x] + 1);
        temp.frame_index = 0;
        temp.first_key_frame_index = 0;
        temp.num_key_frames = std::int32_t(seq.frame_count);
        temp.min_position = 0.0f;
        temp.max_position = 1.0f;
        temp.position = 0.0f;
        temp.enabled = index == 0;

        sub_sequences.emplace_back(std::move(temp));
      }

      results.emplace_back(sequence_info{ index, std::to_string(index + 1), index == 0, std::move(sub_sequences) });
    }

    return results;
  }

  std::vector<std::string> dts_renderable_shape::get_detail_levels() const
  {
    return { "default" };
  }

  void dts_renderable_shape::render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes, const std::vector<sequence_info>& sequences) const
  {

    v1::an_anim_list* item = nullptr;
    std::map<std::int16_t, std::vector<std::int16_t>> nodes;

    if (item->relations_count >= item->default_transform_count)
    {
      for (auto& relation : item->relations)
      {
        if (relation.parent < 0)
        {
          nodes.emplace(relation.destination, std::vector<std::int16_t>());
        }

        if (relation.parent > 0)
        {
          auto existing_item = nodes.find(relation.parent);

          if (existing_item == nodes.end())
          {
            existing_item = nodes.emplace(relation.parent, std::vector<std::int16_t>()).first;
          }

          existing_item->second.emplace_back(relation.destination);
        }
      }
    }

    std::map<std::int16_t, std::vector<std::tuple<std::int32_t, std::int16_t, std::int16_t>>> animated_nodes;

    for (const sequence_info& info : sequences)
    {
      if (info.enabled)
      {
        auto& seq = item->sequences.at(info.index);

        auto i = 0u;
        for (auto part : seq.part_list)
        {
          auto animated_node = animated_nodes.find(part);

          if (animated_node == animated_nodes.end())
          {
            animated_node = animated_nodes.emplace(part, std::vector<std::tuple<std::int32_t, std::int16_t, std::int16_t>>()).first;
          }

          auto f = seq.frame_count;
          for (auto x = 0u; x <= seq.transform_index_list.size(); x += seq.part_list_count + i)
          {
            animated_node->second.emplace_back(std::make_tuple(info.index, std::int16_t(seq.frame_count - f), seq.transform_index_list[x]));

            f--;
          }

          i++;
        }
      }
    }


    std::map<std::int16_t, std::vector<std::int16_t>> node_frames;


    auto real_shape = std::get<v1::shape>(shape);


    //    std::vector<v1::group_item> groups = studio::shared::transform_variants<v1::group_item>(real_shape.base.parts);


    const auto visit_poly = [](const v1::poly& real_poly, const v1::group& real_group) {
      std::vector<int> face;
      face.reserve(real_poly.vertex_count);

      for (auto i = real_poly.vertex_list; i < real_poly.vertex_list + real_poly.vertex_count; ++i)
      {
        face.emplace_back(real_group.indexes.at(i));
      }

      return face;
    };

    const auto visit_group = [&](const v1::group& real_group) {
      const std::vector<v1::poly_item> polys = studio::shared::transform_variants<v1::poly_item>(real_group.items);

      std::vector<std::vector<int>> all_faces;

      for (auto& poly : polys)
      {
        all_faces.emplace_back(std::visit(overloaded{
                                            [&](const v1::poly& arg) { return visit_poly(arg, real_group); },
                                            [&](const v1::solid_poly& arg) { return visit_poly(arg.base, real_group); },
                                            [&](const v1::gouraud_poly& arg) { return visit_poly(arg.base.base, real_group); },
                                            [&](const v1::shaded_poly& arg) { return visit_poly(arg.base.base, real_group); },
                                            [&](const v1::texture_for_poly& arg) { return visit_poly(arg.base.base, real_group); },
                                          },
          poly));
      }

      return all_faces;
    };

    const auto visit_group_item = [&](const v1::group_item& child_item) {
      return std::visit(overloaded{
                          [&](const v1::group& arg) { return visit_group(arg); },
                          [&](const v1::bsp_group& arg) { return visit_group(arg.base); } },
        child_item);
    };


    std::vector<v1::part_list_item> group_parents = studio::shared::transform_variants<v1::part_list_item>(real_shape.base.parts);

    // std::vector<v1::group_item> groups = studio::shared::transform_variants<v1::group_item>(real_shape.base.parts);

    const auto get_groups = [](const v1::part_list_item& parent) {
      return std::visit(overloaded{
                          [&](const v1::shape& arg) { return studio::shared::transform_variants<v1::group_item>(arg.base.parts); },
                          [&](const v1::an_shape& arg) { return studio::shared::transform_variants<v1::group_item>(arg.base.base.parts); },
                          [&](const v1::part_list& arg) { return studio::shared::transform_variants<v1::group_item>(arg.parts); },
                          [&](const v1::bsp_part& arg) { return studio::shared::transform_variants<v1::group_item>(arg.base.parts); },
                          [&](const v1::cell_anim_part& arg) { return studio::shared::transform_variants<v1::group_item>(arg.base.parts); },
                          [&](const v1::detail_part& arg) { return studio::shared::transform_variants<v1::group_item>(arg.base.parts); } },
        parent);
    };

    const auto get_part_list_item = [](const v1::part_list_item& parent) {
      return std::visit(overloaded{
                          [&](const v1::shape& arg) { return studio::shared::transform_variants<v1::part_list_item>(arg.base.parts); },
                          [&](const v1::an_shape& arg) { return studio::shared::transform_variants<v1::part_list_item>(arg.base.base.parts); },
                          [&](const v1::part_list& arg) { return studio::shared::transform_variants<v1::part_list_item>(arg.parts); },
                          [&](const v1::bsp_part& arg) { return studio::shared::transform_variants<v1::part_list_item>(arg.base.parts); },
                          [&](const v1::cell_anim_part& arg) { return studio::shared::transform_variants<v1::part_list_item>(arg.base.parts); },
                          [&](const v1::detail_part& arg) { return studio::shared::transform_variants<v1::part_list_item>(arg.base.parts); } },
        parent);
    };

    const std::function<void(const v1::part_list_item&)> render_groups = [&](const v1::part_list_item& parent) {
      const auto groups = get_groups(parent);

      for (auto& group : groups)
      {
        const auto faces = visit_group_item(group);
      }

      const auto parents = get_part_list_item(parent);

      for (auto& other : parents)
      {
        render_groups(other);
      }
    };

    render_groups(real_shape);
  }
}// namespace studio::content::dts::three_space