#ifndef SIEGE_PAL_CONTROLLER_HPP
#define SIEGE_PAL_CONTROLLER_HPP

#include <string>
#include <vector>
#include <any>
#include <siege/platform/shared.hpp>
#include <siege/content/renderable_shape.hpp>

namespace siege::views
{
  struct shape_context;

  class dts_controller
  {
  public:
    constexpr static auto formats = std::array<siege::fs_string_view, 1>{{ FSL".dts" }};
    static bool is_shape(std::istream& image_stream);

    std::size_t load_shape(std::istream& image_stream);

    void render_shape(std::size_t index, content::shape_renderer& renderer);
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

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
