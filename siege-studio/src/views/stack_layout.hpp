#include <siege/platform/win/window.hpp>

namespace siege::views
{
    struct default_window
    {
        HWND handle;

        default_window(HWND handle, CREATESTRUCTW& params) : handle(handle)
        {

        }

        virtual LRESULT default_proc(UINT message, WPARAM wparam, LPARAM lparam)
        {
            return DefWindowProcW(handle, message, wparam, lparma);
        }

        virtual std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) 
        { 
            return std::nullopt;
        }

        virtual ~default_window() = default;
    };

    template<TWindow>
    struct basic_window : default_window
    {
        using default_window::default_window;

        static LRESULT __stdcall window_proc(HWND self, UINT message, WPARAM wparam, LPARAM lparam)
        {
            if (message == WM_NCCREATE)
            {
                try
                {
                    default_window* temp = new TWindow(handle, *(CREATESTRUCTW*)lparam);
                    ::SetWindowLongPtrW(self, 0, (LONG_PTR)temp);

                    return temp->default_proc(message, wparam, lparam);
                }
                catch (...)
                {
                    return FALSE;
                }
            }

            auto* window = (basic_window)::GetWindowLongPtrW(self, 0);

            if (!window)
            {
                return ::DefWindowProcW(self, message, wparam, lparam);
            }

            if (message == WM_NCDESTROY)
            {
                auto result = window->default_proc(message, wparam, lparam);
                delete window;
                ::SetWindowLongPtrW(self, 0, 0);
                return result;
            }

            auto result = window->window_proc(message, wparam, lparam);

            if (result)
            {
                return *result;
            }

            return window->default_proc(message, wparam, lparam);
        }
    };

    struct final stack_layout : basic_window<stack_layout>
    {
        std::vector<HWND> children;

        stack_layout(HWND handle, CREATESTRUCTW& params) : basic_window(handle)
        {
            if (params.style & WS_CHILD)
            {
                params.dwExStyle = params.dwExStyle | WS_EX_CONTROLPARENT;
            }

            params.style = params.style | WS_CLIPCHILDREN;
        }
        
        std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override
        {
            switch (message)
            {
                case WM_PARENTNOTIFY:
                {
                    if (LOWORD(wparam) == WM_CREATE)
                    {
                        children.push_back((HWND)lparam);
                        return 0;
                    }

                    if (LOWORD(wparam) == WM_DESTROY)
                    {
                        auto iter = std::find(children.begin(), children.end(), (HWND)lparam);
                        
                        if (iter != children.end())
                        {
                            children.erase(iter);
                        }

                        return 0;
                    }

                    [[fallthrough]]
                }
                case WM_SIZE:
                {
                    auto preference = (ORIENTATION_PREFERENCE)::GetPropW(handle, L"Orientation");

                    if (preference == ORIENTATION_PREFERENCE_PORTRAIT)
                    {
                        auto defer = ::BeginDeferWindowPos(children);

                        RECT client_rect{};

                        UINT width = LOWORD(lParam);

                        auto y = 0;

                        for (auto child : children)
                        {
                            ::GetClientRect(child, &client_rect);
                            auto height = client_rect->bottom - client_rect->top;
                            ::DeferWindowPos(defer, child, nullptr, 0, y, width, height, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
                            y += height;
                        }

                        ::EndDeferWindowPos(defer);
                    }
                    else if (preference == ORIENTATION_PREFERENCE_LANDSCAPE)
                    {
                        auto defer = ::BeginDeferWindowPos(children);

                        RECT client_rect{};

                        UINT height = LOWORD(lParam);

                        auto x = 0;

                        for (auto child : children)
                        {
                            ::GetClientRect(child, &client_rect);
                            auto width = client_rect->right - client_rect->left;
                            ::DeferWindowPos(defer, child, nullptr, x, 0, width, height, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
                            x += width;
                        }

                        ::EndDeferWindowPos(defer);
                    }

                }
                default:
                    return std::nullopt;
            }

        }

    };


}