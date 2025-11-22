#ifndef EXE_VIEWS_HPP
#define EXE_VIEWS_HPP

#include <siege/platform/stream.hpp>
#include <siege/platform/win/module.hpp>
#include <siege/platform/extension_module.hpp>
#include <siege/platform/shared.hpp>
#include <siege/platform/win/common_controls.hpp>
#include <siege/platform/win/drawing.hpp>
#include <siege/platform/win/theming.hpp>
#include <siege/platform/win/dialog.hpp>
#include <siege/platform/win/basic_window.hpp>
#include <string>
#include "exe_shared.hpp"

namespace siege::views
{
  using namespace std::literals;

  struct exe_view final : win32::basic_window<exe_view>
  {
    std::any state;
    win32::list_box options;
    win32::tool_bar exe_actions;
    win32::image_list exe_actions_icons;

    struct resource_controls
    {
      win32::list_view resource_table;
      win32::popup_menu extract_menu;
      std::set<std::pair<std::wstring, std::wstring>> selected_resource_items;
      win32::list_view string_table;
      std::optional<bool> has_saved = std::nullopt;
    } resource;

    struct launch_controls
    {
      std::any injector;
      win32::list_view launch_table;
      win32::edit launch_table_edit;
      win32::combo_box_ex launch_table_combo;
      win32::ip_address_edit launch_table_ip_address;
      std::function<void()> launch_table_edit_unbind;
      constexpr static int launch_selected_id = 10;
    } launch;

    struct controller_send_input_binding
    {
      std::uint16_t from_vkey;
      siege::platform::hardware_context from_context;
      std::uint16_t to_vkey;
      siege::platform::hardware_context to_context;
    };

    struct input_controls
    {
      win32::list_view keyboard_table;
      win32::list_view controller_table;
      win32::image_list controller_table_icons;
      std::vector<controller_send_input_binding> bound_inputs{ {} };
      constexpr static int controllers_selected_id = 11;
    } input;

    exe_view(win32::hwnd_t self, CREATESTRUCTW& params) : basic_window(self, params)
    {
    }

    win32::lresult_t wm_create();

    win32::lresult_t wm_size(std::size_t type, SIZE client_size);

    win32::lresult_t wm_copy_data(win32::copy_data_message<char> message);

    void recreate_image_lists(std::optional<SIZE> possible_size);

    decltype(input) create_input_controls();
    decltype(launch) create_launch_controls();

    void populate_controller_table(std::span<siege::platform::game_action> actions, std::span<const wchar_t*> controller_input_backends);
    void populate_keyboard_table(std::span<siege::platform::game_action> actions, std::span<const wchar_t*> controller_input_backends);

    std::optional<LRESULT> window_proc(UINT message, WPARAM wparam, LPARAM lparam) override;

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message);
  };
}// namespace siege::views
#endif