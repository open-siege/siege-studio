#ifndef INC_3SPACESTUDIO_NULL_RENDERABLE_SHAPE_HPP
#define INC_3SPACESTUDIO_NULL_RENDERABLE_SHAPE_HPP

#include <siege/content/renderable_shape.hpp>

namespace siege::content::dts
{
  struct null_renderable_shape : public renderable_shape
  {
    null_renderable_shape()
    {
    }

    std::vector<sequence_info> get_sequences(const std::vector<std::size_t>& detail_level_indexes) const override
    {
      return {};
    }

    std::vector<std::string> get_detail_levels() const override
    {
      return {};
    }

    std::vector<material> get_materials() const override
    {
      return {};
    }

    void render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes, const std::vector<sequence_info>& sequences) const override
    {
    }
  };
}

#endif//INC_3SPACESTUDIO_NULL_RENDERABLE_SHAPE_HPP
