#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include "sfx_controller.hpp"
#include "media_module.hpp"

namespace siege::views
{
  struct sfx_view : win32::window_ref
  {
    sfx_controller controller;

    win32::tool_bar player_buttons;
    win32::static_control render_view;
    win32::list_box selection;

    media_module media;

    sfx_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self), media{}
    {
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      player_buttons = *control_factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | TBSTYLE_LIST | TBSTYLE_WRAPABLE });

      player_buttons.InsertButton(-1, { .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK | BTNS_SHOWTEXT, .iString = (INT_PTR)L"Play" });

      player_buttons.InsertButton(-1, { .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK | BTNS_SHOWTEXT, .iString = (INT_PTR)L"Pause" });

      player_buttons.InsertButton(-1, { .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK | BTNS_SHOWTEXT, .iString = (INT_PTR)L"Stop" });

      player_buttons.InsertButton(-1, { .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK | BTNS_SHOWTEXT, .iString = (INT_PTR)L"Loop" });

      render_view = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD, .lpszName = L"Test" });

      selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS });

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      auto left_size = SIZE{ .cx = (client_size.cx / 3) * 2, .cy = client_size.cy };
      auto right_size = SIZE{ .cx = client_size.cx - left_size.cx, .cy = client_size.cy };

      auto height = left_size.cy / 12;
      player_buttons.SetWindowPos(SIZE{ .cx = left_size.cx, .cy = height });
      player_buttons.SetWindowPos(POINT{});
      player_buttons.SetButtonSize(SIZE{ .cx = left_size.cx / (LONG)player_buttons.ButtonCount(), .cy = height });
      player_buttons.AutoSize();

      render_view.SetWindowPos(SIZE{ .cx = left_size.cx, .cy = left_size.cy - height });
      render_view.SetWindowPos(POINT{ .y = height });

      selection.SetWindowPos(right_size);
      selection.SetWindowPos(POINT{ .x = left_size.cx });

      return 0;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (controller.is_sfx(stream))
      {
        auto size = controller.load_sound(stream);

        if (size > 0)
        {
          for (auto i = 0u; i < size; ++i)
          {
            selection.InsertString(-1, L"Stream " + std::to_wstring(i + 1));
          }

          return TRUE;
        }

        return FALSE;
      }

      return FALSE;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        auto parent = this->GetParent();

        win32::apply_theme(*parent, selection);
        win32::apply_theme(*parent, player_buttons);
        win32::apply_theme(*parent, render_view);

        return 0;
      }

      return std::nullopt;
    }

    std::optional<win32::lresult_t> wm_notify(win32::tool_bar_custom_draw_notification message)
    {
      if (message.ref.nmcd.dwDrawStage == CDDS_PREPAINT)
      {
        return CDRF_NOTIFYITEMDRAW;
      }

      if (message.ref.nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
      {
        if (message.ref.nmcd.dwItemSpec == 0)
        {
          RECT rect = message.ref.nmcd.rc;
          SIZE one_third = SIZE{ .cx = (rect.right - rect.left) / 12, .cy = (rect.bottom - rect.top) };
          POINT vertices[] = { { rect.left, rect.top }, { rect.left + ((rect.right - rect.left) / 4), rect.top + ((rect.bottom - rect.top) / 2) }, { rect.left, rect.bottom } };

          ::SelectObject(message.ref.nmcd.hdc, ::GetSysColorBrush(COLOR_BTNTEXT));
          ::Polygon(message.ref.nmcd.hdc, vertices, sizeof(vertices) / sizeof(POINT));
        }

        if (message.ref.nmcd.dwItemSpec == 1)
        {
          RECT rect = message.ref.nmcd.rc;

          SIZE one_third = SIZE{ .cx = (rect.right - rect.left) / 12, .cy = (rect.bottom - rect.top) };
          RECT left = RECT{ .left = rect.left, .top = rect.top, .right = rect.left + one_third.cx, .bottom = rect.bottom };
          RECT middle = RECT{ .left = left.right, .top = rect.top, .right = left.right + one_third.cx, .bottom = rect.bottom };
          RECT right = RECT{ .left = middle.right, .top = rect.top, .right = middle.right + one_third.cx, .bottom = rect.bottom };

          ::FillRect(message.ref.nmcd.hdc, &left, ::GetSysColorBrush(COLOR_BTNTEXT));

          ::FillRect(message.ref.nmcd.hdc, &right, ::GetSysColorBrush(COLOR_BTNTEXT));
        }

        if (message.ref.nmcd.dwItemSpec == 2)
        {
          auto min = std::min<int>(message.ref.nmcd.rc.right - message.ref.nmcd.rc.left, message.ref.nmcd.rc.bottom - message.ref.nmcd.rc.top);
          RECT rect = {
            .left = message.ref.nmcd.rc.left,
            .top = message.ref.nmcd.rc.top,
            .right = message.ref.nmcd.rc.left + min,
            .bottom = message.ref.nmcd.rc.top + min
          };

          FillRect(message.ref.nmcd.hdc, &rect, ::GetSysColorBrush(COLOR_BTNTEXT));
        }

        if (message.ref.nmcd.dwItemSpec == 3)
        {
          return 0;
        }
        return CDRF_SKIPDEFAULT;
      }

      return std::nullopt;
    }

    std::optional<win32::lresult_t> wm_notify(win32::mouse_notification message)
    {
      switch (message.hdr.code)
      {
      case NM_CLICK: {
        if (message.hdr.hwndFrom == player_buttons)
        {
          auto index = message.dwItemSpec;

          if (index == 3)
          {
            auto state = ::SendMessageW(player_buttons, TB_GETSTATE, 3, 0);

            if (!(state & TBSTATE_CHECKED))
            {
              media.StopSound();
            }
          }

          if (index == 0)
          {
            auto state = ::SendMessageW(player_buttons, TB_GETSTATE, 3, 0);

            bool loop = false;
            if (state & TBSTATE_CHECKED)
            {
              loop = true;
            }
            auto path = controller.get_sound_path(0);

            if (path)
            {
              media.PlaySound(*path, true, loop);
            }

            auto data = controller.get_sound_data(0);

            if (data)
            {
              media.PlaySound(*data, true, loop);
            }
          }

          if (index == 1)
          {
            media.StopSound();
          }

          if (index == 2)
          {
            media.StopSound();
          }

          for (auto i = 0; i < 3; ++i)
          {
            if (i == index && i != 2)
            {
              continue;
            }
            auto state = ::SendMessageW(player_buttons, TB_GETSTATE, i, 0);
            ::SendMessageW(player_buttons, TB_SETSTATE, i, MAKELPARAM(state & ~TBSTATE_CHECKED, 0));
          }
        }
        return 0;
      }
      default: {
        return std::nullopt;
      }
      }
    }
  };
}// namespace siege::views

#endif