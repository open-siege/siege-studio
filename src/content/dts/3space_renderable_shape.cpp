#include "3space_renderable_shape.hpp"
#include "shared.hpp"

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

  }
}// namespace studio::content::dts::three_space