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
#include <string>
#include "input_injector.hpp"
#include "exe_shared.hpp"

namespace siege::views
{
  using namespace std::literals;

  std::wstring string_for_vkey(SHORT vkey, siege::platform::hardware_context context);
  std::wstring category_for_vkey(SHORT vkey, siege::platform::hardware_context context);
  std::pair<siege::platform::hardware_context, std::uint16_t> hardware_index_for_controller_vkey(std::span<RAWINPUTDEVICELIST> devices, std::uint32_t index, SHORT vkey);
  std::pair<siege::platform::hardware_context, std::uint16_t> hardware_index_for_xbox_vkey(SHORT vkey);

  struct exe_view final : win32::window_ref
  {
    std::any state;
    std::optional<input_injector> injector;
    win32::list_box options;
    std::function<void()> options_unbind;
    win32::list_view resource_table;
    win32::popup_menu extract_menu;
    std::set<std::pair<std::wstring, std::wstring>> selected_resource_items;

    win32::list_view string_table;

    win32::list_view launch_table;
    win32::edit launch_table_edit;
    win32::combo_box_ex launch_table_combo;
    win32::ip_address_edit launch_table_ip_address;
    std::function<void()> launch_table_edit_unbind;

    struct input_action_binding
    {
      std::uint16_t vkey;
      siege::platform::hardware_context context;
      std::uint16_t action_index;
    };

    std::vector<input_action_binding> bound_actions = { {} };

    struct controller_send_input_binding
    {
      std::uint16_t from_vkey;
      siege::platform::hardware_context from_context;
      std::uint16_t to_vkey;
      siege::platform::hardware_context to_context;
    };

    std::vector<controller_send_input_binding> bound_inputs{ {} };

    win32::list_view keyboard_table;
    win32::list_view controller_table;
    win32::image_list controller_table_icons;
    win32::tool_bar exe_actions;
    win32::image_list exe_actions_icons;

    std::optional<bool> has_saved = std::nullopt;

    constexpr static int add_to_firewall_selected_id = 10;
    constexpr static int launch_selected_id = 11;
    constexpr static int extract_selected_id = 12;

    std::map<std::wstring_view, std::wstring_view> group_names = {
      { L"#1"sv, L"Cursor"sv },
      { L"#2"sv, L"Bitmap"sv },
      { L"#3"sv, L"Icon"sv },
      { L"#4"sv, L"Menu"sv },
      { L"#5"sv, L"Dialog"sv },
      { L"#6"sv, L"String Table"sv },
      { L"#8"sv, L"Font"sv },
      { L"#9"sv, L"Accelerator"sv },
      { L"#10"sv, L"Raw Data"sv },
      { L"#12"sv, L"Cursor Group"sv },
      { L"#14"sv, L"Icon Group"sv },
      { L"#16"sv, L"Version"sv },
      { L"#22"sv, L"Animated Icon"sv },
      { L"#23"sv, L"HTML"sv },
      { L"#24"sv, L"Side-by-Side Assembly Manifest"sv },
    };

    exe_view(win32::hwnd_t self, const CREATESTRUCTW&) : win32::window_ref(self)
    {
    }

    std::optional<win32::lresult_t> wm_create();

    std::optional<win32::lresult_t> wm_size(std::size_t type, SIZE client_size);

    void recreate_image_lists(std::optional<SIZE> possible_size);

    void options_lbn_sel_change(win32::list_box, const NMHDR&);

    void populate_controller_table(std::span<siege::platform::game_action> actions, std::span<const wchar_t*> controller_input_backends);
    void populate_keyboard_table(std::span<siege::platform::game_action> actions, std::span<const wchar_t*> controller_input_backends);

    std::optional<win32::lresult_t> wm_copy_data(win32::copy_data_message<char> message);

    void launch_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message);
    std::optional<LRESULT> handle_keyboard_mouse_press(win32::window_ref dialog, INT message, WPARAM wparam, LPARAM lparam);

    void resource_table_nm_rclick(win32::list_view, const NMITEMACTIVATE& message);
    void resource_table_lvn_item_changed(win32::list_view, const NMLISTVIEW& message);

    void controller_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message);
    void keyboard_table_nm_click(win32::list_view sender, const NMITEMACTIVATE& message);

    void extract_selected_files();

    BOOL exe_actions_nm_click(win32::tool_bar, const NMMOUSE& message);

    std::optional<win32::lresult_t> wm_setting_change(win32::setting_change_message message);
  };
}// namespace siege::views
#endif