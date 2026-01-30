#ifndef EXE_SHARED_HPP
#define EXE_SHARED_HPP

#include <functional>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <any>
#include <map>
#include <span>
#include <array>
#include <siege/platform/extension_module.hpp>
#include <xinput.h>

#define DIRECTINPUT_VERSION 0x800
// Only needed for DIJOYSTATE
#include <dinput.h>

namespace siege::views
{
  constexpr auto zt_fallback_ip = L"ZERO_TIER_FALLBACK_BROADCAST_IP_V4";

  // TODO games which use winmm (and possibly also dinput)
  // rely on the first controller being used.
  // This is governed by the primary controller setting in Windows.
  // We should add which controller is considered to be the primary here.
  // Then, if the user's preferred context is not the primary, we can warn them and
  // ask if they want to change it via the system dialog
  struct hardware_index
  {
    siege::platform::controller_input_type type;

    std::uint16_t index;

    auto operator<=> (const hardware_index&) const = default;
  };

  struct winmm_hardware_index : hardware_index
  {
  };

  struct controller_info
  {
    siege::platform::hardware_context detected_context;
    std::string_view backend;
    std::wstring_view device_path;
    std::wstring_view device_name;
    enum button_preference : bool
    {
      prefer_axis,
      prefer_button
    };

    enum index_preference : bool
    {
      prefer_hid,
      prefer_winmm
    };
    std::function<std::optional<hardware_index>(SHORT, button_preference)> get_hardware_index;
    std::pair<std::uint32_t, std::uint32_t> vendor_product_id;
    bool is_system_preferred = false;
  };

  winmm_hardware_index map_hid_to_winmm(hardware_index, const std::set<std::uint32_t>& axis_indexes);

  struct input_state_change
  {
    WORD vkey;
    std::uint16_t new_value;
    std::uint16_t old_value;
  };

  struct controller_state
  {
    controller_info info;
    std::any caps;
    XINPUT_STATE last_state;
    std::array<input_state_change, 64> buffer;
  };

  using input_type = decltype(siege::views::hardware_index::type);

  std::span<input_state_change> get_changes(const XINPUT_STATE& a, const XINPUT_STATE& b, std::span<input_state_change> buffer);
  std::vector<controller_info> get_connected_controllers();
  std::optional<controller_info> controller_info_for_raw_input_device_handle(HANDLE handle);
  std::optional<controller_info> controller_info_for_raw_input_handle(HRAWINPUT handle);

  std::set<std::uint32_t> hardware_buttons(const controller_state&);
  std::set<std::uint32_t> hardware_axes(const controller_state&);
  std::uint32_t hardware_hat_count(const controller_state&);

  std::set<std::uint32_t> mapped_buttons(const controller_info& info);
  std::set<std::uint32_t> mapped_axes(const controller_info& info);
  std::uint32_t mapped_hat_count(const controller_info&);

  controller_info detect_and_store_controller_context_from_hint(const controller_info& info, siege::platform::hardware_context hint);
  controller_info store_controller_context(const controller_info& info, siege::platform::hardware_context context);
  XINPUT_STATE get_current_state_for_handle(controller_state& state, HRAWINPUT handle);
  DIJOYSTATE get_raw_state_for_handle(controller_state& state, HRAWINPUT handle);

  std::wstring label_for_vkey(SHORT vkey, siege::platform::hardware_context context);
  std::wstring category_for_vkey(SHORT vkey, siege::platform::hardware_context context);
  siege::platform::hardware_context string_to_context(std::wstring_view value);

  struct input_action_binding
  {
    std::uint16_t vkey;
    // The context starts off vague and becomes more
    // defined as more hardware information is detected.
    // For controller input especially, we eventually
    // care about the type of context to use for generating configurations.
    // This is because older games rely on the hardware index
    // of buttons/axises but we care about the conceptual button/axies when binding
    // (ie we want the A button to jump).
    // The reason why each input gets a context
    // is because in more complex configurations,
    // you may have two separate input devices to control a game (think of HOTAS configurations).
    // But, most old games are programmed to only handle one input at a time.
    // The logic to handle separate contextes doesn't strictly exist yet,
    // but it would fall on the extension to do something reasonable
    // (ie configure a primary device as the joystick and then the secondary device to simulate keyboard/mouse inputs)
    siege::platform::hardware_context context;
    std::uint16_t action_index;
  };

  std::span<input_action_binding> get_action_bindings(std::any& state);
  std::size_t add_action_binding(std::any& state, input_action_binding binding);

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

  enum struct extension_setting_type
  {
    unknown,
    string_setting,
    flag_setting,
    int_setting,
    float_setting,
    env_setting,
    computed_setting,
  };


  struct game_setting
  {
    using game_command_line_caps = siege::platform::game_command_line_caps;
    std::wstring setting_name;
    extension_setting_type type;
    std::variant<std::wstring, int, float, bool> value;
    std::wstring display_name;
    std::wstring display_value;
    bool visible = true;
    bool enabled = true;
    int group_id = 0;
    std::function<std::span<siege::platform::predefined_string>(std::wstring_view name)> get_predefined_string;
    std::function<std::span<siege::platform::predefined_int>(std::wstring_view name)> get_predefined_int;
    std::function<void()> persist;

    void update_value(int new_value, std::wstring_view new_display_value = L"");
    void update_value(std::wstring_view new_value, std::wstring_view new_display_value = L"");

    std::wstring get_computed_display_value();
  };

  bool has_extension_module(const std::any& state);
  bool can_support_zero_tier(std::any& state);

  siege::platform::game_extension_module& get_extension(std::any& state);
  const siege::platform::game_extension_module& get_extension(const std::any& state);

  std::optional<std::wstring> get_extension_for_name(const std::any& state, std::wstring type, std::wstring name);

  std::vector<std::byte> get_resource_data(const std::any& state, std::wstring type, std::wstring name, bool raw = false);

  std::size_t load_executable(std::any& state, std::istream& image_stream, std::optional<std::filesystem::path>) noexcept;

  std::map<std::wstring, std::set<std::wstring>> get_resource_names(const std::any& state);

  std::optional<menu_info> get_resource_menu_items(const std::any& state, std::wstring type, std::wstring name);

  std::vector<std::wstring> get_resource_strings(const std::any& state, std::wstring type, std::wstring name);

  std::set<std::string_view> get_function_names(const std::any& state);

  std::set<std::string_view> get_variable_names(const std::any& state);

  std::filesystem::path get_exe_path(const std::any& state);

  std::span<const siege::fs_string_view> get_executable_formats() noexcept;
  std::span<const siege::fs_string_view> get_library_formats() noexcept;

  std::span<game_setting> init_launch_settings(std::any& state);
  bool is_vkey_for_controller(WORD vkey);
  std::optional<std::reference_wrapper<game_setting>> get_game_setting(std::any& state, std::size_t index);

  void set_ip_for_current_network(std::any& state, std::string ip_address);

  bool controller_can_be_toggled(std::any& state);

  void disable_controller_for_extension(std::any& state);
  void enable_controller_for_extension(std::any& state);

  bool is_exe_or_lib(std::istream& stream) noexcept;
  siege::platform::owning_packaged_args get_packaged_args(std::any& state);

  HRESULT launch_game_with_extension(std::any& state, siege::platform::packaged_args& game_args, PROCESS_INFORMATION* process_info) noexcept;
}// namespace siege::views

#endif// !EXE_SHARED_HPP
