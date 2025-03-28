#ifndef DTS_VIEW_HPP
#define DTS_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/shell.hpp>
#include <siege/platform/win/theming.hpp>
#include "dts_controller.hpp"
#include "gl_renderer.hpp"
#include <siege/content/obj_renderer.hpp>
#include <glm/ext/vector_float3.hpp>

namespace siege::views
{
  struct opengl_deleter
  {
    void operator()(HGLRC gl_context)
    {
      auto result = ::wglDeleteContext(gl_context);
      assert(result == TRUE);
    }
  };

  struct dts_view final : win32::window_ref
  {
    dts_controller controller;

    win32::static_control render_view;
    win32::list_box detail_level_list;
    win32::popup_menu sequence_menu;

    std::optional<gl_renderer> renderer;

    std::map<std::optional<std::string>, std::map<std::string, bool>> visible_nodes;
    std::map<std::string, std::map<std::string, bool>> visible_objects;
    win32::gdi::drawing_context gdi_context;
    win32::auto_handle<HGLRC, opengl_deleter> opengl_context;
    win32::tool_bar shape_actions;
    win32::image_list image_list;

    bool is_panning = false;
    bool is_rotating = false;
    std::function<bool()> pan_timer;
    std::optional<POINTS> last_mouse_position = std::nullopt;

    glm::vec3 translation = { 0, 0, -20 };
    content::vector3f rotation = { 45, 90, -35 };

