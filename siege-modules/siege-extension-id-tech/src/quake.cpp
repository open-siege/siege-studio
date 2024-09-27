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
#include <siege/platform/win/desktop/window_module.hpp>
#include <siege/platform/win/desktop/window_impl.hpp>
#include <detours.h>
#include "shared.hpp"
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"


extern "C" {
using game_action = siege::platform::game_action;
using controller_binding = siege::platform::controller_binding;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .supports_ip_connect = true,
  .supports_ip_host = true,
  .supports_custom_mod_folder = true,
  .supports_custom_configurations = false
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
extern auto template_configuration_paths = std::array<const wchar_t*, 3>{ { L"Id1/PAK0.PAK/default.cfg", L"Id1/default.cfg" } };
extern auto autoexec_configuration_paths = std::array<const wchar_t*, 4>{ { L"Id1/autoexec.cfg", L"hipnotic/autoexec.cfg", L"rogue/autoexec.cfg" } };
extern auto profile_configuration_paths = std::array<const wchar_t*, 4>{ { L"Id1/config.cfg", L"hipnotic/config.cfg", L"rogue/config.cfg" } };

HRESULT bind_virtual_key_to_action_for_file(const siege::fs_char* filename, controller_binding* inputs, std::size_t inputs_size)
{
  return S_FALSE;
}

HRESULT bind_virtual_key_to_action_for_process(DWORD process_id, controller_binding* inputs, std::size_t inputs_size)
{
  return S_FALSE;
}

extern void(__cdecl* ConsoleEvalCdecl)(const char*);

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 4> verification_strings = { {
  // win quake
  std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x470e84) },
    { "cmd"sv, std::size_t(0x470e9c) },
    { "cl_pitchspeed"sv, std::size_t(0x475af8) } } },
  // gl quake
  std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x449594) },
    { "cmd"sv, std::size_t(0x449580) },
    { "cl_pitchspeed"sv, std::size_t(0x448704) } } },

  // quake world
  std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x47c6c8) },
    { "cmd"sv, std::size_t(0x47c6ac) },
    { "cl_pitchspeed"sv, std::size_t(0x47a348) } } },

  // gl quake world
  std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x44eae4) },
    { "cmd"sv, std::size_t(0x44eb04) },
    { "cl_pitchspeed"sv, std::size_t(0x44c990) } } },
} };

// the order changes between regular quake and glquake, so each value has to be checked by itself
constexpr static std::array<std::pair<std::string_view, std::string_view>, 6> function_name_ranges{ {
  { "timerefresh"sv, "timerefresh"sv },
  { "pointfile"sv, "pointfile"sv },
  { "joyadvancedupdate"sv, "joyadvancedupdate"sv },
  { "force_centerview"sv, "force_centerview"sv },
  { "stuffcmds"sv, "stuffcmds"sv },
  { "wait"sv, "wait"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 7> variable_name_ranges{{ 
    { "joyadvanced"sv, "joyadvanced"sv },
    { "joyadvaxisx"sv, "joyadvaxisx"sv },
    { "joyadvaxisy"sv, "joyadvaxisy"sv },
    { "joyadvaxisz"sv, "joyadvaxisz"sv },
    { "joyadvaxisr"sv, "joyadvaxisr"sv },
    { "joyadvaxisu"sv, "joyadvaxisu"sv },
    { "joyadvaxisv"sv, "joyadvaxisv"sv },
 }};

inline void set_gog_win_exports()
{
  ConsoleEvalCdecl = (decltype(ConsoleEvalCdecl))0x42bc80;
}

inline void set_gog_gl_exports()
{
  ConsoleEvalCdecl = (decltype(ConsoleEvalCdecl))0x4056c0;
}

inline void set_gog_qw_exports()
{
  ConsoleEvalCdecl = (decltype(ConsoleEvalCdecl))0x40fdd0;
}

inline void set_gog_gl_qw_exports()
{
  ConsoleEvalCdecl = (decltype(ConsoleEvalCdecl))0x40c768;
}

constexpr std::array<void (*)(), 4> export_functions = { {
  set_gog_win_exports,
  set_gog_gl_exports,
  set_gog_qw_exports,
  set_gog_gl_qw_exports,
} };

HRESULT get_function_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(function_name_ranges, length, data, saved);
}

HRESULT get_variable_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(variable_name_ranges, length, data, saved);
}

HRESULT executable_is_supported(_In_ const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
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

        std::unordered_set<std::string_view> functions;
        std::unordered_set<std::string_view> variables;

        bool module_is_valid = false;

        for (const auto& item : verification_strings)
        {
          win32::module_ref temp((void*)item[0].second);

          if (temp != app_module)
          {
            continue;
          }

          module_is_valid = std::all_of(item.begin(), item.end(), [](const auto& str) {
            return std::memcmp(str.first.data(), (void*)str.second, str.first.size()) == 0;
          });


          if (module_is_valid)
          {
            export_functions[index]();

            std::string_view string_section((const char*)ConsoleEvalCdecl, 1024 * 1024 * 2);

            functions = siege::extension::GetGameFunctionNames(string_section, function_name_ranges);

            break;
          }
          index++;
        }

        if (!ConsoleEvalCdecl)
        {
          return FALSE;
        }

        if (!module_is_valid)
        {
          return FALSE;
        }

        DetourRestoreAfterWith();

        auto self = win32::window_module_ref(hinstDLL);
        hook = ::SetWindowsHookExW(WH_GETMESSAGE, dispatch_copy_data_to_cdecl_quake_1_console, self, ::GetCurrentThreadId());
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
