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
#include "IdTechScriptDispatch.hpp"
#include "MessageHandler.hpp"

extern "C" {
using game_action = siege::game_action;
using controller_binding = siege::controller_binding;

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "forward", L"Move Forward", L"Movement" },
  game_action{ game_action::analog, "back", L"Move Backward", L"Movement" },
  game_action{ game_action::analog, "moveleft", L"Strafe Left", L"Movement" },
  game_action{ game_action::analog, "moveright", L"Strafe Right", L"Movement" },
  game_action{ game_action::analog, "left", L"Turn Left", L"Movement" },
  game_action{ game_action::analog, "right", L"Turn Right", L"Movement" },
  game_action{ game_action::analog, "moveup", L"Jump", L"Movement" },
  game_action{ game_action::analog, "movedown", L"Crouch", L"Movement" },
  game_action{ game_action::digital, "weapnext", L"Next Weapon", L"Combat" },
  game_action{ game_action::digital, "weaprev", L"Previous Weapon", L"Combat" },
  game_action{ game_action::digital, "attack", L"Crouch", L"Combat" },
  game_action{ game_action::digital, "altattack", L"Crouch", L"Combat" },
  game_action{ game_action::digital, "itemnext", L"Crouch", L"Combat" },
  game_action{ game_action::digital, "itemuse", L"Crouch", L"Combat" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };
extern auto keyboard_input_backends = std::array<const wchar_t*, 2>{ { L"user32" } };
extern auto mouse_input_backends = std::array<const wchar_t*, 2>{ { L"user32" } };
extern auto configuration_extensions = std::array<const wchar_t*, 2>{ { L".cfg" } };
extern auto template_configuration_paths = std::array<const wchar_t*, 3>{ { L"base/pak0.pak/default.cfg", L"base/default.cfg" } };
extern auto autoexec_configuration_paths = std::array<const wchar_t*, 2>{ { L"base/autoexec.cfg" } };
extern auto profile_configuration_paths = std::array<const wchar_t*, 3>{ { L"base/profiles/current.cfg", L"base/profiles/matthew.cfg" } };

HRESULT bind_virtual_key_to_action_for_file(const char* filename, controller_binding* inputs, std::size_t inputs_size)
{
  return S_FALSE;
}

HRESULT bind_virtual_key_to_action_for_process(DWORD process_id, controller_binding* inputs, std::size_t inputs_size)
{
  return S_FALSE;
}

HRESULT update_action_intensity_for_process(DWORD process_id, const char* action, float intensity)
{
  return S_FALSE;
}

static void(__cdecl* ConsoleEval)(const char*) = nullptr;

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x20120494) },
  { "cmdlist"sv, std::size_t(0x4ac5a8) },
  { "cl_minfps"sv, std::size_t(0x4a928c) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 3> function_name_ranges{ { { "scoreboard"sv, "+forward"sv },
  { "joy_advancedupdate"sv, "-mlook"sv },
  { "echo"sv, "echo"sv } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 1> variable_name_ranges{{ 
    { "in_initmouse"sv, "in_mouse"sv } 
  }};

inline void set_gog_exports()
{
  ConsoleEval = (decltype(ConsoleEval))0x4599d0;
}

constexpr std::array<void (*)(), 5> export_functions = { {
  set_gog_exports,
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

            std::string_view string_section((const char*)ConsoleEval, 1024 * 1024 * 2);

            functions = siege::extension::GetGameFunctionNames(string_section, function_name_ranges);

            break;
          }
          index++;
        }

        if (!module_is_valid)
        {
          return FALSE;
        }

        DetourRestoreAfterWith();

        auto self = win32::window_module_ref(hinstDLL);
        auto atom = self.RegisterClassExW(win32::static_window_meta_class<siege::extension::MessageHandler>{});

        auto type_name = win32::type_name<siege::extension::MessageHandler>();

        auto host = std::make_unique<siege::extension::IdTechScriptDispatch>(std::move(functions), std::move(variables), [](std::string_view eval_string) -> std::string_view {
          ConsoleEval(eval_string.data());

          return "";
        });

        // TODO register multiple script hosts
        if (auto message = self.CreateWindowExW(CREATESTRUCTW{
              .lpCreateParams = host.release(),
              .hwndParent = HWND_MESSAGE,
              .style = WS_CHILD,
              .lpszName = L"siege::extension::Anachronox::ScriptHost",
              .lpszClass = win32::type_name<siege::extension::MessageHandler>().c_str() });
            message)
        {
        }

        thread_local HHOOK hook;
        struct hook_data
        {
          static LRESULT CALLBACK do_hook(int code, WPARAM wparam, LPARAM lparam)
          {
            if (code == HC_ACTION)
            {
              win32::com::init_com(COINIT_APARTMENTTHREADED);
              UnhookWindowsHookEx(hook);
            }

            return CallNextHookEx(nullptr, code, wparam, lparam);
          }
        };

        hook = ::SetWindowsHookExW(WH_GETMESSAGE, hook_data::do_hook, self, ::GetCurrentThreadId());
      }
      catch (...)
      {
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      auto window = ::FindWindowExW(HWND_MESSAGE, nullptr, win32::type_name<siege::extension::MessageHandler>().c_str(), L"siege::extension::soldierOfFortune::ScriptHost");
      ::DestroyWindow(window);
      auto self = win32::window_module(hinstDLL);

      self.UnregisterClassW<siege::extension::MessageHandler>();
    }
  }

  return TRUE;
}
}
