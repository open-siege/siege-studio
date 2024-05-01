#ifndef WIN32_STACK_PANEL_HPP
#define WIN32_STACK_PANEL_HPP

#include <siege/platform/win/desktop/win32_window.hpp>

namespace win32
{
	struct stack_panel : window
	{
		// for client code
		using window::window;

		stack_panel(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window(self)
	    {
	    }	
	};
}

#endif // !WIN32_STACK_PANEL_HPP
