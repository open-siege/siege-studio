#ifndef INC_3SPACESTUDIO_FONT_HPP
#define INC_3SPACESTUDIO_FONT_HPP

#include "content/bmp/bitmap.hpp"

namespace studio::content::fnt
{
  bool is_phoenix_font(std::basic_istream<std::byte>& raw_data);

  bmp::pbmp_data read_phoenix_font(std::basic_istream<std::byte>& raw_data);
}

#endif//INC_3SPACESTUDIO_FONT_HPP
