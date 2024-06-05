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
#include <siege/content/bmp/image.hpp>
#include "bmp_shared.hpp"

namespace siege::views
{
  class bmp_controller 
  {
  public:
      constexpr static auto formats = std::array<std::wstring_view, 16>{{
        L".jpg", L".jpeg", L".gif", L".png", L".tag", L".bmp", L".dib" , L".pba", L".dmb", L".db0", L".db1", L".db2", L".hba", L".hb0", L".hb1", L".hb2"    
        }};

      static bool is_bmp(std::istream& image_stream) noexcept;

      using get_embedded_pal_filenames = std::set<std::filesystem::path>(std::filesystem::path);
      using resolve_embedded_pal = std::vector<char>(std::filesystem::path);

      std::future<void>
          load_palettes_async(std::optional<std::filesystem::path> folder_hint, std::move_only_function<get_embedded_pal_filenames>, std::move_only_function<resolve_embedded_pal>);

      std::size_t load_bitmap(std::istream& image_stream) noexcept;
    
      enum class colour_strategy : int
      {
        do_nothing,
        remap,
        remap_unique
      };

      std::size_t convert(std::size_t frame, std::pair<int, int> size, int bits, std::span<std::byte> destination) const noexcept;
  private:
        std::optional<content::bmp::platform_image> original_image;
        std::deque<std::vector<content::pal::palette>> palettes;
  };

}
#endif//DARKSTARDTSCONVERTER_BMP_VIEW_HPP
