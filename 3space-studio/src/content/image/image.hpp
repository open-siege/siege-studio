#ifndef OPEN_SIEGE_IMAGE_HPP
#define OPEN_SIEGE_IMAGE_HPP

#include <fstream>
#include "content/bmp/bitmap.hpp"
#include "resources/archive_plugin.hpp"

namespace studio::content::bmp
{
  bool is_png(std::basic_istream<std::byte>& raw_data);
  bool is_jpg(std::basic_istream<std::byte>& raw_data);
  bool is_gif(std::basic_istream<std::byte>& raw_data);
  bool is_tga(std::basic_istream<std::byte>& raw_data);

  windows_bmp_data get_png_data(const studio::resources::file_info& info, std::basic_istream<std::byte>& raw_data);
  windows_bmp_data get_jpg_data(const studio::resources::file_info& info, std::basic_istream<std::byte>& raw_data);
  windows_bmp_data get_gif_data(const studio::resources::file_info& info, std::basic_istream<std::byte>& raw_data);
  windows_bmp_data get_tga_data(const studio::resources::file_info& info, std::basic_istream<std::byte>& raw_data);
}

#endif// OPEN_SIEGE_IMAGE_HPP
