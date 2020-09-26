#ifndef DARKSTARDTSCONVERTER_UTILTY_HPP
#define DARKSTARDTSCONVERTER_UTILTY_HPP

#include <wx/wx.h>

#ifdef __WXGTK__
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <wx/gtk/win_gtk.h>
#endif

#include <SFML/OpenGL.hpp>

auto get_handle(const wxControl* const control)
{
#ifdef __WXGTK__

  // GTK implementation requires to go deeper to find the
  // low-level X11 identifier of the widget
  gtk_widget_realize(m_wxwindow);
  gtk_widget_set_double_buffered(m_wxwindow, false);
  GdkWindow* Win = GTK_PIZZA(m_wxwindow)->bin_window;
  XFlush(GDK_WINDOW_XDISPLAY(Win));
        return GDK_WINDOW_XWINDOW(Win));
#else
  return control->GetHandle();
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

#endif//DARKSTARDTSCONVERTER_UTILTY_HPP
