#ifndef OPEN_SIEGE_BMP_SHARED_HPP
#define OPEN_SIEGE_BMP_SHARED_HPP

#include <list>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
//#include "siege/resource/resource_explorer.hpp"
#include <siege/platform/archive_plugin.hpp>
#include <siege/content/pal/palette.hpp>
#include <siege/content/bmp/bitmap.hpp>
#include <siege/content/json_boost.hpp>

namespace siege::views
{
  //struct palette_context
  //{
  //  std::list<std::string> sort_order;
  //  std::map<std::string_view, std::pair<siege::platform::file_info, std::vector<content::pal::palette>>> loaded_palettes;

  //  void load_palettes(const siege::resource::resource_explorer& manager, const std::vector<siege::platform::file_info>& palettes);
  //};

  //enum class bitmap_type
  //{
  //  unknown,
  //  microsoft,
  //  earthsiege,
  //  phoenix
  //};

  //using bmp_variant = std::variant<siege::content::bmp::windows_bmp_data, std::vector<siege::content::bmp::dbm_data>, std::vector<content::bmp::pbmp_data>>;
  //using palette_map = std::map<std::string_view, std::pair<siege::platform::file_info, std::vector<content::pal::palette>>>;

  //constexpr static auto auto_generated_name = std::string_view("Auto-generated");

  //std::string get_palette_key(const siege::resource::resource_explorer& explorer,
  //  const siege::platform::file_info& file);

  //void set_default_palette(const siege::resource::resource_explorer& manager,
  //  const std::string& key,
  //  std::string_view name,
  //  nlohmann::json& settings,
  //  std::optional<std::size_t> index = std::nullopt);

  //void set_default_palette(const siege::resource::resource_explorer& manager,
  //  const std::string& key,
  //  std::string_view name,
  //  std::optional<std::size_t> index = std::nullopt);

  //void set_default_palette(const siege::resource::resource_explorer& manager,
  //  const siege::platform::file_info& file,
  //  std::string_view name,
  //  std::optional<std::size_t> index = std::nullopt);

  //void set_selected_palette(const siege::resource::resource_explorer& manager,
  //  const std::string& key,
  //  std::string_view name,
  //  nlohmann::json& settings,
  //  std::optional<std::size_t> index = std::nullopt);

  //void set_selected_palette(const siege::resource::resource_explorer& manager,
  //  const std::string& key,
  //  std::string_view name,
  //  std::optional<std::size_t> index = std::nullopt);

  //void set_selected_palette(const siege::resource::resource_explorer& manager,
  //  const siege::platform::file_info& file,
  //  std::string_view name,
  //  std::optional<std::size_t> index = std::nullopt);

  //std::pair<bitmap_type, bmp_variant> load_image_data_for_pal_detection(const siege::platform::file_info& info, std::istream& image_stream);

  //std::pair<bitmap_type, bmp_variant> load_image_data(const siege::platform::file_info& info, std::istream& image_stream);

  //std::pair<std::string_view, std::size_t> detect_default_palette(
  //  const bmp_variant& data,
  //  const siege::platform::file_info& file,
  //  const siege::resource::resource_explorer& manager,
  //  const palette_map& loaded_palettes,
  //  bool ignore_settings = false);

  //std::optional<std::pair<std::string_view, std::size_t>> default_palette_from_settings(
  //  const siege::platform::file_info& file,
  //  const siege::resource::resource_explorer& manager,
  //  const palette_map& loaded_palettes);

  //std::optional<std::pair<std::string_view, std::size_t>> selected_palette_from_settings(
  //  const siege::platform::file_info& file,
  //  const siege::resource::resource_explorer& manager,
  //  const palette_map& loaded_palettes);
}

#endif// OPEN_SIEGE_BMP_SHARED_HPP
