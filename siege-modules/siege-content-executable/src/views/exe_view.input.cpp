
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include "input-filter.hpp"
#include "input_injector.hpp"
#include "exe_views.hpp"

namespace siege::views
{
  void exe_view::populate_controller_table(std::span<siege::platform::game_action> actions, std::span<const wchar_t*> controller_input_backends)
  {
    if (controller_input_backends.empty())
    {
      int id = 1;
      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"D-Pad"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });
      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Face Buttons"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });

      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Bumpers and Triggers"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });
      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Left Stick"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });

      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"Right Stick"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });

      controller_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = const_cast<wchar_t*>(L"System Buttons"),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });


      std::map<WORD, WORD> default_mappings = {
        { VK_GAMEPAD_DPAD_UP, VK_UP },
        { VK_GAMEPAD_DPAD_DOWN, VK_DOWN },
        { VK_GAMEPAD_DPAD_LEFT, VK_LEFT },
        { VK_GAMEPAD_DPAD_RIGHT, VK_RIGHT },
        { VK_GAMEPAD_A, VK_SPACE },
        { VK_GAMEPAD_B, VK_LCONTROL },
        { VK_GAMEPAD_X, 'F' },
        { VK_GAMEPAD_Y, VK_OEM_4 },
        { VK_GAMEPAD_LEFT_SHOULDER, 'G' },
        { VK_GAMEPAD_RIGHT_SHOULDER, 'T' },
        { VK_GAMEPAD_LEFT_TRIGGER, VK_RBUTTON },
        { VK_GAMEPAD_RIGHT_TRIGGER, VK_LBUTTON },
        { VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, VK_LSHIFT },
        { VK_GAMEPAD_LEFT_THUMBSTICK_UP, 'W' },
        { VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, 'S' },
        { VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, 'A' },
        { VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, 'D' },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, 'E' },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_UP, WM_MOUSEMOVE },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, WM_MOUSEMOVE + 1 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, WM_MOUSEMOVE + 2 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, WM_MOUSEMOVE + 3 },
      };

      std::map<WORD, int> images = {
        { VK_GAMEPAD_A, 0 },
        { VK_GAMEPAD_B, 1 },
        { VK_GAMEPAD_X, 2 },
        { VK_GAMEPAD_Y, 3 },
        { VK_GAMEPAD_LEFT_SHOULDER, 4 },
        { VK_GAMEPAD_RIGHT_SHOULDER, 5 },
        { VK_GAMEPAD_LEFT_TRIGGER, 6 },
        { VK_GAMEPAD_RIGHT_TRIGGER, 7 },
        { VK_GAMEPAD_DPAD_UP, 9 },
        { VK_GAMEPAD_DPAD_DOWN, 10 },
        { VK_GAMEPAD_DPAD_LEFT, 11 },
        { VK_GAMEPAD_DPAD_RIGHT, 12 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, 13 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, 14 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_UP, 15 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, 16 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, 17 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, 18 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_UP, 15 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, 16 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, 17 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, 18 },
      };

      std::map<WORD, int> grouping = {
        { VK_GAMEPAD_DPAD_UP, 1 },
        { VK_GAMEPAD_DPAD_DOWN, 1 },
        { VK_GAMEPAD_DPAD_LEFT, 1 },
        { VK_GAMEPAD_DPAD_RIGHT, 1 },
        { VK_GAMEPAD_A, 2 },
        { VK_GAMEPAD_B, 2 },
        { VK_GAMEPAD_X, 2 },
        { VK_GAMEPAD_Y, 2 },
        { VK_GAMEPAD_LEFT_SHOULDER, 3 },
        { VK_GAMEPAD_RIGHT_SHOULDER, 3 },
        { VK_GAMEPAD_LEFT_TRIGGER, 3 },
        { VK_GAMEPAD_RIGHT_TRIGGER, 3 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, 4 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, 5 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_UP, 4 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, 4 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, 4 },
        { VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, 4 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_UP, 5 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, 5 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, 5 },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, 5 },
      };

      for (auto mapping : default_mappings)
      {
        win32::list_view_item up(string_for_vkey(mapping.first), images[mapping.first]);
        up.iGroupId = grouping[mapping.first];
        up.lParam = MAKELPARAM(mapping.first, mapping.second);
        up.mask = up.mask | LVIF_PARAM;

        up.sub_items.emplace_back(category_for_vkey(mapping.second));
        up.sub_items.emplace_back(string_for_vkey(mapping.second));
        controller_table.InsertRow(up);
      }
    }
    else
    {
      /*
          enum
    {
      unknown,
      digital,
      analog
    } type;
    std::array<char, 32> action_name;
    std::array<char16_t, 64> action_display_name;
    std::array<char16_t, 64> group_display_name;
      */

      std::set<std::u16string_view> grouping = {};

      int id = 1;
      for (auto& action : actions)
      {
        grouping.emplace(action.group_display_name.data());

        controller_table.InsertGroup(-1, LVGROUP{
                                           .pszHeader = (wchar_t*)(action.group_display_name.data()),
                                           .iGroupId = id++,
                                           .state = LVGS_COLLAPSIBLE,
                                         });
      }

      for (auto& action : actions)
      {
        win32::list_view_item up((wchar_t*)action.action_display_name.data());
        auto iter = grouping.find(action.group_display_name.data());
        up.iGroupId = std::distance(grouping.begin(), iter) + 1;
        //up.lParam = MAKELPARAM(mapping.first, mapping.second);

        //        up.sub_items.emplace_back(category_for_vkey(mapping.second));
        up.sub_items.emplace_back((wchar_t*)action.group_display_name.data());
        controller_table.InsertRow(up);
      }
    }
  }

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