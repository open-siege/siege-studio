#include <siege/extension/detours.hpp>
import std;
import win32.file;
import win32.window;

import siege.configuration.id_tech;
import siege.configuration.shared;
import siege.extension.id_tech;
import siege.extension.id_tech.v3_0;
import siege.extension.shared;
import siege.platform;
import siege.platform.shared;

extern "C" {
using hardware_context = siege::platform::hardware_context;
using game_action = siege::platform::game_action;
using keyboard_binding = siege::platform::keyboard_binding;
using mouse_binding = siege::platform::mouse_binding;
using controller_binding = siege::platform::controller_binding;
using vkey = siege::platform::vkey;
using game_action = siege::platform::game_action;
using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;

extern auto command_line_caps = game_command_line_caps{
  .int_settings = { { L"r_customwidth", L"r_customheight", L"r_mode", L"net_noipx", L"in_joystick" } },
  .string_settings = { { L"name", L"connect", L"map", L"r_glDriver" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  // not actually the setting because the game starts a listen server by default
  // by this forces the UI to have the "listen" option.
  // TODO look into a better way of representing this
  .listen_setting = L"net_noipx",
  .render_backend_setting = L"r_glDriver",
  .controller_enabled_setting = L"in_joystick",
  .selected_map_setting = L"map",
  .predefined_resolution_setting = L"r_mode",
  .custom_width_setting = L"r_customwidth",
  .custom_height_setting = L"r_customheight"
};

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+back", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+moveleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+moveright", u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, "+moveup", u"Jump", u"Movement" },
  game_action{ game_action::analog, "+movedown", u"Crouch", u"Movement" },
  game_action{ game_action::digital, "+speed", u"Run", u"Movement" },
  game_action{ game_action::analog, "+left", u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, "+right", u"Turn Right", u"Aiming" },
  game_action{ game_action::analog, "+lookup", u"Look Up", u"Aiming" },
  game_action{ game_action::analog, "+lookdown", u"Look Down", u"Aiming" },
  game_action{ game_action::digital, "+attackleft", u"Attack Left", u"Combat" },
  game_action{ game_action::digital, "+attackright", u"Attack Right", u"Combat" },
  game_action{ game_action::digital, "nextinv", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "previnv", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "holster", u"Holster", u"Combat" },
  game_action{ game_action::digital, "invnext", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "toggleitem", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "+scores", u"Score", u"Interface" },
  game_action{ game_action::digital, "togglemenu", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "inventory", u"Inventory", u"Interface" },
  game_action{ game_action::digital, "klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

extern auto network_backends = std::array<const wchar_t*, 2>{ { L"wsock32" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 1>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 1>{ {
  { "Software\\Ritual\\FAKK2"sv, std::size_t(0x4f011c) },
} } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 4> function_name_ranges{ { { "ctrlbindlist"sv, "unbind"sv },
  { "cl_dumpallclasses"sv, "cl_eventlist"sv },
  { "-cameralook"sv, "+moveup"sv },
  { "ui_checkrestart"sv, "pushmenu"sv } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 1> variable_name_ranges{ { { "in_mouse"sv, "in_midi"sv } } };

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
}

std::errc apply_prelaunch_settings(const wchar_t* exe_path_str, siege::platform::game_command_line_args* args)
{
  if (exe_path_str == nullptr)
  {
    return std::errc::bad_address;
  }

  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  auto config = load_config_from_file(L"fakk\\config.cfg")
                  .or_else([]() { return load_config_from_pk3(L"fakk\\default.cfg", L"fakk/pak0.pk3", L"fakk/pak0.pk3"); })
                  .value_or(siege::configuration::text_game_config(siege::configuration::id_tech::id_tech_2::save_config));
  unbind_joystick_for_id_tech_3_0_config(config);

  save_bindings_to_config(*args, config, q3_mapping_context{});

  std::ofstream custom_bindings("fakk/config.cfg", std::ios::binary | std::ios::trunc);
  config.save(custom_bindings);
  insert_string_setting_once(*args, L"console", L"1");

  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (auto config = load_config_from_pk3(L"fakk\\default.cfg", L"fakk/pak0.pk3", L"fakk/pak0.pk3"))
  {
    upsert_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<vkey, std::string_view>, 4> axes{
    {
      { vkey::up, "+lookup"},
      { vkey::down, "+lookdown"},
      { vkey::left, "+left"},
      { vkey::right, "+right"},
    }
  };

  upsert_mouse_axis_defaults(game_actions, axes, *binding);

  return std::errc{};
}

std::errc init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (auto config = load_config_from_pk3(L"fakk\\default.cfg", L"fakk/pak0.pk3", L"fakk/pak0.pk3"))
  {
    upsert_keyboard_bindings(*config, *binding);
  }

  return std::errc{};
}

std::errc default_controller_inputs(controller_binding* binding, std::uint32_t layout_index)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  if (layout_index > 0)
  {
    return std::errc::invalid_argument;
  }

  std::array<std::pair<vkey, std::string_view>, 23> actions{
    {
      { vkey::gamepad_right_trigger, "+attackright"},
      { vkey::gamepad_left_trigger, "+attackleft"},
      { vkey::gamepad_a, "+moveup"},
      { vkey::gamepad_b, "+movedown"},
      { vkey::gamepad_left_thumbstick_button, "+speed"},
      { vkey::gamepad_left_thumbstick_up, "+forward"},
      { vkey::gamepad_left_thumbstick_down, "+back"},
      { vkey::gamepad_left_thumbstick_left, "+moveleft"},
      { vkey::gamepad_left_thumbstick_right, "+moveright"},
      { vkey::gamepad_right_thumbstick_left, "+left"},
      { vkey::gamepad_right_thumbstick_right, "+right"},
      { vkey::gamepad_right_thumbstick_up, "+lookup"},
      { vkey::gamepad_right_thumbstick_down, "+lookdown"},
      { vkey::gamepad_x, "+use"},
      { vkey::gamepad_y, "nextinv"},
      { vkey::gamepad_right_shoulder, "holster"},
      { vkey::gamepad_dpad_down, "holster"},
      { vkey::gamepad_dpad_left, "previnv"},
      { vkey::gamepad_dpad_right, "nextinv"},
      { vkey::gamepad_view, "inventory"},
      { vkey::gamepad_menu, "togglemenu"},
    }
  };

  append_controller_defaults(game_actions, actions, *binding);

  return std::errc{};
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
    static auto modes = std::array<predefined_int, 9>{
      predefined_int{ .label = L"640x480", .value = 1 },
      predefined_int{ .label = L"800x600", .value = 1 },
      predefined_int{ .label = L"960x720", .value = 1 },
      predefined_int{ .label = L"1024x768", .value = 1 },
      predefined_int{ .label = L"1152x864", .value = 1 },
      predefined_int{ .label = L"1280x960", .value = 1 },
      predefined_int{ .label = L"1600x1200", .value = 1 },
      predefined_int{ .label = L"Custom", .value = -1 },
      predefined_int{},
    };

    return modes.data();
  }

  return nullptr;
}

predefined_string*
  get_predefined_string_command_line_settings(const wchar_t* name) noexcept
{
  if (name && std::wstring_view(name) == L"map")
  {
    return get_predefined_id_tech_3_map_command_line_settings(L"fakk");
  }

  return nullptr;
}

static auto* TrueRegOpenKeyA = RegOpenKeyA;
static auto* TrueRegQueryValueExA = RegQueryValueExA;
static auto* TrueGetDriveTypeA = GetDriveTypeA;

static ATOM fake_key = 0;

LSTATUS __stdcall WrappedRegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult)
{
  if (lpSubKey && std::string_view(lpSubKey) == "Software\\Ritual\\FAKK2" && phkResult)
  {
    if (!fake_key)
    {
      fake_key = ::AddAtomA("Software\\Ritual\\FAKK2");
    }

    *phkResult = (HKEY)fake_key;

    return 0;
  }

  return TrueRegOpenKeyA(hKey, lpSubKey, phkResult);
}

LSTATUS __stdcall WrappedRegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
  if (hKey == (HKEY)fake_key)
  {
    if (lpValueName && std::string_view(lpValueName) == "PATH" && lpData && lpcbData)
    {
      namespace fs = std::filesystem;
      auto exe_path = fs::path(win32::module_ref::current_application().GetModuleFileName()).parent_path().string();
      auto size = exe_path.size() < *lpcbData ? exe_path.size() : (std::size_t)*lpcbData;
      std::memcpy(lpData, exe_path.c_str(), size);
      return ERROR_SUCCESS;
    }

    return ERROR_FILE_NOT_FOUND;
  }

  return TrueRegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

UINT WINAPI WrappedGetDriveTypeA(LPCSTR lpRootPathName)
{
  namespace fs = std::filesystem;
  auto exe_path = fs::path(win32::module_ref::current_application().GetModuleFileName()).parent_path().string();
  if (lpRootPathName && !exe_path.empty() && lpRootPathName[0] == exe_path[0])
  {
    return DRIVE_CDROM;
  }

  return TrueGetDriveTypeA(lpRootPathName);
}

static std::array<std::pair<void**, void*>, 3> detour_functions{ {
  { &(void*&)TrueRegOpenKeyA, WrappedRegOpenKeyA },
  { &(void*&)TrueRegQueryValueExA, WrappedRegQueryValueExA },
  { &(void*&)TrueGetDriveTypeA, WrappedGetDriveTypeA },
} };

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,
  DWORD fdwReason,
  LPVOID lpvReserved) noexcept
{
  if constexpr (sizeof(void*) != sizeof(std::uint32_t))
  {
    return TRUE;
  }

  if (::DetourIsHelperProcess())
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
      try
      {
        ::DetourRestoreAfterWith();
        ::DetourTransactionBegin();
        ::DetourUpdateThread(::GetCurrentThread());

        std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { ::DetourAttach(func.first, func.second); });

        ::DetourTransactionCommit();
      }
      catch (...)
      {
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
      if (fake_key)
      {
        ::DeleteAtom(fake_key);
        fake_key = 0;
      }
    }
  }

  return TRUE;
}
std::errc init_game_settings(siege::platform::common_game_settings* args)
{
  return init_id_tech_3_0_game_settings(args, {
    .packed_default_cfg = FSL"fakk\\default.cfg",
    .packed_default_cfg_pak = FSL"fakk/pak0.pk3",
    .packed_default_cfg_pak_folder = FSL"fakk/pak0.pk3",
    .loose_user_cfg = FSL"fakk\\config.cfg",
  });
}
}
