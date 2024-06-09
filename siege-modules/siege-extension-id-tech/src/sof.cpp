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
#include "IdTechScriptDispatch.hpp"
#include "MessageHandler.hpp"

extern "C" {
static void(__cdecl* ConsoleEval)(const char*) = nullptr;

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x20120494) },
  { "cmdlist"sv, std::size_t(0x2012049c) },
  { "cl_minfps"sv, std::size_t(0x2011e600) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 3> function_name_ranges{ { { "-klook"sv, "centerview"sv },
  { "joy_advancedupdate"sv, "+mlook"sv },
  { "echo"sv, "echo"sv } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 1> variable_name_ranges{ { { "joy_yawsensitivity"sv, "in_mouse"sv } } };

inline void set_gog_exports()
{
  ConsoleEval = (decltype(ConsoleEval))0x200194f0;
}

constexpr std::array<void (*)(), 5> export_functions = { {
  set_gog_exports,
} };

HRESULT __stdcall ExecutableIsSupported(_In_ const wchar_t* filename) noexcept
{
  return siege::ExecutableIsSupported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
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


            for (auto& pair : function_name_ranges)
            {
              auto first_index = string_section.find(pair.first.data(), 0, pair.first.size() + 1);

              if (first_index != std::string_view::npos)
              {
                auto second_index = string_section.find(pair.second.data(), first_index, pair.second.size() + 1);

                if (second_index != std::string_view::npos)
                {
                  auto second_ptr = string_section.data() + second_index;
                  auto end = second_ptr + std::strlen(second_ptr) + 1;

                  for (auto start = string_section.data() + first_index; start != end; start += std::strlen(start) + 1)
                  {
                    std::string_view temp(start);

                    if (temp.size() == 1)
                    {
                      continue;
                    }

                    if (!std::all_of(temp.begin(), temp.end(), [](auto c) { return c == '+' || c == '-' || std::isalnum(c) != 0; }))
                    {
                      break;
                    }

                    functions.emplace(temp);
                  }
                }
              }
            }

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
              .lpszName = L"siege::extension::soldierOfFortune::ScriptHost",
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
