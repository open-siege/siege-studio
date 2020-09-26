#ifndef DARKSTARDTSCONVERTER_UTILTY_HPP
#define DARKSTARDTSCONVERTER_UTILTY_HPP

#include <wx/wx.h>
#include <SFML/OpenGL.hpp>

WXWidget get_handle(const wxControl* const control);

void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar);

#endif//DARKSTARDTSCONVERTER_UTILTY_HPP
