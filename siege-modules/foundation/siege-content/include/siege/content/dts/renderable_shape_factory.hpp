#ifndef RENDERABLE_SHAPE_FACTORY_HPP
#define RENDERABLE_SHAPE_FACTORY_HPP

#include <siege/content/renderable_shape.hpp>

namespace siege::content::dts
{
  std::unique_ptr<content::renderable_shape> make_shape(std::istream& shape_stream);
}

#endif//INC_3SPACESTUDIO_RENDERABLE_SHAPE_FACTORY_HPP
