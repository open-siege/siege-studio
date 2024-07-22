#ifndef THEME_VIEW_HPP
#define THEME_VIEW_HPP

#include <siege/platform/win/desktop/window_factory.hpp>

namespace siege::views
{
    struct theme_view : win32::window_ref
	{
      win32::window_ref theme_properties;

      // list box for simple and advanced settings
      // simple settings has preferred theme option (from system or user-defined)
      // simple settings has preferred accent color (from system or user-defined)
      // simple settings has theme selection (light, dark)
      // advanced allows theme settings to be changed
      // allows theme settings to be saved
      // theme settings changed per control type

      theme_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
      {

	  }

      auto wm_create()
      {
        return 0;
      }

      auto wm_size(int, SIZE new_size)
      {

      }

      std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message)
      {
        return std::nullopt;
      }
    };
}

#endif