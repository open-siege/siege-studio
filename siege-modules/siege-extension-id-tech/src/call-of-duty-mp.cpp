#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <detours.h>
#include "shared.hpp"
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"


extern "C" {
using game_action = siege::platform::game_action;
using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .int_settings = { { L"r_customwidth", L"r_customheight", L"r_mode" } },
  .string_settings = { { L"name", L"connect", L"map" } },
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
extern auto template_configuration_paths = std::array<const wchar_t*, 3>{ { L"Main/pak0.pak/default.cfg", L"Main/default.cfg" } };
extern auto autoexec_configuration_paths = std::array<const wchar_t*, 4>{ { L"Main/autoexec.cfg" } };
extern auto profile_configuration_paths = std::array<const wchar_t*, 4>{ { L"Main/config.cfg" } };

using namespace std::literals;

constexpr static std::array<std::pair<std::string_view, std::string_view>, 0> function_name_ranges{};
constexpr static std::array<std::pair<std::string_view, std::string_view>, 0> variable_name_ranges{};


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
  if (filename == nullptr)
  {
    return E_POINTER;
  }

  std::error_code last_error;

  if (!std::filesystem::exists(filename, last_error))
  {
    return E_INVALIDARG;
  }

  auto exe_path = std::filesystem::path(filename);
  auto parent_path = exe_path.parent_path();

  if (exe_path.stem() == "CoDMP" && 
      exe_path.extension() == ".exe" &&
      std::filesystem::exists(parent_path / "gamex86.dll", last_error) &&
      std::filesystem::exists(parent_path / "cgamex86.dll", last_error) &&
      std::filesystem::exists(parent_path / "uix86.dll", last_error) &&
      std::filesystem::is_directory(parent_path / "main", last_error))
  {
    return S_OK;
  }

  return S_FALSE;
}

predefined_int*
  get_predefined_int_command_line_settings(const wchar_t* name) noexcept
{
  if (name == nullptr)
  {
    return nullptr;
  }

  auto name_str = std::wstring_view(name);


  if (name_str == L"r_mode")
  {
    static auto modes = std::array<predefined_int, 8>{
      predefined_int{ .label = L"640x480", .value = 1 },
      predefined_int{ .label = L"800x600", .value = 1 },
      predefined_int{ .label = L"960x720", .value = 1 },
      predefined_int{ .label = L"1024x768", .value = 1 },
      predefined_int{ .label = L"1152x864", .value = 1 },
      predefined_int{ .label = L"1280x960", .value = 1 },
      predefined_int{ .label = L"1600x1200", .value = 1 },
      predefined_int{},
    };

    return modes.data();
  }

  return nullptr;
}

predefined_string*
  get_predefined_id_tech_3_map_command_line_settings(const wchar_t* base_dir) noexcept;

predefined_string*
  get_predefined_string_command_line_settings(const wchar_t* name) noexcept
{
  if (name && std::wstring_view(name) == L"map")
  {
    return get_predefined_id_tech_3_map_command_line_settings(L"main");
  }

  return nullptr;
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
      DetourRestoreAfterWith();
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
    }
  }

  return TRUE;
}
}
