#ifndef PAL_VIEW_HPP
#define PAL_VIEW_HPP

#include <string_view>
#include <istream>
#include <spanstream>
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include "cfg_controller.hpp"

namespace siege::views
{
  struct cfg_view : win32::window_ref
  {
    cfg_controller controller;

    cfg_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    auto on_create(const win32::create_message&)
    {
      auto control_factory = win32::window_factory(ref());

      on_setting_change(win32::setting_change_message{ 0, (LPARAM)L"ImmersiveColorSet" });
      
      return 0;
    }

    auto on_size(win32::size_message sized)
    {
     
      return 0;
    }

    auto on_copy_data(win32::copy_data_message<char> message)
    {
     
      return FALSE;
    }

    std::optional<win32::lresult_t> on_setting_change(win32::setting_change_message message)
    {
      if (message.setting == L"ImmersiveColorSet")
      {
        return 0;
      }

      return std::nullopt;
    }
  };
}// namespace siege::views

#endif