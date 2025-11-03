#include "3d_shared.hpp"
#include <siege/content/dts/darkstar.hpp>

namespace siege::views
{
  using namespace siege::content;

  bool is_material(std::istream& image_stream)
  {
    return dts::darkstar::is_darkstar_dml(image_stream);
  }
  std::span<const siege::fs_string_view> get_material_formats() noexcept
  {
    constexpr static auto formats = std::array<siege::fs_string_view, 1>{ { FSL ".dml" } };
    return formats;
  }

  std::size_t load_material(material_context& state, std::istream& image_stream)
  {
    auto dml = dts::darkstar::read_shape(image_stream);

    if (auto* real_dml = std::get_if<dts::darkstar::material_list_variant>(&dml); real_dml)
    {
      state.emplace<siege::content::dts::darkstar::material_list_variant>(std::move(*real_dml));

      auto real_material = std::any_cast<siege::content::dts::darkstar::material_list_variant>(&state);

      return std::visit([](auto& object) {
        return object.materials.size();
      },
        *real_material);
    }
    return 0;
  }

  std::optional<std::filesystem::path> get_filename(const material_context& state, std::size_t index)
  {
    auto real_material = std::any_cast<siege::content::dts::darkstar::material_list_variant>(&state);

    if (!real_material)
    {
      return std::nullopt;
    }

    return std::visit([=](auto& object) -> std::optional<std::filesystem::path> {
      auto& material = object.materials[index];

      if (std::find(material.file_name.begin(), material.file_name.end(), '\0') == material.file_name.begin())
      {
        return std::nullopt;
      }

      return std::filesystem::path(material.file_name.data());
    },
      *real_material);
  }

}// namespace siege::views
// namespace siege::views
