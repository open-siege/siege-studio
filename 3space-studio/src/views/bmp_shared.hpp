#ifndef OPEN_SIEGE_BMP_SHARED_HPP
#define OPEN_SIEGE_BMP_SHARED_HPP

#include <list>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include "resources/resource_explorer.hpp"
#include "content/pal/palette.hpp"
#include "content/bmp/bitmap.hpp"

namespace studio::views
{
  struct palette_context
  {
    std::list<std::string> sort_order;
    std::map<std::string_view, std::pair<studio::resources::file_info, std::vector<content::pal::palette>>> loaded_palettes;

    void load_palettes(const studio::resources::resource_explorer& manager, const std::vector<studio::resources::file_info>& palettes);
  };

  enum class bitmap_type
  {
    unknown,
    microsoft,
    earthsiege,
    phoenix
  };

  using bmp_variant = std::variant<studio::content::bmp::windows_bmp_data, std::vector<studio::content::bmp::dbm_data>, std::vector<content::bmp::pbmp_data>>;
  using palette_map = std::map<std::string_view, std::pair<studio::resources::file_info, std::vector<content::pal::palette>>>;

  constexpr static auto auto_generated_name = std::string_view("Auto-generated");

  std::pair<bitmap_type, bmp_variant> load_image_data_for_pal_detection(const studio::resources::file_info& info, std::basic_istream<std::byte>& image_stream);

  std::pair<bitmap_type, bmp_variant> load_image_data(const studio::resources::file_info& info, std::basic_istream<std::byte>& image_stream);

  std::pair<std::string_view, std::size_t> detect_default_palette(
    const bmp_variant& data,
    const studio::resources::file_info& file,
    const studio::resources::resource_explorer& manager,
    const palette_map& loaded_palettes,
    bool ignore_settings = false);

  std::optional<std::pair<std::string_view, std::size_t>> default_palette_from_settings(
    const studio::resources::file_info& file,
    const studio::resources::resource_explorer& manager,
    const palette_map& loaded_palettes);
}

#endif// OPEN_SIEGE_BMP_SHARED_HPP
