#ifndef SIEGE_PAL_CONTROLLER_HPP
#define SIEGE_PAL_CONTROLLER_HPP

#include <string>
#include <vector>
#include <siege/content/pal/palette.hpp>

namespace siege::views
{
  class pal_controller
  {
  public:
    constexpr static auto formats = std::array<std::wstring_view, 4>{{ L".pal", L".ipl", L".ppl", L".dpl"}};
    
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
