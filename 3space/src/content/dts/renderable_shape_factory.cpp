#include "content/dts/darkstar.hpp"
#include "content/dts/3space.hpp"
#include "content/dts/dts_renderable_shape.hpp"
#include "content/dts/3space_renderable_shape.hpp"
#include "content/dts/null_renderable_shape.hpp"

namespace studio::content::dts
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

    return std::make_unique<null_renderable_shape>();
  }
}