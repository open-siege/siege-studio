#include <any>
#include <siege/content/dts/darkstar.hpp>
#include <siege/content/dts/3space.hpp>
#include <siege/content/renderable_shape_factory.hpp>
#include <siege/content/null_renderable_shape.hpp>
#include <siege/content/wtb.hpp>
#include "3d_shared.hpp"

namespace siege::content::tmd
{
  bool is_tmd(std::istream& stream);
  std::vector<std::any> load_tmd(std::istream& stream);
}// namespace siege::content::tmd

namespace siege::content::bnd
{
  bool is_bnd(std::istream& stream);
  std::vector<std::any> load_bnd(std::istream& stream);
}// namespace siege::content::bnd

namespace siege::content::mdl
{
  bool is_mdl(std::istream& stream);
  std::any load_mdl(std::istream& stream);

  bool is_md2(std::istream& stream);
  std::any load_md2(std::istream& stream);

  bool is_mdx(std::istream& stream);
  std::any load_mdx(std::istream& stream);
}// namespace siege::content::mdl

namespace siege::content::bwd
{
  bool is_bwd(std::istream& stream);
  std::any load_bwd(std::istream& stream);
}// namespace siege::content::bwd

namespace siege::views
{
  struct shape_data
  {
    // to keep it copyable :(
    std::shared_ptr<content::renderable_shape> shape;
    std::vector<std::string> detail_levels;
    std::vector<content::material> materials;
    std::vector<std::size_t> selected_detail_levels;
    std::vector<content::sequence_info> sequences;
  };

  std::span<const siege::fs_string_view> get_shape_formats() noexcept
  {
    constexpr static auto formats = std::array<siege::fs_string_view, 12>{ { FSL ".dts", FSL ".tmd", FSL ".bnd", FSL ".cwg", FSL ".trk", FSL ".bwd", FSL ".wtb", FSL ".erf", FSL ".mdl", FSL ".md2", FSL ".mdx", FSL ".dkm" } };
    return formats;
  }

  bool is_shape(std::istream& stream) noexcept
  {
    return siege::content::dts::darkstar::is_darkstar_dts(stream)
           || siege::content::dts::three_space::v1::is_3space_dts(stream)
           || siege::content::tmd::is_tmd(stream)
           || siege::content::bnd::is_bnd(stream)
           || siege::content::mdl::is_mdl(stream)
           || siege::content::mdl::is_md2(stream)
           || siege::content::mdl::is_mdx(stream)
           || siege::content::bwd::is_bwd(stream)
           || siege::content::wtb::is_wtb(stream);
  }

  std::size_t load_shape(shape_context& state, std::istream& stream)
  {
    if (siege::content::tmd::is_tmd(stream))
    {
      siege::content::tmd::load_tmd(stream);
    }

    if (siege::content::bnd::is_bnd(stream))
    {
      siege::content::bnd::load_bnd(stream);
    }

    if (siege::content::mdl::is_mdl(stream))
    {
      siege::content::mdl::load_mdl(stream);
    }

    if (siege::content::mdl::is_md2(stream))
    {
      siege::content::mdl::load_md2(stream);
    }

    if (siege::content::mdl::is_mdx(stream))
    {
      siege::content::mdl::load_mdx(stream);
    }

    auto shape = content::dts::make_shape(stream);

    if (dynamic_cast<content::dts::null_renderable_shape*>(shape.get()) != nullptr)
    {
      return {};
    }

    auto detail_levels = shape->get_detail_levels();
    std::vector<std::size_t> selected_detail_levels;
    if (!detail_levels.empty())
    {
      selected_detail_levels.emplace_back(0);
    }

    auto materials = shape->get_materials();

    auto sequences = shape->get_sequences(selected_detail_levels);

    std::vector<shape_data> shapes;
    shapes.push_back(shape_data{
      .shape = std::move(shape),
      .detail_levels = detail_levels,
      .materials = std::move(materials),
      .selected_detail_levels = std::move(selected_detail_levels),
      .sequences = std::move(sequences) });

    auto result = shapes.size();
    state = std::move(shapes);

    return result;
  }

