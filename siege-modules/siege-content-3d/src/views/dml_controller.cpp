#include "dml_controller.hpp"
#include <siege/content/dts/darkstar.hpp>

namespace siege::views
{
  using namespace siege::content;
  
  bool dml_controller::is_material(std::istream& image_stream)
  {
    return dts::darkstar::is_darkstar_dml(image_stream);
  }

  std::size_t dml_controller::load_material(std::istream& image_stream)
  {
    auto dml = dts::darkstar::read_shape(image_stream);
    
    if (auto* real_dml = std::get_if<dts::darkstar::material_list_variant>(&dml); real_dml)
    {
      material_list.emplace(std::move(*real_dml));
      return 1;
    }
    return 0;
  }
}// namespace siege::views
// namespace siege::views
