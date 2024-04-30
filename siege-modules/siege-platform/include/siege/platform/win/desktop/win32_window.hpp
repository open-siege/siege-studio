#ifndef WIN32_WINDOW_HPP
#define WIN32_WINDOW_HPP

#include <siege/platform/win/gaming/win32_user32.hpp>
#include <siege/platform/win/auto_handle.hpp>

namespace win32
{
    struct window_deleter
    {
        void operator()(hwnd_t window) 
        {
            if (::GetParent(window) == nullptr)
            {
                ::DestroyWindow(window);            
            }
        }
    };

	struct window : win32::auto_handle<hwnd_t, window_deleter>
	{
        using base = win32::auto_handle<hwnd_t, window_deleter>;
        using base::base;

        operator hwnd_t()
        {
            return get();
        }
	};
}


#endif // !WIN32_WINDOW_HPP
