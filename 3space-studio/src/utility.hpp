#ifndef DARKSTARDTSCONVERTER_UTILTY_HPP
#define DARKSTARDTSCONVERTER_UTILTY_HPP

#include <string>
#include <string_view>
#include <algorithm>
#include <locale>
#include <wx/wx.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window/WindowHandle.hpp>

namespace studio
{
  inline void default_wx_deleter(wxWindowBase* control)
  {
    if (!control->IsBeingDeleted())
    {
      delete control;
    }
  }

  sf::WindowHandle get_handle(const wxControl& control);

  void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar);
}
#endif//DARKSTARDTSCONVERTER_UTILTY_HPP
