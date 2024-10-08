#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <utility>

namespace siege::views
{
  struct default_view final : win32::window_ref
  {
    inline static auto first_time = true;

    win32::static_control heading;//"Welcome to Siege Studio."
    win32::static_control logo;

    //      std::string url = "https://github.com/open-siege/open-siege/wiki/" + extension;
    //"This particular file is not yet supported by Siege Studio.\nThough, you can still read about it on our wiki.\nClick the link below to find out more."
    default_view(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      win32::window_factory factory(ref());

      heading = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE });
      logo = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE });

      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MAXIMIZED || type == SIZE_RESTORED)
      {
        auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 2 };
        auto bottom_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 2 };

        heading.SetWindowPos(POINT{});
        heading.SetWindowPos(top_size);

        logo.SetWindowPos(POINT{ .y = top_size.cy });
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

        return 0;
      }

      return std::nullopt;
    }

    auto wm_command()
    {
      // wxLaunchDefaultBrowser(url);
    }
  };
}// namespace siege::views