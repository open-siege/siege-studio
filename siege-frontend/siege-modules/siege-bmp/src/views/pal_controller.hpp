#ifndef DARKSTARDTSCONVERTER_PAL_VIEW_HPP
#define DARKSTARDTSCONVERTER_PAL_VIEW_HPP

#include <string>
#include <vector>
#include "content/pal/palette.hpp"

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
      studio::content::pal::colour colour;
      std::u8string name;
    };    

    bool is_pal(std::istream& image_stream) const;

    std::size_t load_palettes(std::istream& image_stream);
  private:
      std::vector<std::vector<palette>> palettes;
  };
}// namespace studio::views

#endif//DARKSTARDTSCONVERTER_PAL_VIEW_HPP
