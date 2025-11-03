#include <siege/content/dts/darkstar.hpp>
#include <siege/content/dts/3space.hpp>
#include <siege/content/bwd.hpp>
#include <siege/content/wtb_renderable_shape.hpp>
#include <siege/content/dts/dts_renderable_shape.hpp>
#include <siege/content/dts/3space_renderable_shape.hpp>
#include <siege/content/null_renderable_shape.hpp>

namespace siege::content::dts
{
  std::unique_ptr<content::renderable_shape> make_shape(std::istream& shape_stream)
  {
    if (content::dts::darkstar::is_darkstar_dts(shape_stream))
    {
      return std::make_unique<darkstar::dts_renderable_shape>(content::dts::darkstar::read_shape(shape_stream, std::nullopt));
    }

    if (content::dts::three_space::v1::is_3space_dts(shape_stream))
    {
      auto shapes = content::dts::three_space::v1::read_shapes(shape_stream);

      if (shapes.empty())
      {
        return std::make_unique<null_renderable_shape>();
      }

      return std::make_unique<three_space::dts_renderable_shape>(shapes.front());
    }

    if (content::wtb::is_wtb(shape_stream))
    {
      return std::make_unique<wtb::wtb_renderable_shape>(content::wtb::load_wtb(shape_stream));
    }

    if (content::bwd::is_bwd(shape_stream))
    {
      return std::make_unique<wtb::wtb_renderable_shape>(content::bwd::load_bwd(shape_stream));
    }

    return std::make_unique<null_renderable_shape>();
  }
}// namespace siege::content::dts