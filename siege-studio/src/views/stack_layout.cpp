#include <siege/platform/win/basic_window.hpp>

namespace siege::views
{
  struct stack_layout final : win32::basic_window<stack_layout>
  {
    std::vector<HWND> children;

    stack_layout(HWND handle, CREATESTRUCTW& params) : basic_window(handle, params)
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
      case WM_PARENTNOTIFY: {
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
        [[fallthrough]];
      }
      case WM_SIZE: {
        auto preference = (ORIENTATION_PREFERENCE)(int)::GetPropW(*this, L"Orientation");

        RECT rect{};

        ::GetClientRect(*this, &rect);

        auto default_height = (int)::GetPropW(*this, L"DefaultHeight");
        auto default_width = (int)::GetPropW(*this, L"DefaultWidth");

        if (!default_height && !children.empty())
        {
          default_height = rect.bottom / children.size();
        }

        if (!default_width && !children.empty())
        {
          default_width = rect.right / children.size();
        }

        if (preference & ORIENTATION_PREFERENCE_PORTRAIT)
        {
          auto defer = ::BeginDeferWindowPos((int)children.size());

          RECT client_rect{};

          UINT width = rect.right;

          auto y = 0;

          for (auto child : children)
          {
            ::GetClientRect(child, &client_rect);
            auto height = client_rect.bottom - client_rect.top;
            height = height ? height : default_height;
            ::DeferWindowPos(defer, child, nullptr, 0, y, width, height, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            y += height;
          }

          ::EndDeferWindowPos(defer);
        }
        else if (preference & ORIENTATION_PREFERENCE_LANDSCAPE)
        {
          auto defer = ::BeginDeferWindowPos((int)children.size());

          RECT client_rect{};

          UINT height = rect.bottom;

          auto x = 0;

          for (auto child : children)
          {
            ::GetClientRect(child, &client_rect);
            auto width = client_rect.right - client_rect.left;
            width = width ? width : default_width;
            ::DeferWindowPos(defer, child, nullptr, x, 0, width, height, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            x += width;
          }

          ::EndDeferWindowPos(defer);
        }
        return 0;
      }
      default:
        return std::nullopt;
      }
    }
  };

  ATOM register_stack_layout(HINSTANCE module)
  {
    WNDCLASSEXW info{
      .cbSize = sizeof(info),
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = win32::basic_window<stack_layout>::window_proc,
      .cbWndExtra = sizeof(void*),
      .hInstance = module,
      .lpszClassName = L"StackLayout",

    };
    return ::RegisterClassExW(&info);
  }
}// namespace siege::views