
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/extension/input_filter.hpp>
#include "input_injector.hpp"
#include "exe_view.hpp"

namespace siege::views
{
  void exe_view::populate_controller_table(std::span<siege::platform::game_action> actions, std::span<const wchar_t*> controller_input_backends)
  {
    auto set_tile_info = [this](auto index) {
      UINT columns[2] = { 1, 2 };
      int formats[2] = { LVCFMT_LEFT, LVCFMT_RIGHT };
      LVTILEINFO item_info{ .cbSize = sizeof(LVTILEINFO), .iItem = (int)index, .cColumns = 2, .puColumns = columns, .piColFmt = formats };

      ListView_SetTileInfo(controller_table, &item_info);
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
      { VK_GAMEPAD_VIEW, 19 },
      { VK_GAMEPAD_MENU, 20 },
    };


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


      using namespace siege::platform;
      std::map<WORD, std::pair<WORD, hardware_context>> default_mappings = {
        { VK_GAMEPAD_DPAD_UP, std::make_pair(VK_UP, hardware_context::keyboard) },
        { VK_GAMEPAD_DPAD_DOWN, std::make_pair(VK_DOWN, hardware_context::keyboard) },
        { VK_GAMEPAD_DPAD_LEFT, std::make_pair(VK_LEFT, hardware_context::keyboard) },
        { VK_GAMEPAD_DPAD_RIGHT, std::make_pair(VK_RIGHT, hardware_context::keyboard) },
        { VK_GAMEPAD_A, std::make_pair(VK_SPACE, hardware_context::keyboard) },
        { VK_GAMEPAD_B, std::make_pair(VK_LCONTROL, hardware_context::keyboard) },
        { VK_GAMEPAD_X, std::make_pair('F', hardware_context::keyboard) },
        { VK_GAMEPAD_Y, std::make_pair(VK_OEM_4, hardware_context::keyboard) },
        { VK_GAMEPAD_LEFT_SHOULDER, std::make_pair('G', hardware_context::keyboard) },
        { VK_GAMEPAD_RIGHT_SHOULDER, std::make_pair('T', hardware_context::keyboard) },
        { VK_GAMEPAD_LEFT_TRIGGER, std::make_pair(VK_RBUTTON, hardware_context::keyboard) },
        { VK_GAMEPAD_RIGHT_TRIGGER, std::make_pair(VK_LBUTTON, hardware_context::keyboard) },
        { VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, std::make_pair(VK_LSHIFT, hardware_context::keyboard) },
        { VK_GAMEPAD_LEFT_THUMBSTICK_UP, std::make_pair('W', hardware_context::keyboard) },
        { VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, std::make_pair('S', hardware_context::keyboard) },
        { VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, std::make_pair('A', hardware_context::keyboard) },
        { VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, std::make_pair('D', hardware_context::keyboard) },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, std::make_pair('E', hardware_context::keyboard) },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_UP, std::make_pair(VK_UP, hardware_context::mouse) },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, std::make_pair(VK_DOWN, hardware_context::mouse) },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, std::make_pair(VK_LEFT, hardware_context::mouse) },
        { VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, std::make_pair(VK_RIGHT, hardware_context::mouse) },
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
        win32::list_view_item up(string_for_vkey(mapping.first, siege::platform::hardware_context::controller_xbox), images[mapping.first]);
        up.iGroupId = grouping[mapping.first];

        up.lParam = bound_inputs.size();
        auto& input = bound_inputs.emplace_back();

        input.from_vkey = mapping.first;
        input.from_context = siege::platform::hardware_context::controller_xbox;
        input.to_vkey = mapping.second.first;
        input.to_context = mapping.second.second;
        up.mask = up.mask | LVIF_GROUPID | LVIF_PARAM;

        up.sub_items.emplace_back(category_for_vkey(mapping.second.first, mapping.second.second));
        up.sub_items.emplace_back(string_for_vkey(mapping.second.first, mapping.second.second));
        set_tile_info(controller_table.InsertRow(up));
      }
    }
    else
    {
      std::set<std::u16string_view> grouping = {};
      std::map<std::u16string_view, int> ids_for_grouping = {};

      auto bindings = get_extension(state).init_controller_inputs();

      if (!bindings)
      {
        return;
      }

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

      struct context
      {
        WORD vkey;
        siege::platform::hardware_context context;
        siege::platform::game_action action;
        WORD action_index;
      };

      std::vector<context> action_settings;
      action_settings.reserve((*bindings)->inputs.size());

      for (auto& binding : (*bindings)->inputs)
      {
        if (binding.input_type == binding.unknown)
        {
          break;
        }
        auto action_iter = std::find_if(actions.begin(), actions.end(), [&](auto& action) { return action.action_name == binding.action_name; });

        if (action_iter != actions.end())
        {
          action_settings.emplace_back(context{ binding.virtual_key, (siege::platform::hardware_context)binding.context, *action_iter, (WORD)(std::distance(actions.begin(), action_iter)) });
        }
      }

      for (auto& context : action_settings)
      {
        win32::list_view_item up((wchar_t*)context.action.action_display_name.data(), images[context.vkey]);
        up.mask = up.mask | LVIF_GROUPID | LVIF_PARAM;
        up.iGroupId = ids_for_grouping[context.action.group_display_name.data()];
        up.lParam = (LPARAM)add_action_binding(state, input_action_binding{ .vkey = context.vkey, .context = context.context, .action_index = context.action_index });

        set_tile_info(controller_table.InsertRow(up));
      }
    }
  }

  void exe_view::populate_keyboard_table(std::span<siege::platform::game_action> actions, std::span<const wchar_t*> controller_input_backends)
  {
    if (actions.empty())
    {
      return;
    }

    auto kb_bindings = get_extension(state).init_keyboard_inputs();
    auto ms_bindings = get_extension(state).init_mouse_inputs();

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

    struct ui_context
    {
      WORD vkey;
      siege::platform::hardware_context context;
      siege::platform::game_action action;
      WORD action_index;
    };

    std::vector<ui_context> action_settings;

    if (kb_bindings)
    {
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
          action_settings.emplace_back(ui_context{ binding.virtual_key, (siege::platform::hardware_context)binding.context, *action_iter, (WORD)(std::distance(actions.begin(), action_iter)) });
        }
      }
    }

    if (ms_bindings)
    {
      action_settings.reserve((*ms_bindings)->inputs.size());
      for (auto& binding : (*ms_bindings)->inputs)
      {
        if (binding.input_type == binding.unknown)
        {
          break;
        }
        auto action_iter = std::find_if(actions.begin(), actions.end(), [&](auto& action) { return action.action_name == binding.action_name; });

        if (action_iter != actions.end())
        {
          action_settings.emplace_back(ui_context{ binding.virtual_key, (siege::platform::hardware_context)binding.context, *action_iter, (WORD)(std::distance(actions.begin(), action_iter)) });
        }
      }
    }

    WORD action_index = 0;
    for (auto& action : actions)
    {
      auto action_iter = std::find_if(action_settings.begin(), action_settings.end(), [&](auto& context) { return context.action.action_name == action.action_name; });

      if (action_iter == action_settings.end())
      {
        action_settings.emplace_back(ui_context{ 0, siege::platform::hardware_context::global, action, action_index++ });
      }
    }

    for (auto& context : action_settings)
    {
      win32::list_view_item up((wchar_t*)context.action.action_display_name.data());
      up.mask = up.mask | LVIF_GROUPID | LVIF_PARAM;
      up.iGroupId = ids_for_grouping[context.action.group_display_name.data()];
      auto vkey = context.vkey;
      up.lParam = (LPARAM)add_action_binding(state, input_action_binding{ .vkey = vkey, .context = context.context, .action_index = context.action_index });
      up.sub_items.emplace_back(string_for_vkey(vkey, context.context));
      keyboard_table.InsertRow(up);
    }
  }

  std::optional<LRESULT> exe_view::handle_keyboard_mouse_press(win32::window_ref dialog, INT message, WPARAM wparam, LPARAM lparam)
  {
    using namespace siege::platform;
    static std::wstring temp;
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

          if (state == VK_RETURN)
          {
            auto context = hardware_context::keyboard;
            temp.resize(255);

            temp.resize(::GetKeyNameTextW(lparam, temp.data(), temp.size() + 1));

            context = temp == L"Numpad Enter" ? hardware_context::keypad : hardware_context::keyboard;
            EndDialog(dialog, MAKELRESULT(state, context));
          }
          else
          {
            EndDialog(dialog, MAKELRESULT(state, hardware_context::keyboard));
          }
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
        EndDialog(dialog, MAKELRESULT(VK_LBUTTON, hardware_context::mouse));
      }

      if (wparam & MK_RBUTTON)
      {
        KillTimer(dialog, 1);
        EndDialog(dialog, MAKELRESULT(VK_RBUTTON, hardware_context::mouse));
      }

      if (wparam & MK_MBUTTON)
      {
        KillTimer(dialog, 1);
        EndDialog(dialog, MAKELRESULT(VK_MBUTTON, hardware_context::mouse));
      }

      if (wparam & MK_XBUTTON1)
      {
        KillTimer(dialog, 1);
        EndDialog(dialog, MAKELRESULT(VK_XBUTTON1, hardware_context::mouse));
      }

      if (wparam & MK_XBUTTON2)
      {
        KillTimer(dialog, 1);
        EndDialog(dialog, MAKELRESULT(VK_XBUTTON2, hardware_context::mouse));
      }

      return 0;
    }
    case WM_SYSKEYDOWN: {
      KillTimer(dialog, 1);

      if (wparam == VK_MENU)
      {
        if (::GetKeyState(VK_LMENU) & 0x80)
        {
          EndDialog(dialog, MAKELRESULT(VK_LMENU, hardware_context::keyboard));
        }
        else if (::GetKeyState(VK_RMENU) & 0x80)
        {
          EndDialog(dialog, MAKELRESULT(VK_RMENU, hardware_context::keyboard));
        }
      }
      else
      {
        EndDialog(dialog, MAKELRESULT(wparam, hardware_context::keyboard));
      }

      return 0;
    }
    case WM_KEYDOWN: {
      KillTimer(dialog, 1);
      if (wparam == VK_SHIFT)
      {
        if (::GetKeyState(VK_LSHIFT) & 0x80)
        {
          EndDialog(dialog, MAKELRESULT(VK_LSHIFT, hardware_context::keyboard));
        }
        else if (::GetKeyState(VK_RSHIFT) & 0x80)
        {
          EndDialog(dialog, MAKELRESULT(VK_RSHIFT, hardware_context::keyboard));
        }
      }
      else if (wparam == VK_CONTROL)
      {
        if (::GetKeyState(VK_LCONTROL) & 0x80)
        {
          EndDialog(dialog, MAKELRESULT(VK_LCONTROL, hardware_context::keyboard));
        }
        else if (::GetKeyState(VK_RCONTROL) & 0x80)
        {
          EndDialog(dialog, MAKELRESULT(VK_RCONTROL, hardware_context::keyboard));
        }
      }
      else
      {
        EndDialog(dialog, MAKELRESULT(wparam, hardware_context::keyboard));
      }
      return 0;
    }
    default:
      return std::nullopt;
    };
  }

  void exe_view::keyboard_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message)
  {
    if (!has_extension_module(state))
    {
      return;
    }

    if (get_extension(state).game_actions.empty())
    {
      return;
    }

    auto result = win32::DialogBoxIndirectParamW(win32::module_ref::current_application(),
      win32::default_dialog({ .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 200, .cy = 100 }),
      ref(),
      std::bind_front(&exe_view::handle_keyboard_mouse_press, this));

    auto item = keyboard_table.GetItem(LVITEMW{
      .mask = LVIF_PARAM,
      .iItem = message.iItem });

    if (item && item->lParam)
    {
      auto& context = get_action_bindings(state)[item->lParam];

      context.vkey = LOWORD(result);
      context.context = static_cast<decltype(context.context)>(HIWORD(result));

      auto temp = string_for_vkey(result, context.context);
      ListView_SetItemText(keyboard_table, message.iItem, 1, temp.data());
    }
  }

  void exe_view::controller_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message)
  {
    if (has_extension_module(state) && !get_extension(state).controller_input_backends.empty())
    {
      return;
    }

    auto result = win32::DialogBoxIndirectParamW(win32::module_ref::current_application(),
      win32::default_dialog({ .style = DS_CENTER | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU, .cx = 200, .cy = 100 }),
      ref(),
      std::bind_front(&exe_view::handle_keyboard_mouse_press, this));

    auto item = controller_table.GetItem(LVITEMW{
      .mask = LVIF_PARAM,
      .iItem = message.iItem });

    if (item && item->lParam)
    {
      auto& binding = bound_inputs.at(item->lParam);

      auto context = static_cast<siege::platform::hardware_context>(HIWORD(result));
      auto vkey = LOWORD(result);
      std::wstring temp = category_for_vkey(vkey, context);
      ListView_SetItemText(controller_table, message.iItem, 1, temp.data());

      temp = string_for_vkey(vkey, context);
      ListView_SetItemText(controller_table, message.iItem, 2, temp.data());
      binding.to_context = context;
      binding.to_vkey = vkey;
    }
  }

}// namespace siege::views