  std::vector<shape_data>* self(shape_context& state)
  {
    return std::any_cast<std::vector<shape_data>>(&state);
  }

  const std::vector<shape_data>* self(const shape_context& state)
  {
    return std::any_cast<std::vector<shape_data>>(&state);
  }

  std::vector<std::string> get_detail_levels_for_shape(const shape_context& state, std::size_t index)
  {
    auto* shapes = self(state);
    auto detail_levels = shapes->at(index).detail_levels;

    if (detail_levels.empty())
    {
      detail_levels.emplace_back("Default Detail Level");
    }

    return detail_levels;
  }

  std::vector<content::sequence_info> get_sequence_info_for_shape(const shape_context& state, std::size_t index)
  {
    auto* shapes = self(state);
    return shapes->at(index).sequences;
  }

  std::vector<std::int32_t> get_sequence_ids_for_shape(const shape_context& state, std::size_t index)
  {
    auto* shapes = self(state);
    auto& shape = shapes->at(index);
    std::vector<std::int32_t> results;
    results.reserve(shape.sequences.size());

    std::transform(shape.sequences.begin(), shape.sequences.end(), std::back_inserter(results), [](const auto& seq) {
      return seq.index;
    });

    return results;
  }

  std::vector<std::size_t> get_selected_detail_levels(const shape_context& state, std::size_t index)
  {
    auto* shapes = self(state);
    return shapes->at(index).selected_detail_levels;
  }

  void set_selected_detail_levels(shape_context& state, std::size_t index, std::span<std::size_t> span)
  {
    auto* shapes = self(state);
    auto& shape = shapes->at(index);
    shape.selected_detail_levels.resize(span.size());
    std::copy(span.begin(), span.end(), shape.selected_detail_levels.begin());
  }

  bool is_sequence_enabled(const shape_context& state, std::size_t shape_index, std::int32_t index)
  {
    auto* shapes = self(state);
    
    auto& shape = shapes->at(shape_index);

    for (auto& sequence : shape.sequences)
    {
      if (sequence.index == index)
      {
        return sequence.enabled;
      }
    }
    return false;
  }

  void enable_sequence(shape_context& state, std::size_t shape_index, std::int32_t index)
  {
    auto* shapes = self(state);
    
    auto& shape = shapes->at(shape_index);

    for (auto& sequence : shape.sequences)
    {
      sequence.enabled = sequence.index == index;

      for (auto& sub : sequence.sub_sequences)
      {
        sub.enabled = sequence.enabled;
      }
    }
  }

  void disable_sequence(shape_context& state, std::size_t shape_index, std::int32_t index)
  {
    auto* shapes = self(state);
    
    auto& shape = shapes->at(shape_index);

    for (auto& sequence : shape.sequences)
    {
      if (sequence.index != index)
      {
        continue;
      }
      sequence.enabled = false;

      for (auto& sub : sequence.sub_sequences)
      {
        sub.enabled = sequence.enabled;
      }
    }
  }

  void advance_sequence(shape_context& state, std::size_t shape_index, std::int32_t index)
  {
    auto* shapes = self(state);
    auto& shape = shapes->at(shape_index);

    auto& sequence = shape.sequences.at(index);

    for (auto& sub : sequence.sub_sequences)
    {
      sub.frame_index++;
      if (sub.frame_index >= sub.num_key_frames)
      {
        sub.frame_index = 0;
      }
    }
  }

  void render_shape(const shape_context& state, std::size_t index, content::shape_renderer& renderer)
  {
    auto* shapes = self(state);
    auto& context = shapes->operator[](index);
    context.shape->render_shape(renderer, context.selected_detail_levels, context.sequences);
  }
}// namespace siege::views