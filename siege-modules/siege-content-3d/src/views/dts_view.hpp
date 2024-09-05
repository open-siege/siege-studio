#ifndef DTS_VIEW_HPP
#define DTS_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include "dts_controller.hpp"
#include "gl_renderer.hpp"
#include <glm/ext/vector_float3.hpp>


namespace siege::views
{
  struct opengl_deleter
  {
    void operator()(HGLRC gl_context)
    {
      assert(::wglDeleteContext(gl_context) == TRUE);
    }
  };

  struct dts_view final : win32::window_ref
    , win32::static_control::notifications
  {
    dts_controller controller;

    win32::static_control render_view;
    win32::list_box selection;

    std::optional<gl_renderer> renderer;

    std::map<std::optional<std::string>, std::map<std::string, bool>> visible_nodes;
    std::map<std::string, std::map<std::string, bool>> visible_objects;
    win32::gdi::drawing_context gdi_context;
    win32::auto_handle<HGLRC, opengl_deleter> opengl_context;

    glm::vec3 translation = { 0, 0, -20 };
    content::vector3f rotation = { 115, 180, -35 };

    dts_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      render_view = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_OWNERDRAW });

      selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS,
      });

      selection.InsertString(-1, L"Palette 1");
      selection.InsertString(-1, L"Palette 2");
      selection.InsertString(-1, L"Palette 3");

      gdi_context = win32::gdi::drawing_context(render_view.ref());

      assert(gdi_context.get() != nullptr);
      PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .cDepthBits = 32
      };
      int iPixelFormat;

      iPixelFormat = ChoosePixelFormat(gdi_context, &pfd);

      assert(iPixelFormat != 0);
      assert(SetPixelFormat(gdi_context, iPixelFormat, &pfd) == TRUE);
      opengl_context.reset(::wglCreateContext(gdi_context));
      assert(opengl_context.get() != nullptr);

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(selection);
        return 0;
      }

      return std::nullopt;
    }

    auto wm_destroy()
    {
      ::wglMakeCurrent(nullptr, nullptr);

      return 0;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (controller.is_shape(stream))
      {
        auto size = controller.load_shape(stream);

        if (size > 0)
        {
          renderer.emplace(visible_nodes, visible_objects);
          ::InvalidateRect(render_view, nullptr, TRUE);
          return TRUE;
        }
      }

      return FALSE;
    }

    auto get_gl_context()
    {
      if (::wglMakeCurrent(gdi_context, opengl_context) == FALSE)
      {
      }

      return opengl_context.get();
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      auto left_size = SIZE{ .cx = (client_size.cx / 3) * 2, .cy = client_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - left_size.cx, .cy = client_size.cy };

      render_view.SetWindowPos(left_size);
      render_view.SetWindowPos(POINT{});

      win32::gdi::drawing_context gdi_context(render_view.ref());
      auto context = get_gl_context();
      glViewport(0, 0, left_size.cx, left_size.cy);
      glClearDepth(1.f);

      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      perspectiveGL(90.f, double(left_size.cx) / double(left_size.cy), 1.f, 1200.0f);
      selection.SetWindowPos(right_size);
      selection.SetWindowPos(POINT{ .x = left_size.cx });

      return 0;
    }

    std::optional<HBRUSH> wm_control_color(win32::static_control, win32::gdi::drawing_context_ref) override
    {
      auto gl_context = get_gl_context();
      glClearColor(0.3f, 0.3f, 0.3f, 0.f);

      return GetStockBrush(DC_BRUSH);
    }

    std::optional<win32::lresult_t> wm_draw_item(win32::static_control, DRAWITEMSTRUCT& item) override
    {
      if (item.hwndItem == render_view && item.itemAction == ODA_DRAWENTIRE && renderer)
      {
        auto existing_gl_context = get_gl_context();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(translation.x, translation.y, translation.z);

        glRotatef(rotation.x, 1.f, 0.f, 0.f);
        glRotatef(rotation.y, 0.f, 1.f, 0.f);
        glRotatef(rotation.z, 0.f, 0.f, 1.f);

        glBegin(GL_TRIANGLES);
        controller.render_shape(0, *renderer);
        glEnd();
        glFlush();
      }

      return TRUE;
    }
  };
}// namespace siege::views

#endif