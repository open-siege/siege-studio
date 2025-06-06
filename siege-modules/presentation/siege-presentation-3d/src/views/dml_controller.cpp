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

      return std::visit([](auto& object) {
        return object.materials.size();
      },
        *material_list);
    }
    return 0;
  }

  std::optional<std::filesystem::path> dml_controller::get_filename(std::size_t index)
  {
    return std::visit([=](auto& object) -> std::optional<std::filesystem::path> {
      auto& material = object.materials[index];

      if (std::find(material.file_name.begin(), material.file_name.end(), '\0') == material.file_name.begin())
      {
        return std::nullopt;
      }

      return std::filesystem::path(material.file_name.data());
    }, *material_list);
  }

}// namespace siege::views
// namespace siege::views
