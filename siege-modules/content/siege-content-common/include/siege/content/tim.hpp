#ifndef SIEGE_TIM_HPP
#define SIEGE_TIM_HPP

#include <siege/platform/bitmap.hpp>

namespace siege::content::tim
{
  bool is_tim(std::istream& raw_data);

  platform::bitmap::windows_bmp_data get_tim_data_as_bitmap(std::istream& raw_data);
}// namespace siege::content::tim
#endif