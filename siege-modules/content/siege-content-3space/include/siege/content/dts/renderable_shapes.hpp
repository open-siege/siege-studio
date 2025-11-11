#ifndef DTS_RENDERABLE_SHAPE
#define DTS_RENDERABLE_SHAPE

#include <siege/content/renderable_shape.hpp>
#include <siege/content/dts/darkstar_structures.hpp>
#include <siege/content/dts/3space.hpp>

namespace siege::content::dts::darkstar
{
  renderable_shape_value make_renderable_shape(shape_variant);
}

namespace siege::content::dts::three_space
{
  renderable_shape_value make_renderable_shape(v1::shape_item);
}


#endif// DTS_RENDERABLE_SHAPE
