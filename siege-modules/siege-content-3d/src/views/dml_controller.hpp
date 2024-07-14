#ifndef DML_CONTROLLER_HPP
#define DML_CONTROLLER_HPP

#include <array>
#include <siege/platform/shared.hpp>
#include <siege/content/dts/darkstar.hpp>

namespace siege::views
{
  class dml_controller
  {
  public:
    constexpr static auto formats = std::array<siege::fs_string_view, 1>{ { FSL ".dml" } };

    static bool is_material(std::istream& image_stream);

    std::size_t load_material(std::istream& image_stream);
    private:
    std::optional<siege::content::dts::darkstar::material_list_variant> material_list;
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_DARKSTAR_DTS_VIEW_HPP
