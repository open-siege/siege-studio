module;
#include <siege/platform/windows.hpp>

export module dml.views;

import std;
import win32.window;

import win32.window;
import win32.common_controls;

import siege.platform.stream;

import dml.shared;

namespace siege::views
{
  struct dml_view : win32::basic_window<dml_view>
  {
    material_context state;
    win32::list_box ref_names;

    dml_view(HWND self, CREATESTRUCTW& params) : basic_window(self, params)
    {
    }

    auto wm_create()
    {
      ref_names = *win32::CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
        .hwndParent = *this,
        .style = WS_VISIBLE | WS_CHILD | LBS_HASSTRINGS,
      });

      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      ref_names.SetWindowPos(client_size);
      ref_names.SetWindowPos(POINT{});

      return 0;
    }

    auto wm_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (is_material(stream))
      {
        auto size = load_material(state, stream);

        if (size > 0)
        {
          for (auto i = 0u; i < size; ++i)
          {
            auto filename = get_filename(state, i);
            if (filename)
            {
              ref_names.AddString(filename->wstring());
            }
            else
            {
              ref_names.AddString(L"Material " + std::to_wstring(i + 1));
            }
          }

          return TRUE;
        }
      }

      return FALSE;
    }

    std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override
    {
      switch (message)
      {
      case WM_CREATE:
        return wm_create();
      case WM_SIZE:
        return (LRESULT)wm_size((std::size_t)wparam, SIZE(win32::loword(lparam), win32::hiword(lparam)));
      case WM_COPYDATA:
        return (LRESULT)wm_copy_data(win32::copy_data_message<char>(wparam, lparam));
      default:
        return std::nullopt;
      }
    }
  };

  export ATOM register_dml_view(win32::window_module_ref module)
  {
    WNDCLASSEXW info{
      .cbSize = sizeof(info),
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = win32::basic_window<dml_view>::window_proc,
      .cbWndExtra = sizeof(void*),
      .hInstance = module,
      .lpszClassName = win32::type_name<dml_view>().c_str(),
    };
    return ::RegisterClassExW(&info);
  }
}// namespace siege::views
