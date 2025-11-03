#ifndef SIEGE_DTS_SHARED_HPP
#define SIEGE_DTS_SHARED_HPP

#include <string>
#include <vector>
#include <any>
#include <span>
#include <siege/platform/shared.hpp>
#include <siege/content/renderable_shape.hpp>

namespace siege::views
{
  std::span<const siege::fs_string_view> get_shape_formats() noexcept;
  bool is_shape(std::istream& image_stream) noexcept;

  std::span<const siege::fs_string_view> get_material_formats() noexcept;
  bool is_material(std::istream& image_stream);

  struct material_context : std::any
  {
    using std::any::any;
  };

  std::size_t load_material(material_context&, std::istream& image_stream);
  std::optional<std::filesystem::path> get_filename(const material_context&, std::size_t index);

  struct shape_context : public std::any
  {
    using std::any::any;
  };

  std::size_t load_shape(shape_context&, std::istream& image_stream);

  std::vector<std::string> get_detail_levels_for_shape(const shape_context&, std::size_t);

  std::vector<content::sequence_info> get_sequence_info_for_shape(const shape_context&, std::size_t);

  std::vector<std::int32_t> get_sequence_ids_for_shape(const shape_context&, std::size_t);

  std::vector<std::size_t> get_selected_detail_levels(const shape_context&, std::size_t);

  void set_selected_detail_levels(shape_context&, std::size_t, std::span<std::size_t> span);

  void render_shape(const shape_context&, std::size_t index, content::shape_renderer& renderer);

  void enable_sequence(shape_context&, std::size_t shape, std::int32_t index);

  void disable_sequence(shape_context&, std::size_t shape, std::int32_t index);

  void advance_sequence(shape_context&, std::size_t shape, std::int32_t index);

  bool is_sequence_enabled(const shape_context&, std::size_t shape, std::int32_t index);
}// namespace siege::views

#endif// DARKSTARDTSCONVERTER_PAL_VIEW_HPP
