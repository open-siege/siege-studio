#include <any>
#include <siege/content/dts/darkstar.hpp>
#include <siege/content/dts/3space.hpp>
#include <siege/content/dts/renderable_shape_factory.hpp>
#include <siege/content/dts/null_renderable_shape.hpp>
#include "dts_controller.hpp"

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

namespace siege::views
{
  bool dts_controller::is_shape(std::istream& stream)
  {
    return siege::content::dts::darkstar::is_darkstar_dts(stream)
           || siege::content::dts::three_space::v1::is_3space_dts(stream)
           || siege::content::tmd::is_tmd(stream)
           || siege::content::bnd::is_bnd(stream);
  }

  std::size_t dts_controller::load_shape(std::istream& stream)
  {
    if (siege::content::tmd::is_tmd(stream))
    {
      siege::content::tmd::load_tmd(stream);
    }

    if (siege::content::bnd::is_bnd(stream))
    {
      siege::content::bnd::load_bnd(stream);
    }

    auto shape = content::dts::make_shape(stream);

    if (dynamic_cast<content::dts::null_renderable_shape*>(shape.get()) != nullptr)
    {
      return 0;
    }

    auto detail_levels = shape->get_detail_levels();
    std::vector<std::size_t> selected_detail_levels;
    if (!detail_levels.empty())
    {
      selected_detail_levels.emplace_back(0);
    }

    auto materials = shape->get_materials();

    auto sequences = shape->get_sequences(selected_detail_levels);

    shapes.push_back(shape_context{
      .shape = std::move(shape),
      .detail_levels = std::move(detail_levels),
      .materials = std::move(materials),
      .selected_detail_levels = std::move(selected_detail_levels),
      .sequences = std::move(sequences) });

    return 1;
  }


  void dts_controller::render_shape(std::size_t index, content::shape_renderer& renderer)
  {
    auto& context = shapes[index];
    context.shape->render_shape(renderer, context.selected_detail_levels, context.sequences);
  }
}// namespace siege::views