#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theme_module.hpp>
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

    auto on_create(const win32::create_message&)
    {
      auto control_factory = win32::window_factory(ref());

      player_buttons = *control_factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | TBSTYLE_LIST | TBSTYLE_WRAPABLE });
      win32::theme_module().SetWindowTheme(player_buttons, L"", L"");

      ::COLORSCHEME colors{ .dwSize = sizeof(::COLORSCHEME), .clrBtnHighlight = 0x00383838 };

      SendMessageW(player_buttons, TB_SETCOLORSCHEME, 0, (LPARAM)&colors);

      player_buttons.InsertButton(-1, { .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK | BTNS_SHOWTEXT, .iString = (INT_PTR)L"Play" });

      player_buttons.InsertButton(-1, { .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK | BTNS_SHOWTEXT, .iString = (INT_PTR)L"Pause" });

      player_buttons.InsertButton(-1, { .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK | BTNS_SHOWTEXT, .iString = (INT_PTR)L"Stop" });

      player_buttons.InsertButton(-1, { .iBitmap = I_IMAGENONE, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK | BTNS_SHOWTEXT, .iString = (INT_PTR)L"Loop" });

      render_view = *control_factory.CreateWindowExW<win32::static_control>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD, .lpszName = L"Test" });

      selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS });


      selection.InsertString(-1, L"Palette 1");
      selection.InsertString(-1, L"Palette 2");
      selection.InsertString(-1, L"Palette 3");


      return 0;
    }

    auto on_size(win32::size_message sized)
    {
      auto left_size = SIZE{ .cx = (sized.client_size.cx / 3) * 2, .cy = sized.client_size.cy };
      auto right_size = SIZE{ .cx = sized.client_size.cx - left_size.cx, .cy = sized.client_size.cy };

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

    auto on_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (controller.is_sfx(stream))
      {
        auto size = controller.load_sound(stream);

        if (size > 0)
        {
          return TRUE;
        }

        return FALSE;
      }

      return FALSE;
    }

    std::optional<win32::lresult_t> on_notify(win32::tool_bar_custom_draw_notification message)
    {
      if (message.ref.nmcd.dwDrawStage == CDDS_PREPAINT)
      {
        return CDRF_NOTIFYITEMDRAW;
      }

      if (message.ref.nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
      {
        message.ref.clrBtnFace = 0x00000000;
        message.ref.clrBtnHighlight = 0x00383838;
        message.ref.clrText = 0x00FFFFFF;
        //      message.ref.nStringBkMode = OPAQUE;
        return CDRF_NEWFONT | TBCDRF_USECDCOLORS;
      }

      if (message.ref.nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
      {
        message.ref.clrBtnFace = 0x00000000;
        message.ref.clrBtnHighlight = 0x00383838;
        message.ref.clrText = 0x00FFFFFF;
        //        message.ref.nStringBkMode = OPAQUE;
        return CDRF_NEWFONT | TBCDRF_USECDCOLORS;
      }

      return CDRF_DODEFAULT;
    }


    std::optional<win32::lresult_t> on_notify(win32::mouse_notification message)
    {
      switch (message.hdr.code)
      {
      case NM_CLICK: {
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