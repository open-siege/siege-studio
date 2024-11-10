#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <siege/platform/win/core/file.hpp>
#include <siege/platform/win/core/com/base.hpp>
#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/window_impl.hpp>
#include <detours.h>
#include "shared.hpp"
#include "GameExport.hpp"
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"


extern "C" {
using game_action = siege::platform::game_action;
using controller_binding = siege::platform::controller_binding;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
};

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "back", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "moveleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "moveright", u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, "moveup", u"Jump", u"Movement" },
  game_action{ game_action::analog, "movedown", u"Crouch", u"Movement" },
  game_action{ game_action::digital, "speed", u"Run", u"Movement" },
  game_action{ game_action::analog, "left", u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, "right", u"Turn Right", u"Aiming" },
  game_action{ game_action::analog, "lookup", u"Look Up", u"Aiming" },
  game_action{ game_action::analog, "lookdown", u"Look Down", u"Aiming" },
  game_action{ game_action::digital, "attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "altattack", u"Alt Attack", u"Combat" },
  game_action{ game_action::digital, "melee-attack", u"Melee Attack", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "itemnext", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "itemuse", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "score", u"Score", u"Interface" },
  game_action{ game_action::digital, "menu-objectives", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };
extern auto keyboard_input_backends = std::array<const wchar_t*, 2>{ { L"user32" } };
extern auto mouse_input_backends = std::array<const wchar_t*, 2>{ { L"user32" } };
extern auto configuration_extensions = std::array<const wchar_t*, 2>{ { L".cfg" } };
extern auto template_configuration_paths = std::array<const wchar_t*, 3>{ { L"base/default.cfg", L"2015/default.cfg" } };
extern auto autoexec_configuration_paths = std::array<const wchar_t*, 2>{ { L"base/autoexec.cfg", L"2015/autoexec.cfg" } };
extern auto profile_configuration_paths = std::array<const wchar_t*, 3>{ { L"base/players/*/config.cfg", L"2015/players/*/config.cfg" } };

HRESULT bind_virtual_key_to_action_for_file(const siege::fs_char* filename, controller_binding* inputs, std::size_t inputs_size)
{
  return S_FALSE;
}

HRESULT bind_virtual_key_to_action_for_process(DWORD process_id, controller_binding* inputs, std::size_t inputs_size)
{
  return S_FALSE;
}

extern void(__fastcall* ConsoleEvalFastcall)(const char*);

using namespace std::literals;

constexpr static auto game_export = siege::game_export<>{
  .preferred_base_address = 0x400000,
  .module_size = 1024 * 1024 * 2,
  .verification_strings = std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x540b6c) },
    { "cmdlist"sv, std::size_t(0x540b64) },
    { "cl_pitchspeed"sv, std::size_t(0x539900) } } },
  .function_name_ranges = std::array<std::pair<std::string_view, std::string_view>, 4>{ { { "centerview"sv, "-klook"sv },
    { "+mlook"sv, "-mlook"sv },
    { "cmdlist"sv, "toggle"sv },
    { "print"sv, "echo"sv } } },
  .variable_name_ranges = std::array<std::pair<std::string_view, std::string_view>, 1>{ { { "in_initmouse"sv, "in_joystick"sv } } },
  .console_eval = 0x4774b0
};

HRESULT get_function_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(game_export.function_name_ranges, length, data, saved);
}

HRESULT get_variable_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(game_export.variable_name_ranges, length, data, saved);
}

HRESULT executable_is_supported(_In_ const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, game_export.verification_strings, game_export.function_name_ranges, game_export.variable_name_ranges);
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
  if constexpr (sizeof(void*) != sizeof(std::uint32_t))
  {
    return TRUE;
  }

  if (DetourIsHelperProcess())
  {
    return TRUE;
  }

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

    auto value = app_module.GetProcAddress<std::uint32_t*>("DisableSiegeExtensionModule");

    if (value && *value == -1)
    {
      return TRUE;
    }
  }

  if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_PROCESS_DETACH)
  {
    thread_local HHOOK hook;
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      int index = 0;
      try
      {
        auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

        MODULEINFO app_module_info{};

        auto exports = game_export;

        if (GetModuleInformation(::GetCurrentProcess(), app_module, &app_module_info, sizeof(MODULEINFO)))
        {
          for (auto& item : exports.verification_strings)
          {
            item.second = item.second - exports.preferred_base_address + (std::size_t)app_module_info.lpBaseOfDll;
          }

          exports.console_eval = exports.console_eval - exports.preferred_base_address + (std::size_t)app_module_info.lpBaseOfDll;
          exports.preferred_base_address = (std::size_t)app_module_info.lpBaseOfDll;
          exports.module_size = app_module_info.SizeOfImage;
        }

        bool module_is_valid = false;

        win32::module_ref temp((void*)exports.verification_strings[0].second);

        if (temp != app_module)
        {
          return FALSE;
        }

        module_is_valid = std::all_of(exports.verification_strings.begin(), exports.verification_strings.end(), [](const auto& str) {
          return std::memcmp(str.first.data(), (void*)str.second, str.first.size()) == 0;
        });

        if (!module_is_valid)
        {
          return FALSE;
        }

        ConsoleEvalFastcall = (decltype(ConsoleEvalFastcall))exports.console_eval;
        DetourRestoreAfterWith();

        auto self = win32::window_module_ref(hinstDLL);
        hook = ::SetWindowsHookExW(WH_GETMESSAGE, dispatch_input_to_fastcall_quake_2_console, self, ::GetCurrentThreadId());
      }
      catch (...)
      {
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      UnhookWindowsHookEx(hook);
    }
  }

  return TRUE;
}
}
