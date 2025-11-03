#ifndef SIEGE_ABOUT_VIEW_HPP
#define SIEGE_ABOUT_VIEW_HPP

#include <utility>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/basic_window.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/wic.hpp>
#include <siege/platform/win/capabilities.hpp>
#include <imagehlp.h>

namespace fs = std::filesystem;

std::optional<SIZE> get_module_version()
{
  auto dll_path = fs::path(win32::module_ref::current_module().GetModuleFileName());
  std::error_code last_error;

  if (fs::exists(dll_path, last_error))
  {
    LOADED_IMAGE image{};
    auto dll_string = dll_path.filename().string();
    auto dll_path_string = dll_path.parent_path().string();
    if (::MapAndLoad(dll_string.c_str(), dll_path_string.c_str(), &image, dll_path.extension().string() == ".dll" ? TRUE : FALSE, TRUE))
    {
      SIZE result{};
      result.cx = image.FileHeader->OptionalHeader.MajorImageVersion;
      result.cy = image.FileHeader->OptionalHeader.MinorImageVersion;
      ::UnMapAndLoad(&image);

      if (result.cx && result.cy)
      {
        return result;
      }
    }
  }

  return std::nullopt;
}

namespace siege::views
{
  struct about_view final : win32::basic_window<about_view>
  {
    win32::static_control heading;
    win32::static_control logo;

    win32::gdi::icon logo_icon;
    win32::gdi::bitmap logo_preview;

    win32::window text_stack;

    // https://github.com/open-siege/siege-studio/

    // https://thesiegehub.itch.io/siege-studio

    about_view(win32::hwnd_t self, CREATESTRUCTW& params) : basic_window(self, params)
    {
      logo_icon.reset((HICON)::LoadImageW(params.hInstance, L"AppIcon", IMAGE_ICON, 0, 0, 0));
      win32::apply_window_theme(*this);
    }

    auto wm_create()
    {
      ::SetWindowTextW(*this, L"About Siege Studio");


      text_stack = win32::window(CreateWindowExW(0, L"StackLayout", nullptr, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, *this, nullptr, nullptr, nullptr));

      heading = *win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = L"Siege Studio is an open-source reverse engeering tool to preview, convert or extract files from various games, like Starsiege, Quake and many more." });

      logo = *win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE | SS_REALSIZEIMAGE });

      auto github_link = *win32::CreateWindowExW<win32::button>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE | BS_COMMANDLINK, .lpszName = L"https://github.com/open-siege/siege-studio/" });
      github_link.bind_bn_clicked([](auto sender, auto&) {
        std::array<wchar_t, 255> temp{};
        ::GetWindowTextW(sender, temp.data(), temp.size());
        ::ShellExecuteW(NULL, L"open", temp.data(), nullptr, nullptr, SW_SHOWNORMAL);
      });

      auto itch_link = *win32::CreateWindowExW<win32::button>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE | BS_COMMANDLINK, .lpszName = L"https://thesiegehub.itch.io/siege-studio/" });
      itch_link.bind_bn_clicked([](auto sender, auto&) {
        std::array<wchar_t, 255> temp{};
        ::GetWindowTextW(sender, temp.data(), temp.size());
        ::ShellExecuteW(NULL, L"open", temp.data(), nullptr, nullptr, SW_SHOWNORMAL);
      });

      text_stack.SetPropW(L"Orientation", ORIENTATION_PREFERENCE::ORIENTATION_PREFERENCE_PORTRAIT);
      text_stack.SetPropW(L"DefaultHeight", win32::get_system_metrics(SM_CYSIZE) * 2);

      auto version_to_check = get_module_version().value_or(SIZE{ SIEGE_MAJOR_VERSION, SIEGE_MINOR_VERSION });

      win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = std::wstring(L"Siege Studio version: " + std::to_wstring(version_to_check.cx) + L"." + std::to_wstring(version_to_check.cy)).c_str() });

      auto gdi_version = win32::get_gdi_plus_version();

      if (gdi_version)
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = std::wstring(L"GDI+ version: " + std::to_wstring(gdi_version->major) + L"." + std::to_wstring(gdi_version->minor)).c_str() });
      }
      else
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = L"GDI+ not available" });
      }

      auto wic_version = win32::get_wic_version();

      if (wic_version)
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = std::wstring(L"Windows Imaging Component version: " + std::to_wstring(wic_version->major) + L"." + std::to_wstring(wic_version->minor)).c_str() });
      }
      else
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = L"Windows Imaging Component not available" });
      }

      auto wam_version = win32::get_scenic_animation_version();

      if (wam_version)
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = std::wstring(L"Windows Animation Manager version: " + std::to_wstring(wam_version->major) + L"." + std::to_wstring(wam_version->minor)).c_str() });
      }
      else
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = L"Windows Animation Manager not available" });
      }

      auto direct2d_version = win32::get_direct2d_version();

      if (direct2d_version)
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = std::wstring(L"Direct2D version: " + std::to_wstring(direct2d_version->major) + L"." + std::to_wstring(direct2d_version->minor)).c_str() });
      }
      else
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = L"Direct2D not available" });
      }

      auto directwrite_version = win32::get_direct_write_version();

      if (directwrite_version)
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = std::wstring(L"DirectWrite version: " + std::to_wstring(directwrite_version->major) + L"." + std::to_wstring(direct2d_version->minor)).c_str() });
      }
      else
      {
        win32::CreateWindowExW<win32::static_control>(CREATESTRUCTW{ .hwndParent = text_stack, .style = WS_CHILD | WS_VISIBLE, .lpszName = L"DirectWrite not available" });
      }

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
        auto top_size = SIZE{ .cx = client_size.cx - 10, .cy = client_size.cy / 16 };
        auto middle_size = SIZE{ .cx = client_size.cx - 10, .cy = client_size.cy / 2 };
        auto bottom_size = SIZE{ .cx = client_size.cx - 10, .cy = client_size.cy };

        heading.SetWindowPos(POINT{});
        heading.SetWindowPos(top_size);

        logo.SetWindowPos(POINT{ .x = 0, .y = top_size.cy });
        logo.SetWindowPos(middle_size);

        text_stack.SetWindowPos(POINT{ .x = 5, .y = 5 });
        text_stack.SetWindowPos(bottom_size);

        auto min = std::min(middle_size.cx, middle_size.cy);
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

    std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override
    {
      switch (message)
      {
      case WM_CREATE:
        return wm_create();
      case WM_SIZE:
        return wm_size((std::size_t)wparam, SIZE(LOWORD(lparam), HIWORD(lparam)));
      case WM_CLOSE:
        return wm_close();
      default:
        return std::nullopt;
      }
    }
  };

  void show_about_dialog(win32::window_ref parent)
  {
    win32::DialogBoxIndirectParamW<about_view>(::GetModuleHandleW(nullptr),
      win32::default_dialog{ { .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 350, .cy = 400 } },
      std::move(parent));
  }
}// namespace siege::views

#endif