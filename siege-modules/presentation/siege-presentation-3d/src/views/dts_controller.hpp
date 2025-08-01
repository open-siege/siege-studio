#ifndef SIEGE_PAL_CONTROLLER_HPP
#define SIEGE_PAL_CONTROLLER_HPP

#include <string>
#include <vector>
#include <any>
#include <span>
#include <siege/platform/shared.hpp>
#include <siege/content/renderable_shape.hpp>

namespace siege::views
{
  struct shape_context;

  class dts_controller
  {
  public:
    constexpr static auto formats = std::array<siege::fs_string_view, 12>{ { FSL ".dts", FSL ".tmd", FSL ".bnd", FSL ".cwg", FSL ".trk", FSL ".bwd", FSL ".wtb", FSL ".erf", FSL ".mdl", FSL ".md2", FSL ".mdx", FSL ".dkm" } };
    static bool is_shape(std::istream& image_stream);

    std::size_t load_shape(std::istream& image_stream);

    std::vector<std::string> get_detail_levels_for_shape(std::size_t) const;

    std::vector<content::sequence_info> get_sequence_info_for_shape(std::size_t) const;

    std::vector<std::int32_t> get_sequence_ids_for_shape(std::size_t) const;

    std::vector<std::size_t> get_selected_detail_levels(std::size_t) const;

    void set_selected_detail_levels(std::size_t, std::span<std::size_t> span);

    void render_shape(std::size_t index, content::shape_renderer& renderer);

    void enable_sequence(std::size_t shape, std::int32_t index);

    void disable_sequence(std::size_t shape, std::int32_t index);

    void advance_sequence(std::size_t shape, std::int32_t index);

    bool is_sequence_enabled(std::size_t shape, std::int32_t index) const;
  private:
    std::vector<shape_context> shapes;
  };

  struct shape_context
  {
    std::unique_ptr<content::renderable_shape> shape;
    std::vector<std::string> detail_levels;
    std::vector<content::material> materials;
    std::vector<std::size_t> selected_detail_levels;
    std::vector<content::sequence_info> sequences;
  };

}// namespace siege::views

#endif// DARKSTARDTSCONVERTER_PAL_VIEW_HPP
