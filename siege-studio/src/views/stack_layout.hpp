#ifndef STACK_LAYOUT_HPP
#define STACK_LAYOUT_HPP

#include "basic_window.hpp"

namespace siege::views
{

  struct stack_layout final : basic_window<stack_layout>
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
        auto preference = (ORIENTATION_PREFERENCE)(int)::GetPropW(handle, L"Orientation");

        if (preference == ORIENTATION_PREFERENCE_PORTRAIT)
        {
          auto defer = ::BeginDeferWindowPos((int)children.size());

          RECT client_rect{};

          UINT width = LOWORD(lparam);

          auto y = 0;

          for (auto child : children)
          {
            ::GetClientRect(child, &client_rect);
            auto height = client_rect.bottom - client_rect.top;
            ::DeferWindowPos(defer, child, nullptr, 0, y, width, height, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            y += height;
          }

          ::EndDeferWindowPos(defer);
        }
        else if (preference == ORIENTATION_PREFERENCE_LANDSCAPE)
        {
          auto defer = ::BeginDeferWindowPos((int)children.size());

          RECT client_rect{};

          UINT height = LOWORD(lparam);

          auto x = 0;

          for (auto child : children)
          {
            ::GetClientRect(child, &client_rect);
            auto width = client_rect.right - client_rect.left;
            ::DeferWindowPos(defer, child, nullptr, x, 0, width, height, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            x += width;
          }

          ::EndDeferWindowPos(defer);
          return 0;
        }
      }
      default:
        return std::nullopt;
      }
    }
  };
}// namespace siege::views

#endif