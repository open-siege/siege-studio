#ifndef DARKSTARDTSCONVERTER_UTILTY_HPP
#define DARKSTARDTSCONVERTER_UTILTY_HPP

#include <string_view>
#include <algorithm>
#include <wx/wx.h>
#include <SFML/OpenGL.hpp>

inline bool ends_with(std::string_view value, std::string_view ending)
{
  if (ending.size() > value.size())
  {
    return false;
  }
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

WXWidget get_handle(const wxControl* const control);

void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar);

#endif//DARKSTARDTSCONVERTER_UTILTY_HPP
