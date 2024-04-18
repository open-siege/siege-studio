#ifndef OPEN_SIEGE_IMAGE_HPP
#define OPEN_SIEGE_IMAGE_HPP

#include <fstream>
#include <siege/content/bmp/bitmap.hpp"
#include "siege/resource/archive_plugin.hpp"

namespace siege::content::bmp
{
  bool is_png(std::istream& raw_data);
  bool is_jpg(std::istream& raw_data);
  bool is_gif(std::istream& raw_data);
  bool is_tga(std::istream& raw_data);

  windows_bmp_data get_png_data(const siege::platform::file_info& info, std::istream& raw_data);
  windows_bmp_data get_jpg_data(const siege::platform::file_info& info, std::istream& raw_data);
  windows_bmp_data get_gif_data(const siege::platform::file_info& info, std::istream& raw_data);
  windows_bmp_data get_tga_data(const siege::platform::file_info& info, std::istream& raw_data);
}

#endif// OPEN_SIEGE_IMAGE_HPP
