#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/basic_window.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include "sfx_shared.hpp"
#include "media_module.hpp"

namespace siege::views
{
  struct sfx_view final : win32::basic_window<sfx_view>
  {
    std::any state;

    win32::tool_bar player_buttons;
    win32::list_box selection;

    win32::image_list image_list;
    media_module media;

    sfx_view(win32::hwnd_t self, CREATESTRUCTW& params) : basic_window(self, params), media{}
    {
    }

    ~sfx_view()
    {
    }

    constexpr static auto play_id = 0;
    constexpr static auto stop_id = 1;
    constexpr static auto loop_id = 2;

    auto wm_create()
    {
      player_buttons = *win32::CreateWindowExW<win32::tool_bar>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS });

      player_buttons.bind_nm_click(std::bind_front(&sfx_view::player_buttons_nm_click, this));
      player_buttons.InsertButton(-1, { .iBitmap = 0, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK, .iString = (INT_PTR)L"Play" });

      player_buttons.InsertButton(-1, { .iBitmap = 2, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK, .iString = (INT_PTR)L"Stop" });

      player_buttons.InsertButton(-1, { .iBitmap = 3, .fsState = TBSTATE_ENABLED, .fsStyle = BTNS_BUTTON | BTNS_CHECK, .iString = (INT_PTR)L"Loop" });
      player_buttons.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

      selection = *win32::CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS });

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

      if (is_sfx(stream))
      {
        auto size = load_sound(state, stream);

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
        recreate_image_list(std::nullopt);
        SendMessageW(player_buttons, TB_SETIMAGELIST, 0, (LPARAM)image_list.get());

        return 0;
      }

      return std::nullopt;
    }

    // LRESULT wm_notify(win32::tool_bar, NMTBCUSTOMDRAW& message) override
    //{
    //   if (message.nmcd.dwDrawStage == CDDS_PREPAINT)
    //   {
    //     return CDRF_NOTIFYITEMDRAW;
    //   }

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

        if (index == loop_id)
        {
          auto state = ::SendMessageW(player_buttons, TB_GETSTATE, loop_id, 0);

          if (!(state & TBSTATE_CHECKED))
          {
            media.StopSound();
          }
        }

        if (index == play_id)
        {
          auto state = ::SendMessageW(player_buttons, TB_GETSTATE, loop_id, 0);

          bool loop = false;
          if (state & TBSTATE_CHECKED)
          {
            loop = true;
          }
          auto path = get_sound_path(this->state, 0);

          if (path)
          {
            media.PlaySound(*path, true, loop);
          }

          auto data = get_sound_data(this->state, 0);

          if (data)
          {
            media.PlaySound(*data, true, loop);
          }
        }

        if (index == stop_id)
        {
          media.StopSound();
        }

        for (auto i = 0; i < 2; ++i)
        {
          if (i == index && i != stop_id)
          {
            continue;
          }
          auto state = ::SendMessageW(player_buttons, TB_GETSTATE, i, 0);
          ::SendMessageW(player_buttons, TB_SETSTATE, i, MAKELPARAM(state & ~TBSTATE_CHECKED, 0));
        }
      }
      return TRUE;
    }

    std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override
    {
      switch (message)
      {
      case WM_CREATE:
        return wm_create();
      case WM_SETTINGCHANGE:
        return wm_setting_change(win32::setting_change_message(wparam, lparam));
      case WM_SIZE:
        return (LRESULT)wm_size((std::size_t)wparam, SIZE(LOWORD(lparam), HIWORD(lparam)));
      case WM_COPYDATA:
        return (LRESULT)wm_copy_data(win32::copy_data_message<char>(wparam, lparam));
      default:
        return std::nullopt;
      }
    }
  };

  ATOM register_sfx_view(win32::window_module_ref module)
  {
    WNDCLASSEXW info{
      .cbSize = sizeof(info),
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = win32::basic_window<sfx_view>::window_proc,
      .cbWndExtra = sizeof(void*),
      .hInstance = module,
      .lpszClassName = win32::type_name<sfx_view>().c_str(),
    };
    return ::RegisterClassExW(&info);
  }
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