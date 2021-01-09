#ifndef DARKSTARDTSCONVERTER_UTILTY_HPP
#define DARKSTARDTSCONVERTER_UTILTY_HPP

#include <string>
#include <string_view>
#include <algorithm>
#include <locale>
#include <wx/wx.h>
#include <SFML/OpenGL.hpp>

inline void default_wx_deleter(wxWindowBase* control)
{
  if (!control->IsBeingDeleted())
  {
    delete control;
  }
}

inline std::string to_lower(std::string_view some_string, const std::locale& locale)
{
  std::string result(some_string);
  std::transform(result.begin(), result.end(), result.begin(), [&](auto c) { return std::tolower(c, locale); });
  return result;
}

inline std::string to_lower(std::string_view some_string)
{
  std::string result(some_string);
  std::transform(result.begin(), result.end(), result.begin(), [&](auto c) { return std::tolower(c, std::locale()); });
  return result;
}

inline bool ends_with(std::string_view value, std::string_view ending)
{
  if (ending.size() > value.size())
  {
    return false;
  }
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

WXWidget get_handle(const wxControl& control);

void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar);

#endif//DARKSTARDTSCONVERTER_UTILTY_HPP
