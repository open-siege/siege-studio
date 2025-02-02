#ifndef DARKSTARDTSCONVERTER_BMP_VIEW_HPP
#define DARKSTARDTSCONVERTER_BMP_VIEW_HPP

#include <future>
#include <set>
#include <map>
#include <variant>
#include <utility>
#include <optional>
#include <filesystem>
#include <deque>
#include <siege/content/pal/palette.hpp>
#include <siege/platform/image.hpp>
#include "bmp_shared.hpp"

namespace siege::views
{
  struct palette_info
  {
    std::filesystem::path path;
    std::vector<content::pal::palette> children;
  };

  class bmp_controller
  {
  public:
    constexpr static auto formats = std::array<siege::fs_string_view, 20>{ { FSL ".jpg",
      FSL ".jpeg",
      FSL ".gif",
      FSL ".png",
      FSL ".tag",
      FSL ".bmp",
      FSL ".dds",
      FSL ".ico",
      FSL ".tiff",
      FSL ".dib",
      FSL ".tim",
      FSL ".pba",
      FSL ".dmb",
      FSL ".db0",
      FSL ".db1",
      FSL ".db2",
      FSL ".hba",
      FSL ".hb0",
      FSL ".hb1",
      FSL ".hb2" } };

    static bool is_bmp(std::istream& image_stream) noexcept;

    using get_embedded_pal_filenames = std::set<std::filesystem::path>(std::filesystem::path);
    using resolve_embedded_pal = std::vector<char>(std::filesystem::path);

    std::future<const std::deque<palette_info>&>
      load_palettes_async(std::optional<std::filesystem::path> folder_hint, std::move_only_function<get_embedded_pal_filenames>, std::move_only_function<resolve_embedded_pal>);

    std::size_t load_bitmap(std::istream& image_stream, const std::future<const std::deque<palette_info>&>&) noexcept;

    enum class colour_strategy : int
    {
      do_nothing,
      remap,
      remap_unique
    };

    using size = siege::platform::bitmap::size;

    size get_size(std::size_t frame) const noexcept;

    std::size_t get_frame_count() const noexcept
    {
      if (original_image)
      {
        return original_image->frame_count();
      }
      return 0;
    }

    std::size_t convert(std::size_t frame, size size, int bits, std::span<std::byte> destination) const noexcept;

  private:
    std::optional<platform::bitmap::platform_image> original_image;
    std::deque<palette_info> palettes;
    std::deque<palette_info>::iterator selected_palette_file;
    std::size_t selected_palette;

  public:
    inline auto get_selected_palette() const
    {
      return std::make_pair(selected_palette_file, selected_palette);
    }
  };

}// namespace siege::views
#endif// DARKSTARDTSCONVERTER_BMP_VIEW_HPP
