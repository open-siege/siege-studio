#ifndef OPEN_SIEGE_PAL_SETTINGS_HPP
#define OPEN_SIEGE_PAL_SETTINGS_HPP

#include <fstream>

namespace studio::content::pal
{
  bool is_pal_settings_file(std::basic_istream<std::byte>& raw_data);
}

#endif// OPEN_SIEGE_PAL_SETTINGS_HPP
