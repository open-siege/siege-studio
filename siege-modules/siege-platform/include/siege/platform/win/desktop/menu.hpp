#ifndef WIN32_MENU_HPP
#define WIN32_MENU_HPP

#include <siege/platform/win/desktop/messages.hpp>
#include <map>
#include <initguid.h>
#include <oleacc.h>

namespace win32
{
  struct menu_no_deleter
  {
    void operator()(HMENU menu)
    {
    }
  };

  struct menu_deleter
  {
    void operator()(HMENU menu)
    {
      ::DestroyMenu(menu);
    }
  };

  template<typename TDeleter>
  struct menu_base : win32::auto_handle<HMENU, TDeleter>
  {
    using base = win32::auto_handle<HMENU, TDeleter>;
    using base::base;

    template<typename TMenu>
    struct notifications
    {
      virtual std::optional<win32::lresult_t> wm_command(TMenu, int)
      {
        return std::nullopt;
      }

      virtual std::optional<win32::lresult_t> wm_menu_command(TMenu, int)
      {
        return std::nullopt;
      }

      virtual std::optional<win32::lresult_t> wm_draw_item(TMenu, DRAWITEMSTRUCT&)
      {
        return std::nullopt;
      }

      virtual SIZE wm_measure_item(TMenu, const MEASUREITEMSTRUCT&)
      {
        return SIZE{};
      }

      template<typename TWindow>
      static std::optional<lresult_t> dispatch_message(TWindow* window, std::uint32_t message, wparam_t wParam, lparam_t lParam)
      {
        if constexpr (std::is_base_of_v<notifications, TWindow>)
        {
          auto* self = static_cast<notifications*>(window);

          if (message == WM_COMMAND && HIWORD(wParam) == 0)
          {
            return self->wm_command(TMenu(nullptr), LOWORD(wParam));
          }

          if (message == WM_MENUCOMMAND)
          {
            return self->wm_menu_command(TMenu((HMENU)lParam), wParam);
          }

          if (message == WM_MEASUREITEM)
          {
            auto& context = *(MEASUREITEMSTRUCT*)lParam;

            if (context.CtlType == ODT_MENU)
            {
              auto size = self->wm_measure_item(TMenu(nullptr), context);
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
              return self->wm_draw_item(TMenu((HMENU)context.hwndItem), context);
            }
          }
        }

        return std::nullopt;
      }
    };


    struct co_task_deleter
    {
      void operator()(void* data)
      {
        ::CoTaskMemFree(data);
      }
    };

    struct menu_string_info : MSAAMENUINFO
    {
      menu_string_info(std::wstring_view data) : MSAAMENUINFO{ .dwMSAASignature = MSAA_MENU_SIG, .cchWText = data.size(), .pszWText = ::SysAllocString(data.data()) }
      {
      }

      ~menu_string_info()
      {
        if (pszWText)
        {
          ::SysFreeString(pszWText);
        }
      }
    };

    auto AppendMenuW(UINT flags, UINT_PTR id, std::wstring_view data = L"")
    {
      if (flags & MF_OWNERDRAW && !data.empty())
      {
        static std::map<std::wstring_view, std::unique_ptr<menu_string_info, co_task_deleter>> menu_items;

        auto iter = menu_items.find(data);

        if (iter == menu_items.end())
        {
          auto* storage = ::CoTaskMemAlloc(sizeof(menu_string_info));
          auto* acc_info = new (storage) menu_string_info(data);

          iter = menu_items.emplace(data, std::unique_ptr<menu_string_info, co_task_deleter>(acc_info)).first;
        }

        MENUITEMINFOW info{
          .cbSize = sizeof(MENUITEMINFOW),
          .fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID
        };        

        info.fType = flags;
        info.dwItemData = (ULONG_PTR)iter->second.get();
        info.wID = id;
        return this->InsertMenuItemW(id, FALSE, info);
      }

      return ::AppendMenuW(*this, flags, id, data.data());
    }


    auto InsertMenuItemW(UINT item, BOOL by_position, MENUITEMINFOW info)
    {
      return ::InsertMenuItemW(*this, item, by_position, &info);
    }
  };

  struct menu_ref : menu_base<menu_no_deleter>
  {
    using base = menu_base<menu_no_deleter>;
    using base::base;
    using notifications = base::notifications<menu_ref>;
  };

  struct menu : menu_base<menu_deleter>
  {
    using base = menu_base<menu_deleter>;
    using notifications = base::notifications<menu_ref>;

    menu()
    {
      this->reset(::CreateMenu());
    }
  };

  template<typename TDeleter>
  struct popup_menu_base : menu_base<TDeleter>
  {
    using base = menu_base<TDeleter>;
    using base::base;

    inline auto TrackPopupMenuEx(UINT flags, POINT coords, hwnd_t owner, std::optional<TPMPARAMS> params = std::nullopt)
    {
      if (params)
      {
        params.value().cbSize = sizeof(TPMPARAMS);
        return ::TrackPopupMenuEx(*this, flags, coords.x, coords.y, owner, &params.value());
      }
      else
      {
        return ::TrackPopupMenuEx(*this, flags, coords.x, coords.y, owner, nullptr);
      }
    }
  };

  struct popup_menu_ref : popup_menu_base<menu_no_deleter>
  {
    using base = popup_menu_base<menu_no_deleter>;
    using base::base;
    using notifications = base::notifications<popup_menu_ref>;
  };

  struct popup_menu : popup_menu_base<menu_deleter>
  {
    using base = popup_menu_base<menu_deleter>;
    using base::base;
    using notifications = base::notifications<popup_menu_ref>;

    popup_menu()
    {
      this->reset(::CreatePopupMenu());
    }
  };
}// namespace win32

#endif