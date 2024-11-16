
#include <siege/platform/win/desktop/common_controls.hpp>
#include <siege/platform/win/desktop/drawing.hpp>
#include <siege/platform/win/desktop/window_factory.hpp>
#include <siege/platform/win/desktop/theming.hpp>
#include <siege/platform/win/desktop/dialog.hpp>
#include "input-filter.hpp"
#include "input_injector.hpp"
#include "exe_views.hpp"

namespace siege::views
{
  void exe_view::controller_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message)
  {
    struct handler : win32::window_ref
    {
      handler(win32::hwnd_t self, const CREATESTRUCTW& params) : win32::window_ref(self)
      {
      }

      auto wm_create()
      {
        SetWindowTextW(*this, L"Press a keyboard or mouse button");
        SetTimer(*this, 1, USER_TIMER_MINIMUM, nullptr);
        return 0;
      }

      auto wm_timer(std::size_t id, TIMERPROC)
      {
        constexpr static std::array<SHORT, 8> states = { { VK_RETURN, VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_TAB, VK_SNAPSHOT } };

        for (auto state : states)
        {
          if (::GetKeyState(state) & 0x80)
          {
            KillTimer(*this, 1);
            EndDialog(*this, state);
            break;
          }
        }

        return 0;
      }

      auto wm_mouse_button_down(std::size_t state, POINTS)
      {
        if (state & MK_LBUTTON)
        {
          KillTimer(*this, 1);
          EndDialog(*this, VK_LBUTTON);
        }

        if (state & MK_RBUTTON)
        {
          KillTimer(*this, 1);
          EndDialog(*this, VK_RBUTTON);
        }

        if (state & MK_MBUTTON)
        {
          KillTimer(*this, 1);
          EndDialog(*this, VK_MBUTTON);
        }

        if (state & MK_XBUTTON1)
        {
          KillTimer(*this, 1);
          EndDialog(*this, VK_XBUTTON1);
        }

        if (state & MK_XBUTTON2)
        {
          KillTimer(*this, 1);
          EndDialog(*this, MK_XBUTTON2);
        }

        return 0;
      }

      auto wm_sys_key_down(KEYBDINPUT input)
      {
        KillTimer(*this, 1);

        if (input.wVk == VK_MENU)
        {
          if (::GetKeyState(VK_LMENU) & 0x80)
          {
            EndDialog(*this, VK_LMENU);
          }
          else if (::GetKeyState(VK_RMENU) & 0x80)
          {
            EndDialog(*this, VK_RMENU);
          }
        }
        else
        {
          EndDialog(*this, input.wVk);
        }

        return 0;
      }

      auto wm_key_down(KEYBDINPUT input)
      {
        KillTimer(*this, 1);

        if (input.wVk == VK_SHIFT)
        {
          if (::GetKeyState(VK_LSHIFT) & 0x80)
          {
            EndDialog(*this, VK_LSHIFT);
          }
          else if (::GetKeyState(VK_RSHIFT) & 0x80)
          {
            EndDialog(*this, VK_RSHIFT);
          }
        }
        else if (input.wVk == VK_CONTROL)
        {
          if (::GetKeyState(VK_LCONTROL) & 0x80)
          {
            EndDialog(*this, VK_LCONTROL);
          }
          else if (::GetKeyState(VK_RCONTROL) & 0x80)
          {
            EndDialog(*this, VK_RCONTROL);
          }
        }
        else
        {
          EndDialog(*this, input.wVk);
        }
        return 0;
      }
    };

    auto result = win32::DialogBoxIndirectParamW<handler>(win32::module_ref::current_application(),
      win32::default_dialog({ .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 200, .cy = 100 }),
      ref());

    auto item = controller_table.GetItem(LVITEMW{
      .mask = LVIF_PARAM,
      .iItem = message.iItem });

    if (item && item->lParam)
    {
      auto game_pad_code = LOWORD(item->lParam);

      std::wstring temp = category_for_vkey(result);
      ListView_SetItemText(controller_table, message.iItem, 1, temp.data());
      temp = string_for_vkey(result);
      ListView_SetItemText(controller_table, message.iItem, 2, temp.data());

      LVITEMW item{
        .mask = LVIF_PARAM,
        .iItem = message.iItem,
        .lParam = MAKELPARAM(game_pad_code, result)
      };
      ListView_SetItem(controller_table, &item);
    }
  }

}// namespace siege::views