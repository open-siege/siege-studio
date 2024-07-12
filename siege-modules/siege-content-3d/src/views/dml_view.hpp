#ifndef DML_VIEW_HPP
#define DML_VIEW_HPP

#include <siege/platform/win/desktop/window_factory.hpp>
#include "dml_controller.hpp"

namespace siege::views
{
  struct dml_view : win32::window_ref
  {
    dml_controller controller;

    dml_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto on_copy_data(win32::copy_data_message<char> message)
    {
      std::spanstream stream(message.data);

      if (controller.is_material(stream))
      {
        auto size = controller.load_material(stream);

        if (size > 0)
        {
          return TRUE;
        }
      }

      return FALSE;
    }
  };
}// namespace siege::views

#endif