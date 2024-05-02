#ifndef WIN32_STACK_PANEL_HPP
#define WIN32_STACK_PANEL_HPP

#include <siege/platform/win/desktop/win32_window.hpp>

namespace win32
{
	struct stack_panel : window
	{
		stack_panel(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window(self)
	    {
	    }

		auto on_pos_changed(win32::pos_changed_message sized)
	    {
			std::vector<window_ref> children;
			children.reserve(4);
		    win32::ForEachDirectChildWindow(*this, [&](auto child) {
				children.emplace_back(child);
		    });

			win32::StackChildren(SIZE{.cx = sized.data.cx, .cy = sized.data.cy}, children, win32::StackDirection::Vertical, POINT{.x = sized.data.x, .y = sized.data.y});

		    return std::nullopt;
	    }
	};
}

#endif // !WIN32_STACK_PANEL_HPP
