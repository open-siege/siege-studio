#ifndef DARKSTARDTSCONVERTER_DTS_RENDER_HPP
#define DARKSTARDTSCONVERTER_DTS_RENDER_HPP

#include <iterator>
#include <algorithm>
#include <utility>
#include <optional>
#include <map>
#include <unordered_map>
#include <set>
#include <memory_resource>

#include <siege/content/renderable_shape.hpp>
#include "darkstar_structures.hpp"

namespace siege::content::dts::darkstar
{
  class dts_renderable_shape final : public renderable_shape
  {
  public:
    dts_renderable_shape(shape_variant shape)
      : shape(std::move(shape))
    {
    }

    std::vector<sequence_info> get_sequences(const std::vector<std::size_t>& detail_level_indexes) const override;
    std::vector<std::string> get_detail_levels() const override;
    std::vector<material> get_materials() const override;

    void render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes, const std::vector<sequence_info>& sequences) const override;

  private:
    shape_variant shape;
  };
}

#endif//DARKSTARDTSCONVERTER_DTS_RENDER_HPP
