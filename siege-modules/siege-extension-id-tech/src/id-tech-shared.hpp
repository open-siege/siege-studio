#ifndef ID_TECH_SHARED_HPP
#define ID_TECH_SHARED_HPP

#include <siege/platform/win/window_module.hpp>
#include <siege/configuration/id_tech.hpp>
#include <siege/platform/extension_module.hpp>
#include <filesystem>

std::optional<siege::configuration::text_game_config> load_config_from_pak(std::filesystem::path real_file_path, std::wstring pak_path, std::wstring pak_folder_path);

using hardware_context = siege::platform::hardware_context;

std::optional<std::pair<WORD, hardware_context>> key_to_vkey(std::string_view value);
std::optional<std::string> vkey_to_key(WORD vkey, hardware_context context);
std::optional<std::string_view> hardware_index_to_joystick_setting(WORD vkey, WORD index);

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