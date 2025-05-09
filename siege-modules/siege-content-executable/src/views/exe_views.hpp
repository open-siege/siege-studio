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

namespace siege::views
{
  struct menu_item : ::MENUITEMINFOW
  {
    menu_item() : ::MENUITEMINFOW()
    {
    }

    menu_item(std::wstring text) : ::MENUITEMINFOW(), text(std::move(text))
    {
      this->dwTypeData = text.data();
    }

    std::wstring text;
    std::vector<menu_item> sub_items;
  };

  struct menu_info : ::MENUINFO
  {
    std::vector<menu_item> menu_items;
  };

  class exe_controller
  {
  public:
    constexpr static auto exe_formats = std::array<siege::fs_string_view, 2>{ { FSL ".exe", FSL ".com" } };
    constexpr static auto lib_formats = std::array<siege::fs_string_view, 6>{ {
      FSL ".dll",
      FSL ".ocx",
      FSL ".olb",
      FSL ".lib",
      FSL ".asi",
      FSL ".ovl",
    } };

    static bool is_exe(std::istream& image_stream);

    std::map<std::wstring, std::set<std::wstring>> get_resource_names() const;

    std::set<std::string_view> get_function_names() const;

    std::set<std::string_view> get_variable_names() const;

    bool set_game_settings(const siege::platform::persistent_game_settings&);

    const siege::platform::persistent_game_settings& get_game_settings();

    std::optional<menu_info> get_resource_menu_items(std::wstring type, std::wstring name) const;

    std::vector<std::wstring> get_resource_strings(std::wstring type, std::wstring name) const;

    std::vector<std::byte> get_resource_data(std::wstring type, std::wstring name, bool raw = false) const;
    std::optional<std::wstring> get_extension_for_name(std::wstring type, std::wstring name) const;

    std::size_t load_executable(std::istream& image_stream, std::optional<std::filesystem::path>) noexcept;

    inline std::filesystem::path get_exe_path() { return loaded_path; }

    inline bool has_extension_module() { return matching_extension != extensions.end(); }
    bool has_zero_tier_extension();

    inline siege::platform::game_extension_module& get_extension() { return *matching_extension; }

    HRESULT launch_game_with_extension(const siege::platform::game_command_line_args* game_args, PROCESS_INFORMATION* process_info) noexcept;

  private:
    std::list<siege::platform::game_extension_module> extensions;
    std::filesystem::path loaded_path;
    win32::module loaded_module;
    std::list<siege::platform::game_extension_module>::iterator matching_extension;
    siege::platform::persistent_game_settings game_settings{};
  };

  using namespace std::literals;

  std::wstring string_for_vkey(SHORT vkey, siege::platform::hardware_context context);
  std::wstring category_for_vkey(SHORT vkey, siege::platform::hardware_context context);
  std::pair<siege::platform::hardware_context, std::uint16_t> hardware_index_for_controller_vkey(std::span<RAWINPUTDEVICELIST> devices, std::uint32_t index, SHORT vkey);

  struct game_setting
  {
    std::wstring setting_name;
    siege::platform::game_command_line_caps::type type;
    std::variant<std::wstring, int, float, bool> value;
    std::wstring display_name;
    int group_id = 0;
    std::function<std::span<siege::platform::predefined_string>(std::wstring_view name)> get_predefined_string;
    std::function<std::span<siege::platform::predefined_int>(std::wstring_view name)> get_predefined_int;
  };

  struct exe_view final : win32::window_ref
  {
    exe_controller controller;

    win32::list_box options;
    std::function<void()> options_unbind;
    win32::list_view resource_table;
    win32::popup_menu extract_menu;
    std::set<std::pair<std::wstring, std::wstring>> selected_resource_items;

    win32::list_view string_table;

    std::optional<siege::platform::game_command_line_caps::type> dedicated_setting_type{};
    std::optional<siege::platform::game_command_line_caps::type> listen_setting_type{};
    std::vector<game_setting> launch_settings = { {} };
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

    void populate_launch_table(const siege::platform::game_command_line_caps& caps);
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