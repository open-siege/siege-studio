#ifndef EXE_SHARED_HPP
#define EXE_SHARED_HPP

#include <functional>
#include <string>
#include <string_view>
#include <optional>
#include <variant>
#include <any>
#include <map>
#include <siege/platform/extension_module.hpp>

namespace siege::views
{
  constexpr auto zt_fallback_ip = L"ZERO_TIER_FALLBACK_BROADCAST_IP_V4";

  struct controller_info
  {
    siege::platform::hardware_context detected_context;
    std::string_view backend;
    std::uint16_t (*get_hardware_index)(SHORT vkey);
  };

  std::vector<controller_info> get_connected_controllers();

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

  siege::platform::game_command_line_args& get_final_args(std::any& state);

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
  std::optional<std::reference_wrapper<game_setting>> get_game_setting(std::any& state, std::size_t index);

  void set_ip_for_current_network(std::any& state, std::string ip_address);

  bool is_exe_or_lib(std::istream& stream);

  HRESULT launch_game_with_extension(std::any& state, const siege::platform::game_command_line_args* game_args, PROCESS_INFORMATION* process_info) noexcept;
}// namespace siege::views

#endif// !EXE_SHARED_HPP
