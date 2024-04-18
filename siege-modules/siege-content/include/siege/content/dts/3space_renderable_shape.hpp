#ifndef INC_3SPACESTUDIO_3SPACE_RENDERABLE_SHAPE_HPP
#define INC_3SPACESTUDIO_3SPACE_RENDERABLE_SHAPE_HPP

#include "3space.hpp"
#include <siege/content/renderable_shape.hpp>

namespace siege::content::dts::three_space
{
  class dts_renderable_shape : public renderable_shape
  {
  public:
    dts_renderable_shape(v1::shape_item shape)
      : shape(std::move(shape))
    {
    }

    std::vector<sequence_info> get_sequences(const std::vector<std::size_t>& detail_level_indexes) const override;
    std::vector<std::string> get_detail_levels() const override;
    std::vector<material> get_materials() const override;

    void render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes, const std::vector<sequence_info>& sequences) const override;

  private:
    v1::shape_item shape;
  };
}

#endif//INC_3SPACESTUDIO_3SPACE_RENDERABLE_SHAPE_HPP
