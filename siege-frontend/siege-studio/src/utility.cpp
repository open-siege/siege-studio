
#include "utility.hpp"

#ifdef __WXGTK__
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#endif

namespace studio
{
  sf::WindowHandle get_handle(const wxControl& control)
  {
    // TODO fix this code because it doesn't work at the moment.
#ifdef __WXGTK__
    // Currently results in:
    // The error was 'BadAccess (attempt to access private resource denied)'.
    // (Details: serial 57 error_code 10 request_code 2 minor_code 0)
    //return gdk_x11_drawable_get_xid(gtk_widget_get_window(control.GetHandle()));

    // SFML wants a Window, and doesn't appear to deal with XIDs.
    // Making all of this work properly is more time than I have at the moment.
    // Long term, the way SFML is used in the project will be changed (if not removed).
    // Thus, I'm leaving things here for now, to come back to it later.
    return gdk_x11_get_default_root_xwindow();
#else
    return control.GetHandle();
#endif
  }

  void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
  {
    constexpr GLdouble pi = 3.1415926535897932384626433832795;
    GLdouble fW, fH;

    //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
    fH = tan(fovY / 360 * pi) * zNear;
    fW = fH * aspect;

    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
  }
}