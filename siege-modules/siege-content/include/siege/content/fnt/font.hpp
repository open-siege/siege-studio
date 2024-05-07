#ifndef INC_3SPACESTUDIO_FONT_HPP
#define INC_3SPACESTUDIO_FONT_HPP

#include <siege/content/bmp/bitmap.hpp>

namespace siege::content::fnt
{
  bool is_phoenix_font(std::istream& raw_data);

  bmp::pbmp_data read_phoenix_font(std::istream& raw_data);
}

#endif//INC_3SPACESTUDIO_FONT_HPP
