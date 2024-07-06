#ifndef SIEGE_PAL_CONTROLLER_HPP
#define SIEGE_PAL_CONTROLLER_HPP

#include <string>
#include <vector>
#include <siege/platform/shared.hpp>
#include <siege/content/pal/palette.hpp>

namespace siege::views
{
  class pal_controller
  {
  public:
    constexpr static auto formats = std::array<siege::fs_string_view, 4>{{ FSL".pal", FSL".ipl", FSL".ppl", FSL".dpl"}};
    
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

    static bool is_pal(std::istream& image_stream);

    std::size_t load_palettes(std::istream& image_stream);

    const std::vector<colour_entry>& get_palette(std::size_t) const;
  private:
      std::vector<std::vector<colour_entry>> palettes;
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
