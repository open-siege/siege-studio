#ifndef DML_VIEW_HPP
#define DML_VIEW_HPP

#include <siege/platform/win/desktop/window_factory.hpp>
#include "dml_controller.hpp"

namespace siege::views
{
  struct dml_view : win32::window_ref
  {
    dml_controller controller;
    win32::list_box ref_names;

    dml_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto wm_create()
    {
      auto control_factory = win32::window_factory(ref());

      ref_names = *control_factory.CreateWindowExW<win32::list_box>(::CREATESTRUCTW{
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

      if (controller.is_material(stream))
      {
        auto size = controller.load_material(stream);

        if (size > 0)
        {
          for (auto i = 0u; i < size; ++i)
          {
            auto filename = controller.get_filename(i);
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
  };
}// namespace siege::views

#endif