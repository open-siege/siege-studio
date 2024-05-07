#ifndef DARKSTARDTSCONVERTER_PAL_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_VIEW_HPP

#include <string>
#include <vector>
#include <siege/content/pal/palette.hpp>

namespace siege::views
{
  class pal_controller
  {
  public:
    struct rect
    {
      int x;
      int y;
      int width;
      int height;
    };

    struct palette
    {
      rect position;
      siege::content::pal::colour colour;
      std::u8string name;
    };    

    static bool is_pal(std::istream& image_stream);

    std::size_t load_palettes(std::istream& image_stream);
  private:
      std::vector<std::vector<palette>> palettes;
  };
}// namespace siege::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
