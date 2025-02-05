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
  struct sfx_view final : win32::window_ref
  {
    sfx_controller controller;

    win32::tool_bar player_buttons;
    win32::list_box selection;

    win32::image_list image_list;
    media_module media;

    sfx_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self), media{}
    {
    }

    ~sfx_view()
    {
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      player_buttons = *control_factory.CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{ .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS });

      player_buttons.bind_nm_click(std::bind_front(&sfx_view::player_buttons_nm_click, this));
      player_buttons.InsertButton(-1, { .iBitmap = 0, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK, .iString = (INT_PTR)L"Play" });

      player_buttons.InsertButton(-1, { .iBitmap = 1, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK, .iString = (INT_PTR)L"Pause" });

      player_buttons.InsertButton(-1, { .iBitmap = 2, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK, .iString = (INT_PTR)L"Stop" });

      player_buttons.InsertButton(-1, { .iBitmap = 3, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK, .iString = (INT_PTR)L"Loop" });
      player_buttons.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

      selection = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS | LBS_OWNERDRAWFIXED });

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

      std::vector icons{ win32::segoe_fluent_icons::play, win32::segoe_fluent_icons::pause, win32::segoe_fluent_icons::stop, win32::segoe_fluent_icons::repeat_all };
      image_list = win32::create_icon_list(icons, icon_size);
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MINIMIZED)
      {
        return 0;
      }

      auto right_size = SIZE{ .cx = client_size.cx / 3, .cy = client_size.cy };
      auto left_size = SIZE{ .cx = client_size.cx - right_size.cx, .cy = client_size.cy };

      auto height = left_size.cy / 12;

      auto button_size = SIZE{ .cx = left_size.cx / player_buttons.ButtonCount(), .cy = height };
      recreate_image_list(player_buttons.GetIdealIconSize(button_size));

      SendMessageW(player_buttons, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());

      player_buttons.SetWindowPos(SIZE{ .cx = left_size.cx, .cy = height });
      player_buttons.SetWindowPos(POINT{});
      player_buttons.SetButtonSize(button_size);

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
        win32::apply_theme(selection);
        recreate_image_list(std::nullopt);
        SendMessageW(player_buttons, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());
        win32::apply_theme(player_buttons);
        win32::apply_theme(*this);

        return 0;
      }

      return std::nullopt;
    }

    //LRESULT wm_notify(win32::tool_bar, NMTBCUSTOMDRAW& message) override
    //{
    //  if (message.nmcd.dwDrawStage == CDDS_PREPAINT)
    //  {
    //    return CDRF_NOTIFYITEMDRAW;
    //  }

    //  if (message.nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
    //  {
    //    if (message.nmcd.dwItemSpec == 0)
    //    {
    //      RECT rect = message.nmcd.rc;
    //      SIZE one_third = SIZE{ .cx = (rect.right - rect.left) / 12, .cy = (rect.bottom - rect.top) };
    //      POINT vertices[] = { { rect.left, rect.top }, { rect.left + ((rect.right - rect.left) / 4), rect.top + ((rect.bottom - rect.top) / 2) }, { rect.left, rect.bottom } };

    //      ::SelectObject(message.nmcd.hdc, ::GetSysColorBrush(COLOR_BTNTEXT));
    //      ::Polygon(message.nmcd.hdc, vertices, sizeof(vertices) / sizeof(POINT));
    //    }

    //    if (message.nmcd.dwItemSpec == 1)
    //    {
    //      RECT rect = message.nmcd.rc;

    //      SIZE one_third = SIZE{ .cx = (rect.right - rect.left) / 12, .cy = (rect.bottom - rect.top) };
    //      RECT left = RECT{ .left = rect.left, .top = rect.top, .right = rect.left + one_third.cx, .bottom = rect.bottom };
    //      RECT middle = RECT{ .left = left.right, .top = rect.top, .right = left.right + one_third.cx, .bottom = rect.bottom };
    //      RECT right = RECT{ .left = middle.right, .top = rect.top, .right = middle.right + one_third.cx, .bottom = rect.bottom };

    //      ::FillRect(message.nmcd.hdc, &left, ::GetSysColorBrush(COLOR_BTNTEXT));

    //      ::FillRect(message.nmcd.hdc, &right, ::GetSysColorBrush(COLOR_BTNTEXT));
    //    }

    //    if (message.nmcd.dwItemSpec == 2)
    //    {
    //      auto min = std::min<int>(message.nmcd.rc.right - message.nmcd.rc.left, message.nmcd.rc.bottom - message.nmcd.rc.top);
    //      RECT rect = {
    //        .left = message.nmcd.rc.left,
    //        .top = message.nmcd.rc.top,
    //        .right = message.nmcd.rc.left + min,
    //        .bottom = message.nmcd.rc.top + min
    //      };

    //      FillRect(message.nmcd.hdc, &rect, ::GetSysColorBrush(COLOR_BTNTEXT));
    //    }

    //    if (message.nmcd.dwItemSpec == 3)
    //    {
    //      return 0;
    //    }
    //    return CDRF_SKIPDEFAULT;
    //  }

    //  return CDRF_DODEFAULT;
    //}

    BOOL player_buttons_nm_click(win32::tool_bar, const NMMOUSE& message)
    {
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
      return TRUE;
    }
  };
}// namespace siege::views

/* TODO reimplement old logic
*
* ImGui::Begin("File Info");

    ImGui::LabelText("", "Duration (in seconds): %f", buffer.getDuration().asSeconds());
    ImGui::LabelText("", "Sample Rate: %u", buffer.getSampleRate());


    ImGui::End();

    if (ImGui::Button("Export to WAV"))
    {
      auto new_file_name = info.filename.replace_extension(".wav");
      std::filesystem::create_directories(export_path);

      std::ofstream output(export_path / new_file_name, std::ios::binary);
      output.write(original_data.data(), original_data.size());
      if (!opened_folder)
      {
        wxLaunchDefaultApplication(export_path.string());
        opened_folder = true;
      }
    }

*/

#endif