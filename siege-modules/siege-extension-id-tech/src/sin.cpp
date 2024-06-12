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
#include "ExecutableIsSupported.hpp"
#include "GameExport.hpp"
#include "GetGameFunctionNames.hpp"
#include "IdTechScriptDispatch.hpp"
#include "MessageHandler.hpp"

extern "C" {
static void(__fastcall* ConsoleEval)(const char*) = nullptr;

using namespace std::literals;

constexpr static auto game_export = siege::GameExport<>{
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


HRESULT __stdcall ExecutableIsSupported(_In_ const wchar_t* filename) noexcept
{
  return siege::ExecutableIsSupported(filename, game_export.verification_strings, game_export.function_name_ranges, game_export.variable_name_ranges);
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

        std::unordered_set<std::string_view> functions;
        std::unordered_set<std::string_view> variables;

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

        ConsoleEval = (decltype(ConsoleEval))exports.console_eval;

        std::string_view string_section((const char*)exports.preferred_base_address, exports.module_size);

        functions = siege::extension::GetGameFunctionNames(string_section, exports.function_name_ranges);

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
              .lpszName = L"siege::extension::Sin::ScriptHost",
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
