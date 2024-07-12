#ifndef THEME_VIEW_HPP
#define THEME_VIEW_HPP

#include <siege/platform/win/desktop/window_factory.hpp>

namespace siege::views
{
    struct theme_view : win32::window_ref
	{
        theme_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
		{

		}
    };
}

#endif