    dts_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      render_view = *win32::CreateWindowExW<win32::static_control>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SS_OWNERDRAW });
      render_view.bind_custom_draw({ .wm_control_color = std::bind_front(&dts_view::render_view_wm_control_color, this),
        .wm_draw_item = std::bind_front(&dts_view::render_view_wm_draw_item, this) });

      detail_level_list = *win32::CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | LBS_NOTIFY | LBS_HASSTRINGS | LBS_MULTIPLESEL,
      });
      detail_level_list.bind_lbn_sel_change([this](win32::list_box, const NMHDR&) {
        std::vector<int> indexes;
        indexes.resize(ListBox_GetCount(detail_level_list));
        indexes.resize(ListBox_GetSelItems(detail_level_list, indexes.size(), indexes.data()));

        std::vector<std::size_t> widened;
        widened.reserve(indexes.size());
        std::copy(indexes.begin(), indexes.end(), std::back_inserter(widened));

        controller.set_selected_detail_levels(0, widened);
        ::InvalidateRect(render_view, nullptr, TRUE);
      });

      shape_actions = *win32::CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | BTNS_CHECKGROUP });

      shape_actions.InsertButton(-1, { .iBitmap = 0, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Zoom In" });
      shape_actions.InsertButton(-1, { .iBitmap = 1, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Zoom Out" });
      shape_actions.InsertButton(-1, { .iBitmap = 2, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECK, .iString = (INT_PTR)L"Pan" });
      shape_actions.InsertButton(-1, { .iBitmap = 3, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_CHECK, .iString = (INT_PTR)L"Rotate" });
      shape_actions.InsertButton(-1, { .iBitmap = 4, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN, .iString = (INT_PTR)L"Sequence" });
      shape_actions.InsertButton(-1, { .fsStyle = BTNS_SEP });
      shape_actions.InsertButton(-1, { .iBitmap = 5, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_DROPDOWN, .iString = (INT_PTR)L"Export" });
      shape_actions.bind_nm_click(std::bind_front(&dts_view::shape_actions_nm_click, this));
      shape_actions.bind_tbn_dropdown(std::bind_front(&dts_view::shape_actions_tbn_dropdown, this));

      shape_actions.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS | TBSTYLE_EX_DRAWDDARROWS);

      sequence_menu.AppendMenuW(MF_OWNERDRAW, 1, L"Animate");

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
      SetPixelFormat(gdi_context, iPixelFormat, &pfd);
      opengl_context.reset(::wglCreateContext(gdi_context));
      assert(opengl_context.get() != nullptr);

      return 0;
    }

    void set_is_panning(bool is_panning)
    {
      this->is_panning = is_panning;

      if (is_panning && !pan_timer)
      {
        pan_timer = win32::SetTimer(this->ref(), 50, [this](auto, auto, auto, auto) {
          ::InvalidateRect(render_view, nullptr, TRUE);
        });
      }
      else if (!(is_panning || is_rotating) && pan_timer)
      {
        pan_timer();
        pan_timer = nullptr;
      }
    }

    void set_is_rotating(bool is_rotating)
    {
      this->is_rotating = is_rotating;

      if (is_rotating && !pan_timer)
      {
        pan_timer = win32::SetTimer(this->ref(), 50, [this](auto, auto, auto, auto) {
          ::InvalidateRect(render_view, nullptr, TRUE);
        });
      }
      else if (!(is_panning || is_rotating) && pan_timer)
      {
        pan_timer();
        pan_timer = nullptr;
      }
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
        win32::segoe_fluent_icons::rotate,
        win32::segoe_fluent_icons::picture,
        win32::segoe_fluent_icons::save,
      };

      image_list = win32::create_icon_list(icons, icon_size);
      SendMessageW(shape_actions, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());
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
        auto detail_levels = controller.load_shape(stream);

        if (!detail_levels.empty())
        {
          renderer.emplace(visible_nodes, visible_objects);

          std::wstring temp;
          for (auto& detail_level : detail_levels)
          {
            temp.resize(detail_level.size());
            std::transform(detail_level.begin(), detail_level.end(), temp.begin(), [](auto value) { return (wchar_t)value; });
            detail_level_list.InsertString(-1, temp.c_str());
          }

          ::InvalidateRect(render_view, nullptr, TRUE);
          return TRUE;
        }
      }

      return FALSE;
    }

    auto wm_mouse_button_down(std::size_t wparam, POINTS mouse_position)
    {
      if (wparam & MK_MBUTTON || (is_panning && wparam & MK_LBUTTON))
      {
        set_is_panning(!is_panning);
      }

      if (wparam & MK_RBUTTON || (is_rotating && wparam & MK_LBUTTON))
      {
        set_is_rotating(!is_rotating);
      }

      return 0;
    }

    auto wm_mouse_move(std::size_t wparam, POINTS mouse_position)
    {
      if (is_panning)
      {
        if (wparam & MK_MBUTTON)
        {
          set_is_panning(false);
          return 0;
        }

        if (last_mouse_position && mouse_position.x > last_mouse_position->x)
        {
          translation.x += mouse_position.x - last_mouse_position->x;
        }
        else if (last_mouse_position && mouse_position.x < last_mouse_position->x)
        {
          translation.x -= last_mouse_position->x - mouse_position.x;
        }

        if (last_mouse_position && mouse_position.y > last_mouse_position->y)
        {
          translation.y -= mouse_position.y - last_mouse_position->y;
        }
        else if (last_mouse_position && mouse_position.y < last_mouse_position->y)
        {
          translation.y += last_mouse_position->y - mouse_position.y;
        }
        last_mouse_position = mouse_position;
      }

      if (is_rotating)
      {
        if (wparam & MK_RBUTTON)
        {
          set_is_rotating(false);
          return 0;
        }

        if (last_mouse_position && mouse_position.x > last_mouse_position->x)
        {
          rotation.x += mouse_position.x - last_mouse_position->x;
        }
        else if (last_mouse_position && mouse_position.x < last_mouse_position->x)
        {
          rotation.x -= last_mouse_position->x - mouse_position.x;
        }

        if (last_mouse_position && mouse_position.y > last_mouse_position->y)
        {
          rotation.y -= mouse_position.y - last_mouse_position->y;
        }
        else if (last_mouse_position && mouse_position.y < last_mouse_position->y)
        {
          rotation.y += last_mouse_position->y - mouse_position.y;
        }

        last_mouse_position = mouse_position;
      }
      return 0;
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

      recreate_image_list(shape_actions.GetIdealIconSize(SIZE{ .cx = client_size.cx / (LONG)shape_actions.ButtonCount(), .cy = top_size.cy }));

      SendMessageW(shape_actions, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());
      shape_actions.SetWindowPos(POINT{}, SWP_DEFERERASE | SWP_NOREDRAW);
      shape_actions.SetWindowPos(top_size, SWP_DEFERERASE);
      shape_actions.SetButtonSize(SIZE{ .cx = top_size.cx / (LONG)shape_actions.ButtonCount(), .cy = top_size.cy });


      render_view.SetWindowPos(left_size);
      render_view.SetWindowPos(POINT{ .y = top_size.cy });

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
      detail_level_list.SetWindowPos(right_size);
      detail_level_list.SetWindowPos(POINT{ .x = left_size.cx, .y = top_size.cy });

      return 0;
    }

    LRESULT shape_actions_tbn_dropdown(win32::tool_bar, const NMTOOLBAR& message)
    {
      return TBDDRET_NODEFAULT;
    }

    BOOL shape_actions_nm_click(win32::tool_bar, const NMMOUSE& message)
    {
      if (message.dwItemSpec == 0)
      {
        translation.z += 5;
        InvalidateRect(render_view, nullptr, TRUE);
        return TRUE;
      }
      else if (message.dwItemSpec == 1)
      {
        translation.z -= 5;
        InvalidateRect(render_view, nullptr, TRUE);
        return TRUE;
      }
      else if (message.dwItemSpec == 2)
      {
        set_is_panning(!is_panning);
        return TRUE;
      }
      else if (message.dwItemSpec == 3)
      {
        set_is_rotating(!is_rotating);
        return TRUE;
      }
      else if (message.dwItemSpec == 4)
      {
        POINT mouse_pos;

        if (::GetCursorPos(&mouse_pos))
        {
          auto selection = ::TrackPopupMenu(sequence_menu, TPM_CENTERALIGN | TPM_RETURNCMD, mouse_pos.x, mouse_pos.y, 0, *this, nullptr);
        }


        return TRUE;
      }
      else if (message.dwItemSpec == 6)
      {
        auto dialog = win32::com::CreateFileSaveDialog();

        if (dialog)
        {
        }
        return TRUE;
      }

      return FALSE;
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

        controller.render_shape(0, *renderer);
        glFlush();
      }

      return TRUE;
    }
  };
}// namespace siege::views

#endif