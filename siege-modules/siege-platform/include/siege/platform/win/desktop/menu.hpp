#ifndef WIN32_MENU_HPP
#define WIN32_MENU_HPP

#include <siege/platform/win/desktop/messages.hpp>

namespace win32
{
  struct menu_no_deleter
  {
    void operator()(HMENU menu)
    {
    }
  };

  struct menu : win32::auto_handle<HMENU, menu_no_deleter>
  {
    using base = win32::auto_handle<HMENU, menu_no_deleter>;
    using base::base;

    struct notifications
    {
      virtual std::optional<win32::lresult_t> wm_command(menu, int)
      {
        return std::nullopt;
      }

      virtual std::optional<win32::lresult_t> wm_menu_command(menu, int)
      {
        return std::nullopt;
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(menu, DRAWITEMSTRUCT&)
      {
        return std::nullopt;
      }

      virtual SIZE wm_measure_item(menu, const MEASUREITEMSTRUCT&)
      {
        return SIZE{};
      }

      template<typename TWindow>
      static std::optional<lresult_t> dispatch_message(TWindow* self, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
          if (message == WM_COMMAND && HIWORD(wParam) == 0)
          {
            return self->wm_command(menu(GetMenu(*self)), LOWORD(wParam));
          }

          if (message == WM_MENUCOMMAND)
          {
            return self->wm_menu_command(menu((HMENU)lParam), wParam);
          }

          if (message == WM_MEASUREITEM)
          {
            auto& context = *(MEASUREITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_MENU)
            {
              auto size = self->wm_measure_item(menu(GetMenu(*self)), context);
              context.itemWidth = size.cx;
              context.itemHeight = size.cy;
              return 0;
            }
          }

          if (message == WM_DRAWITEM)
          {
            auto& context = *(DRAWITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_MENU)
            {
              return self->wm_draw_item(menu((HMENU)context.hwndItem), context);
            }
          }
        }

        return std::nullopt;
      }
    };
  };

  inline auto TrackPopupMenuEx(HMENU menu, UINT flags, POINT coords, hwnd_t owner, std::optional<TPMPARAMS> params = std::nullopt)
  {
    if (params)
    {
      params.value().cbSize = sizeof(TPMPARAMS);
      return ::TrackPopupMenuEx(menu, flags, coords.x, coords.y, owner, &params.value());
    }
    else
    {
      return ::TrackPopupMenuEx(menu, flags, coords.x, coords.y, owner, nullptr);
    }
  }
}// namespace win32

#endif