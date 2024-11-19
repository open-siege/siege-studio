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
  {
    dts_controller controller;

    win32::static_control render_view;
    win32::list_box selection;

    std::optional<gl_renderer> renderer;

    std::map<std::optional<std::string>, std::map<std::string, bool>> visible_nodes;
    std::map<std::string, std::map<std::string, bool>> visible_objects;
    win32::gdi::drawing_context gdi_context;
    win32::auto_handle<HGLRC, opengl_deleter> opengl_context;
    win32::tool_bar shape_actions;
    win32::image_list image_list;

    glm::vec3 translation = { 0, 0, -20 };
    content::vector3f rotation = { 115, 180, -35 };

    dts_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      render_view = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_OWNERDRAW });
      render_view.bind_custom_draw({ 
          .wm_control_color = std::bind_front(&dts_view::render_view_wm_control_color, this),
        .wm_draw_item = std::bind_front(&dts_view::render_view_wm_draw_item, this)
      });

      selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS,
      });

      selection.InsertString(-1, L"Detail Level 1");
      selection.InsertString(-1, L"Detail Level 2");
      selection.InsertString(-1, L"Detail Level 3");

      shape_actions = *control_factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });

      shape_actions.InsertButton(-1, { .iBitmap = 0, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Zoom In" });
      shape_actions.InsertButton(-1, { .iBitmap = 1, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Zoom Out" });
      shape_actions.InsertButton(-1, { .iBitmap = 2, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECK, .iString = (INT_PTR)L"Pan" });
      shape_actions.InsertButton(-1, { .fsStyle = BTNS_SEP });
      shape_actions.InsertButton(-1, { .iBitmap = 3, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Export" });

      shape_actions.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);


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

    void recreate_image_list(std::optional<SIZE> possible_size)
    {

      SIZE icon_size = possible_size.or_else([this] {
                                      return image_list.GetIconSize();
                                    })
                         .or_else([] {
                           return std::make_optional(SIZE{
                             .cx = ::GetSystemMetrics(SM_CXSIZE),
                             .cy = ::GetSystemMetrics(SM_CYSIZE) });
                         })
                         .value();

      if (image_list)
      {
        image_list.reset();
      }

      std::vector icons{
        win32::segoe_fluent_icons::zoom_in,
        win32::segoe_fluent_icons::zoom_out,
        win32::segoe_fluent_icons::pan_mode,
        win32::segoe_fluent_icons::save,
      };

      image_list = win32::create_icon_list(icons, icon_size);
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(selection);
        win32::apply_theme(shape_actions);
        win32::apply_theme(*this);
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
      auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 12 };


      auto left_size = SIZE{ .cx = (client_size.cx / 3) * 2, .cy = client_size.cy - top_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - left_size.cx, .cy = client_size.cy - top_size.cy };

      recreate_image_list(shape_actions.GetIdealIconSize(SIZE{ .cx = client_size.cx / shape_actions.ButtonCount(), .cy = top_size.cy }));

      SendMessageW(shape_actions, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());
      shape_actions.SetWindowPos(POINT{}, SWP_DEFERERASE | SWP_NOREDRAW);
      shape_actions.SetWindowPos(top_size, SWP_DEFERERASE);
      shape_actions.SetButtonSize(SIZE{ .cx = top_size.cx / shape_actions.ButtonCount(), .cy = top_size.cy });


      render_view.SetWindowPos(left_size);
      render_view.SetWindowPos(POINT{.y = top_size.cy });

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
      selection.SetWindowPos(POINT{ .x = left_size.cx, .y = top_size.cy });

      return 0;
    }

    HBRUSH render_view_wm_control_color(win32::static_control, win32::gdi::drawing_context_ref)
    {
      auto gl_context = get_gl_context();
      glClearColor(0.3f, 0.3f, 0.3f, 0.f);

      return GetStockBrush(DC_BRUSH);
    }

    win32::lresult_t render_view_wm_draw_item(win32::static_control, DRAWITEMSTRUCT& item)
    {
      if (item.itemAction == ODA_DRAWENTIRE && renderer)
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