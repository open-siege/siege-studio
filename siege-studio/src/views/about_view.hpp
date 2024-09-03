#ifndef SIEGE_ABOUT_VIEW_HPP
#define SIEGE_ABOUT_VIEW_HPP


#include <utility>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>

namespace siege::views
{
  struct about_view final : win32::window_ref
  {
    win32::static_control heading;
    win32::static_control logo;

    win32::gdi::icon logo_icon;

    about_view(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
    {
      logo_icon.reset((HICON)::LoadImageW(params.hInstance, L"AppIcon", IMAGE_ICON, 0, 0, 0));
    }

    auto wm_create()
    {
      win32::window_factory factory(ref());

      ::SetWindowTextW(*this, L"About Siege Studio");

      heading = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = L"Siege Studio is an open-source reverse engeering tool to preview, convert or extract files from various games made by Dynamix and using their tecnologies." });
      logo = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE | SS_ICON | SS_REALSIZECONTROL });

      ::SendMessageW(logo, STM_SETIMAGE, IMAGE_ICON, (LPARAM)logo_icon.get());

      wm_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });

      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MAXIMIZED || type == SIZE_RESTORED)
      {
        auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 4 };
        auto bottom_size = SIZE{ .cx = client_size.cy - top_size.cy, .cy = client_size.cy - top_size.cy };

        heading.SetWindowPos(POINT{});
        heading.SetWindowPos(top_size);

        logo.SetWindowPos(POINT{ .x = (client_size.cx - bottom_size.cx) / 2, .y = top_size.cy });
        logo.SetWindowPos(bottom_size);
      }

      return 0;
    }

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        win32::apply_theme(heading);
        win32::apply_theme(logo);
        win32::apply_theme(*this);

        return 0;
      }

      return std::nullopt;
    }

    std::optional<LRESULT> wm_close()
    {
      EndDialog(*this, 0);
      return 0;
    }
  };
}// namespace siege::views

#endif