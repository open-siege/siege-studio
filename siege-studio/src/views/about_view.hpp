#include <utility>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>

namespace siege::views
{
  struct about_view final : win32::window_ref
  {
    win32::static_control heading;//"About Siege Studio."
    win32::static_control logo;

    about_view(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
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

        heading.SetWindowPos(POINT{ });
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
  };
}// namespace siege::views