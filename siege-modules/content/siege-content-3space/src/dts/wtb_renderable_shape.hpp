#ifndef SIEGE_WTB_RENDERABLE_SHAPE_HPP
#define SIEGE_WTB_RENDERABLE_SHAPE_HPP

#include <iterator>
#include <algorithm>
#include <utility>
#include <optional>
#include <map>
#include <unordered_map>
#include <set>
#include <variant>

#include <siege/content/renderable_shape.hpp>
#include <siege/content/dts/wtb.hpp>
#include <siege/content/dts/bwd.hpp>

namespace siege::content::wtb
{
  class wtb_renderable_shape final : public renderable_shape
  {
  public:
    wtb_renderable_shape(wtb_shape shape)
      : shape(std::move(shape))
    {
    }

    wtb_renderable_shape(bwd::bwd_model shape)
      : shape(std::move(shape))
    {
    }

    std::vector<sequence_info> get_sequences(const std::vector<std::size_t>& detail_level_indexes) const override;
    std::vector<std::string> get_detail_levels() const override;
    std::vector<material> get_materials() const override;

    void render_shape(shape_renderer& renderer, const std::vector<std::size_t>& detail_level_indexes, const std::vector<sequence_info>& sequences) const override;

  private:
    std::variant<wtb_shape, bwd::bwd_model> shape;
  };
}// namespace siege::content::wtb

#endif// DARKSTARDTSCONVERTER_DTS_RENDER_HPP
