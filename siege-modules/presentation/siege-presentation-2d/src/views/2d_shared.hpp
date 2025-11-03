#ifndef SIEGE_2D_SHARED_HPP
#define SIEGE_2D_SHARED_HPP

#include <string>
#include <vector>
#include <any>
#include <span>
#include <future>
#include <deque>
#include <siege/platform/shared.hpp>
#include <siege/content/pal/palette.hpp>
#include <siege/platform/image.hpp>

namespace siege::views
{
  struct pal_context : std::any
  {
    using std::any::any;
  };

  struct rect
  {
    int x;
    int y;
    int width;
    int height;
  };

  struct colour_entry
  {
    rect position;
    siege::content::pal::colour colour;
    std::u8string name;
  };

  std::span<const siege::fs_string_view> get_pal_formats() noexcept;
  bool is_pal(std::istream& image_stream) noexcept;
  std::size_t load_palettes(pal_context&, std::istream& image_stream);

  std::span<const colour_entry> get_palette(const pal_context&, std::size_t);

  struct bmp_context : std::any
  {
    using std::any::any;
  };

  struct palette_info
  {
    std::filesystem::path path;
    std::vector<content::pal::palette> children;
  };

  using size = siege::platform::bitmap::size;
  using get_embedded_pal_filenames = std::set<std::filesystem::path>(std::filesystem::path);
  using resolve_embedded_pal = std::vector<char>(std::filesystem::path);


  enum class colour_strategy : int
  {
    do_nothing,
    remap,
    remap_unique
  };

  std::span<const siege::fs_string_view> get_bmp_formats() noexcept;
  bool is_bmp(std::istream& image_stream) noexcept;

  std::shared_future<const std::deque<palette_info>&> load_palettes_async(bmp_context&, std::optional<std::filesystem::path> folder_hint, std::move_only_function<get_embedded_pal_filenames>, std::move_only_function<resolve_embedded_pal>);

  std::size_t load_bitmap(bmp_context&, std::istream& image_stream, std::shared_future<const std::deque<palette_info>&>) noexcept;

  size get_size(const bmp_context&, std::size_t frame) noexcept;
  std::size_t get_frame_count(const bmp_context&) noexcept;
  std::pair<const palette_info&, std::size_t> get_selected_palette(const bmp_context&);

#if WIN32
  win32::wic::bitmap_source& get_frame(bmp_context& state, std::size_t frame);
#endif
}// namespace siege::views

#endif// DARKSTARDTSCONVERTER_PAL_VIEW_HPP
