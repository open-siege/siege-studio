#ifndef INC_3SPACESTUDIO_RENDERABLE_SHAPE_FACTORY_HPP
#define INC_3SPACESTUDIO_RENDERABLE_SHAPE_FACTORY_HPP

namespace studio::content::dts
{
  std::unique_ptr<content::renderable_shape> make_shape(std::basic_istream<std::byte>& shape_stream);
}

#endif//INC_3SPACESTUDIO_RENDERABLE_SHAPE_FACTORY_HPP
