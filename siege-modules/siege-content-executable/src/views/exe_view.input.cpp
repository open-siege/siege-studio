
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
      std::set<std::u16string_view> grouping = {};
      std::map<std::u16string_view, int> ids_for_grouping = {};

      int id = 1;
      for (auto& action : actions)
      {
        auto iter = grouping.emplace(action.group_display_name.data());

        if (iter.second)
        {
          ids_for_grouping[action.group_display_name.data()] = id;
          controller_table.InsertGroup(-1, LVGROUP{
                                             .pszHeader = (wchar_t*)(action.group_display_name.data()),
                                             .iGroupId = id++,
                                             .state = LVGS_COLLAPSIBLE,
                                           });
        }
      }

      for (auto& action : actions)
      {
        win32::list_view_item up((wchar_t*)action.action_display_name.data());
        up.mask = up.mask | LVIF_GROUPID;
        up.iGroupId = ids_for_grouping[action.group_display_name.data()];
        // up.lParam = MAKELPARAM(mapping.first, mapping.second);

        //        up.sub_items.emplace_back(category_for_vkey(mapping.second));
        up.sub_items.emplace_back((wchar_t*)action.group_display_name.data());
        controller_table.InsertRow(up);
      }
    }
  }

  void exe_view::populate_keyboard_table(std::span<siege::platform::game_action> actions, std::span<const wchar_t*> controller_input_backends)
  {
    if (actions.empty())
    {
      return;
    }

    auto kb_bindings = controller.get_extension().init_keyboard_inputs();
    auto ms_bindings = controller.get_extension().init_mouse_inputs();

    std::set<std::u16string_view> grouping = {};
    std::map<std::u16string_view, int> ids_for_grouping = {};

    int id = 1;

    for (auto& action : actions)
    {
      auto iter = grouping.emplace(action.group_display_name.data());

      if (iter.second)
      {
        ids_for_grouping[action.group_display_name.data()] = id;
        keyboard_table.InsertGroup(-1, LVGROUP{
                                         .pszHeader = (wchar_t*)(action.group_display_name.data()),
                                         .iGroupId = id++,
                                         .state = LVGS_COLLAPSIBLE,
                                       });
      }
    }

    struct context
    {
      std::variant<siege::platform::keyboard_binding::action_binding, siege::platform::mouse_binding::action_binding> binding;
      siege::platform::game_action action;
      WORD action_index;
    };

    std::vector<context> action_settings;
    action_settings.reserve((*kb_bindings)->inputs.size());

    for (auto& binding : (*kb_bindings)->inputs)
    {
      if (binding.input_type == binding.unknown)
      {
        break;
      }
      auto action_iter = std::find_if(actions.begin(), actions.end(), [&](auto& action) { return action.action_name == binding.action_name; });

      if (action_iter != actions.end())
      {
        action_settings.emplace_back(context{ binding, *action_iter, (WORD)(std::distance(actions.begin(), action_iter) + 1) });
      }
    }

    for (auto& binding : (*ms_bindings)->inputs)
    {
      if (binding.input_type == binding.unknown)
      {
        break;
      }
      auto action_iter = std::find_if(actions.begin(), actions.end(), [&](auto& action) { return action.action_name == binding.action_name; });

      if (action_iter != actions.end())
      {
        action_settings.emplace_back(context{ binding, *action_iter, (WORD)(std::distance(actions.begin(), action_iter) + 1) });
      }
    }

    for (auto& context : action_settings)
    {
      win32::list_view_item up((wchar_t*)context.action.action_display_name.data());
      up.mask = up.mask | LVIF_GROUPID | LVIF_PARAM;
      up.iGroupId = ids_for_grouping[context.action.group_display_name.data()];
      auto vkey = std::visit([](auto& binding) { return binding.virtual_key; }, context.binding);
      up.lParam = MAKELPARAM(vkey, context.action_index);
      up.sub_items.emplace_back(string_for_vkey(std::visit([](auto& binding) { return binding.virtual_key; }, context.binding)));
      keyboard_table.InsertRow(up);
    }
  }

  std::optional<LRESULT> exe_view::handle_keyboard_mouse_press(win32::window_ref dialog, INT message, WPARAM wparam, LPARAM lparam)
  {
    switch (message)
    {
    case WM_INITDIALOG: {
      SetWindowTextW(dialog, L"Press a keyboard or mouse button");
      SetTimer(dialog, 1, USER_TIMER_MINIMUM, nullptr);
      return FALSE;
    }
    case WM_TIMER: {
      constexpr static std::array<SHORT, 8> states = { { VK_RETURN, VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_TAB, VK_SNAPSHOT } };

      for (auto state : states)
      {
        if (::GetKeyState(state) & 0x80)
        {
          KillTimer(dialog, 1);
          EndDialog(dialog, state);
          break;
        }
      }
      return std::nullopt;
    }
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN: {
      if (wparam & MK_LBUTTON)
      {
        KillTimer(dialog, 1);
        EndDialog(dialog, VK_LBUTTON);
      }

      if (wparam & MK_RBUTTON)
      {
        KillTimer(dialog, 1);
        EndDialog(dialog, VK_RBUTTON);
      }

      if (wparam & MK_MBUTTON)
      {
        KillTimer(dialog, 1);
        EndDialog(dialog, VK_MBUTTON);
      }

      if (wparam & MK_XBUTTON1)
      {
        KillTimer(dialog, 1);
        EndDialog(dialog, VK_XBUTTON1);
      }

      if (wparam & MK_XBUTTON2)
      {
        KillTimer(dialog, 1);
        EndDialog(dialog, MK_XBUTTON2);
      }

      return 0;
    }
    case WM_SYSKEYDOWN: {
      KillTimer(dialog, 1);

      if (wparam == VK_MENU)
      {
        if (::GetKeyState(VK_LMENU) & 0x80)
        {
          EndDialog(dialog, VK_LMENU);
        }
        else if (::GetKeyState(VK_RMENU) & 0x80)
        {
          EndDialog(dialog, VK_RMENU);
        }
      }
      else
      {
        EndDialog(dialog, wparam);
      }

      return 0;
    }
    case WM_KEYDOWN: {
      KillTimer(dialog, 1);
      if (wparam == VK_SHIFT)
      {
        if (::GetKeyState(VK_LSHIFT) & 0x80)
        {
          EndDialog(dialog, VK_LSHIFT);
        }
        else if (::GetKeyState(VK_RSHIFT) & 0x80)
        {
          EndDialog(dialog, VK_RSHIFT);
        }
      }
      else if (wparam == VK_CONTROL)
      {
        if (::GetKeyState(VK_LCONTROL) & 0x80)
        {
          EndDialog(dialog, VK_LCONTROL);
        }
        else if (::GetKeyState(VK_RCONTROL) & 0x80)
        {
          EndDialog(dialog, VK_RCONTROL);
        }
      }
      else
      {
        EndDialog(dialog, wparam);
      }
      return 0;
    }
    default:
      return std::nullopt;
    };
  }

  void exe_view::keyboard_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message)
  {
    auto result = win32::DialogBoxIndirectParamW(win32::module_ref::current_application(),
      win32::default_dialog({ .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 200, .cy = 100 }),
      ref(),
      std::bind_front(&exe_view::handle_keyboard_mouse_press, this));
  }

  void exe_view::controller_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message)
  {
    auto result = win32::DialogBoxIndirectParamW(win32::module_ref::current_application(),
      win32::default_dialog({ .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 200, .cy = 100 }),
      ref(),
      std::bind_front(&exe_view::handle_keyboard_mouse_press, this));

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