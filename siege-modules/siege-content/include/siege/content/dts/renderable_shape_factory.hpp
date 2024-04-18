#ifndef INC_3SPACESTUDIO_RENDERABLE_SHAPE_FACTORY_HPP
#define INC_3SPACESTUDIO_RENDERABLE_SHAPE_FACTORY_HPP

namespace siege::content::dts
{
  std::unique_ptr<content::renderable_shape> make_shape(std::istream& shape_stream);
}

#endif//INC_3SPACESTUDIO_RENDERABLE_SHAPE_FACTORY_HPP
