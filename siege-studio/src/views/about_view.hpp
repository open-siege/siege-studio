#ifndef SIEGE_ABOUT_VIEW_HPP
#define SIEGE_ABOUT_VIEW_HPP

#include <utility>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/window_factory.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/wic.hpp>
#include <siege/platform/win/capabilities.hpp>

namespace siege::views
{
  struct about_view final : win32::window_ref
  {
    win32::static_control heading;
    win32::static_control logo;

    win32::gdi::icon logo_icon;
    win32::gdi::bitmap logo_preview;

    win32::window text_stack;

    about_view(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
    {
      logo_icon.reset((HICON)::LoadImageW(params.hInstance, L"AppIcon", IMAGE_ICON, 0, 0, 0));
      win32::apply_window_theme(*this);
    }

    auto wm_create()
    {
      win32::window_factory factory(ref());

      ::SetWindowTextW(*this, L"About Siege Studio");

      heading = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = L"Siege Studio is an open-source reverse engeering tool to preview, convert or extract files from various games made by Dynamix and using their tecnologies." });
      logo = *factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE | SS_REALSIZEIMAGE });
      text_stack = win32::window(CreateWindowExW(0, L"StackLayout", nullptr, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, *this, nullptr, nullptr, nullptr));
   
      text_stack.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_PORTRAIT);
      text_stack.SetPropW(L"DefaultHeight", win32::get_system_metrics(SM_CYSIZE) * 2);

      win32::window_factory stack_factory(text_stack.ref());


      auto gdi_version = win32::get_gdi_plus_version();

      if (gdi_version)
      {
        stack_factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = std::wstring(L"GDI+ Version: " + std::to_wstring(gdi_version->major) + L"." + std::to_wstring(gdi_version->minor)).c_str() });
      }
      else
      {
        stack_factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = L"GDI+ not available" });
      }

      auto wic_version = win32::get_wic_version();

      if (wic_version)
      {
        stack_factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = std::wstring(L"Windows Imaging Component Version: " + std::to_wstring(wic_version->major) + L"." + std::to_wstring(wic_version->minor)).c_str() });
      }
      else
      {
        stack_factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = L"Windows Imaging Component not available" });
      }

      auto wam_version = win32::get_scenic_animation_version();
      auto direct2d_version = win32::get_direct2d_version();
      auto directwrite_version = win32::get_direct_write_version();

      stack_factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = L"Windows Animation Manager Version: " });
      stack_factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = L"Direct2D Version: " });
      stack_factory.CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .style = WS_CHILD | WS_VISIBLE, .lpszName = L"DirectWrite Version: " });

      return 0;
    }

    auto wm_destroy()
    {
      ::DeleteObject((HBITMAP)::SendMessageW(logo, STM_GETIMAGE, IMAGE_BITMAP, 0));
      return 0;
    }

    auto wm_size(std::size_t type, SIZE client_size)
    {
      if (type == SIZE_MAXIMIZED || type == SIZE_RESTORED)
      {
        auto top_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 8 };
        auto middle_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy / 4 };
        auto bottom_size = SIZE{ .cx = client_size.cx, .cy = client_size.cy - middle_size.cy - top_size.cy };

        heading.SetWindowPos(POINT{});
        heading.SetWindowPos(top_size);

        logo.SetWindowPos(POINT{ .x = 0, .y = top_size.cy });
        logo.SetWindowPos(middle_size);

        text_stack.SetWindowPos(POINT{ .x = 0, .y = middle_size.cy + top_size.cy });
        text_stack.SetWindowPos(bottom_size);

        auto min = std::min(bottom_size.cx, bottom_size.cy);
        auto image_size = SIZE{ .cx = min, .cy = min };

        win32::gdi::bitmap target(image_size, win32::gdi::bitmap::skip_shared_handle);

        win32::wic::bitmap(win32::gdi::icon_ref(logo_icon))
          .scale(image_size.cx, image_size.cy, win32::wic::interpolation_mode::WICBitmapInterpolationModeFant)
          .copy_pixels(target.get_stride(), target.get_pixels_as_bytes());

        auto result = ::SendMessageW(logo, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)target.get());

        if (result != (LRESULT)target.get())
        {
          DeleteObject((HBITMAP)result);
        }

        result = ::SendMessageW(logo, STM_GETIMAGE, IMAGE_BITMAP, 0);

        if (result == (LRESULT)target.get())
        {
          target.release();
        }
      }

      return 0;
    }

    std::optional<LRESULT> wm_close()
    {
      EndDialog(*this, 0);
      return 0;
    }
  };
}// namespace siege::views

#endif