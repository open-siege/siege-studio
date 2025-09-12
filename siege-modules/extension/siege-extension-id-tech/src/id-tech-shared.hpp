#ifndef ID_TECH_SHARED_HPP
#define ID_TECH_SHARED_HPP

#include <siege/platform/win/window_module.hpp>
#include <siege/configuration/id_tech.hpp>
#include <siege/platform/extension_module.hpp>
#include <filesystem>
#include <optional>

std::optional<siege::configuration::text_game_config> load_config_from_pak(std::filesystem::path real_file_path, std::wstring pak_path, std::wstring pak_folder_path);
std::optional<siege::configuration::text_game_config> load_config_from_pk3(std::filesystem::path real_file_path, std::wstring pak_path, std::wstring pak_folder_path);

using hardware_context = siege::platform::hardware_context;
using mouse_context = siege::platform::mouse_context;
using keyboard_context = siege::platform::keyboard_context;

std::optional<std::pair<WORD, hardware_context>> key_to_vkey(std::string_view value);
std::optional<std::string> vkey_to_key(WORD vkey, hardware_context context);
std::optional<std::string_view> hardware_index_to_button_name_id_tech_2_0(WORD index);
std::optional<std::string_view> hardware_index_to_button_name_id_tech_3_0(WORD index);
std::optional<std::string_view> hardware_index_to_button_name_id_tech_3_0_from_zero(WORD index);
std::optional<std::string_view> dpad_name_id_tech_2_0(WORD index);
std::optional<std::string_view> dpad_name_id_tech_3_0(WORD index);
std::optional<std::string_view> hardware_index_to_joystick_axis_id_tech_3_0(WORD vkey, WORD index);
std::optional<std::string_view> hardware_index_to_joystick_axis_id_tech_2_5(WORD vkey, WORD index);
std::optional<std::string_view> hardware_index_to_joystick_axis_id_tech_2_0(WORD vkey, WORD index);

void load_mouse_bindings(siege::configuration::text_game_config& config, siege::platform::mouse_binding& binding);
void load_keyboard_bindings(siege::configuration::text_game_config& config, siege::platform::keyboard_binding& binding);
void upsert_mouse_defaults(const std::span<siege::platform::game_action> game_actions, const std::span<std::pair<WORD, std::string_view>> actions, siege::platform::mouse_binding& binding, mouse_context default_context = mouse_context::mouse);
void upsert_keyboard_defaults(const std::span<siege::platform::game_action> game_actions, const std::span<std::pair<WORD, std::string_view>> actions, siege::platform::keyboard_binding& binding, bool ignore_case = false, keyboard_context default_context = keyboard_context::keyboard);
void append_controller_defaults(const std::span<siege::platform::game_action> game_actions, const std::span<std::pair<WORD, std::string_view>> actions, siege::platform::controller_binding& binding);
void insert_string_setting_once(siege::platform::game_command_line_args& args, std::wstring_view name, std::wstring_view value);

struct mapping_context
{
  std::optional<std::string_view> (*index_to_axis)(WORD vkey, WORD index) = hardware_index_to_joystick_axis_id_tech_2_5;
  std::optional<std::string_view> (*index_to_button)(WORD index) = hardware_index_to_button_name_id_tech_2_0;
  std::optional<std::string_view> (*dpad_name)(WORD index) = dpad_name_id_tech_2_0;
  std::string_view axis_set_prefix = "set";
  bool supports_triggers_as_buttons = false;
};

struct q3_mapping_context : mapping_context
{
  q3_mapping_context()
  {
    index_to_axis = hardware_index_to_joystick_axis_id_tech_3_0;
    index_to_button = hardware_index_to_button_name_id_tech_3_0;
    dpad_name = dpad_name_id_tech_3_0;
    axis_set_prefix = "bind";
    supports_triggers_as_buttons = true;
  };
};

struct raven_mapping_context : mapping_context
{
  raven_mapping_context()
  {
    index_to_axis = hardware_index_to_joystick_axis_id_tech_3_0;
    index_to_button = hardware_index_to_button_name_id_tech_3_0_from_zero;
    dpad_name = dpad_name_id_tech_3_0;
    axis_set_prefix = "bind";
    supports_triggers_as_buttons = true;
  };
};


bool save_bindings_to_config(siege::platform::game_command_line_args& args, siege::configuration::text_game_config& config, mapping_context context = {});
void bind_axis_to_send_input(siege::platform::game_command_line_args& args, std::string_view source, std::string_view target, std::optional<WORD> not_target_vkey = std::nullopt);
void bind_controller_send_input_fallback(siege::platform::game_command_line_args& args, hardware_context expected_context, WORD expected_vkey, std::optional<WORD> not_target_vkey = std::nullopt);
void bind_controller_send_input_fallback(siege::platform::game_command_line_args& args, hardware_context expected_context, WORD expected_vkey, std::string_view target_action, std::optional<WORD> not_target_vkey = std::nullopt);

inline bool is_vkey_for_controller(WORD vkey)
{
  return vkey >= VK_GAMEPAD_A && vkey <= VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT;
}

inline bool is_vkey_for_mouse(WORD vkey)
{
  return vkey == VK_LBUTTON || vkey == VK_RBUTTON || vkey == VK_MBUTTON || vkey == VK_XBUTTON1 || vkey == VK_XBUTTON2;
}

inline bool is_vkey_for_keyboard(WORD vkey)
{
  return !is_vkey_for_mouse(vkey) && !is_vkey_for_controller(vkey);
}

extern "C" {
std::errc apply_dpi_awareness(const wchar_t* exe_path_str);
LRESULT CALLBACK dispatch_input_to_cdecl_quake_2_console(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dispatch_input_to_fastcall_quake_2_console(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dispatch_input_to_cdecl_quake_1_console(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dispatch_input_to_cdecl_quake_3_console(int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK dispatch_input_to_stdcall_quake_3_console(int code, WPARAM wParam, LPARAM lParam);
}

namespace siege::extension
{
  static LRESULT CALLBACK DispatchInputToGameConsole(int code, WPARAM wParam, LPARAM lParam)
  {
    if (code == HC_ACTION && wParam == PM_REMOVE)
    {
      auto* message = (MSG*)lParam;

      if (message->message == WM_INPUT)
      {
        // TODO do dispatching on WM_INPUT here
      }
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  static LRESULT CALLBACK DispatchInputToSendInput(int code, WPARAM wParam, LPARAM lParam)
  {
    if (code == HC_ACTION && wParam == PM_REMOVE)
    {
      auto* message = (MSG*)lParam;

      if (message->message == WM_INPUT)
      {
        // TODO do dispatching on WM_INPUT here
      }
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
  }
}// namespace siege::extension

#endif