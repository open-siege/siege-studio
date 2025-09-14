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
#include <siege/extension/shared.hpp>

#include "id-tech-shared.hpp"


extern "C" {
using hardware_context = siege::platform::hardware_context;
using game_action = siege::platform::game_action;
using keyboard_binding = siege::platform::keyboard_binding;
using mouse_binding = siege::platform::mouse_binding;
using controller_binding = siege::platform::controller_binding;
using game_action = siege::platform::game_action;
using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;

extern auto command_line_caps = game_command_line_caps{
  .flags = { { L"listen", L"dedicated" } },
  .int_settings = { { L"width", L"height", L"vid_mode", L"chase_active", L"chase_back", L"chase_up", L"chase_right" } },// GL Hexen only
  .string_settings = { { L"name", L"connect", L"map", L"preferred_exe", L"game" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"listen",
  .dedicated_setting = L"dedicated",
  .selected_game_setting = L"game",
  .preferred_exe_setting = L"preferred_exe",
};

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+back", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+moveleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+moveright", u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, "+jump", u"Jump", u"Movement" },
  game_action{ game_action::analog, "+movedown", u"Crouch", u"Movement" },
  game_action{ game_action::digital, "+speed", u"Run", u"Movement" },
  game_action{ game_action::analog, "+left", u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, "+right", u"Turn Right", u"Aiming" },
  game_action{ game_action::analog, "+lookup", u"Look Up", u"Aiming" },
  game_action{ game_action::analog, "+lookdown", u"Look Down", u"Aiming" },
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "+altattack", u"Alt Attack", u"Combat" },
  game_action{ game_action::digital, "+melee-attack", u"Melee Attack", u"Combat" },
  game_action{ game_action::digital, "+weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "itemnext", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "itemuse", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "score", u"Score", u"Interface" },
  game_action{ game_action::digital, "menu-objectives", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 4>, 2> verification_strings = { {
  // win hexen 2
  std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "HexenII"sv, std::size_t(0x4b0524) },
    { "exec"sv, std::size_t(0x491de4) },
    { "cmd"sv, std::size_t(0x491dfc) },
    { "cl_pitchspeed"sv, std::size_t(0x48ef60) } } },
  // gl hexen 2
  std::array<std::pair<std::string_view, std::size_t>, 4>{ { { "HexenII"sv, std::size_t(0x482a38) },
    { "exec"sv, std::size_t(0x465774) },
    { "cmd"sv, std::size_t(0x46578c) },
    { "cl_pitchspeed"sv, std::size_t(0x466190) } } },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 6> function_name_ranges{ {
  { "midi_volume"sv, "midi_play"sv },
  { "menu_class"sv, "togglemenu"sv },
  { "toggle_dm"sv, "+showinfo"sv },
  { "port"sv, "slist"sv },
  { "sensitivity_save"sv, "entities"sv },
  { "mcache"sv, "playerclass"sv },
} };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 2> variable_name_ranges{ { { "joyadvaxisv"sv, "joyname"sv },
  { "sv_accelerate"sv, "sv_idealpitchscale"sv } } };

std::errc get_function_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(function_name_ranges, length, data, saved);
}

std::errc get_variable_name_ranges(std::size_t length, std::array<const char*, 2>* data, std::size_t* saved) noexcept
{
  return siege::get_name_ranges(variable_name_ranges, length, data, saved);
}

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings[0], function_name_ranges, variable_name_ranges);
}

std::errc apply_prelaunch_settings(const wchar_t* exe_path_str, siege::platform::game_command_line_args* args)
{
  if (auto result = apply_dpi_awareness(exe_path_str); result != std::errc{})
  {
    return result;
  }

  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  std::ofstream custom_bindings("portals/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  bool enable_controller = save_bindings_to_config(*args, config, mapping_context{ .index_to_axis = hardware_index_to_joystick_axis_id_tech_2_0, .axis_set_prefix = "" });

  if (enable_controller)
  {
    // engine bug - mouse needs to be enabled for the right analog stick to work
    config.emplace(siege::configuration::key_type({ "joystick" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joyadvanced" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joysidesensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joypitchsensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joyadvancedupdate" }), siege::configuration::key_type(""));
  }

  config.save(custom_bindings);

  bind_axis_to_send_input(*args, "+lookup", "+mlook");
  bind_axis_to_send_input(*args, "+lookdown", "+mlook");

  auto iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name == nullptr; });

  std::advance(iter, 1);
  iter->name = L"console";
  iter->value = L"1";

  std::advance(iter, 1);
  iter->name = L"exec";
  iter->value = L"siege_studio_inputs.cfg";

  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }
  auto config = load_config_from_pak(L"portals\\default.cfg", L"portals/pak3.pak", L"portals/pak3.pak");

  if (config)
  {
    load_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 1> actions{
    { std::make_pair<WORD, std::string_view>(VK_MBUTTON, "+use") }
  };

  upsert_mouse_defaults(game_actions, actions, *binding);


  return std::errc{};
}

std::errc init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  auto config = load_config_from_pak(L"portals\\default.cfg", L"portals/pak3.pak", L"portals/pak3.pak");

  if (config)
  {
    load_keyboard_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 8> actions{
    {
      std::make_pair<WORD, std::string_view>('w', "+forward"),
      std::make_pair<WORD, std::string_view>('a', "+moveleft"),
      std::make_pair<WORD, std::string_view>('s', "+back"),
      std::make_pair<WORD, std::string_view>('d', "+moveright"),
      std::make_pair<WORD, std::string_view>('f', "+melee-attack"),
      std::make_pair<WORD, std::string_view>(VK_SPACE, "+moveup"),
      std::make_pair<WORD, std::string_view>(VK_LCONTROL, "+movedown"),
      std::make_pair<WORD, std::string_view>(VK_OEM_5, "+mlook"),
    }
  };

  upsert_keyboard_defaults(game_actions, actions, *binding);

  return std::errc{};
}

std::errc init_controller_inputs(controller_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }
  std::array<std::pair<WORD, std::string_view>, 23> actions{
    {
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_TRIGGER, "+attack"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_TRIGGER, "invuse"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_A, "+jump"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_B, "+movedown"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON, "+speed"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_UP, "+forward"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN, "+back"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT, "+moveleft"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT, "+moveright"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT, "+left"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT, "+right"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_UP, "+lookup"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN, "+lookdown"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, "+melee-attack"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_X, "inven"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_SHOULDER, "invnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_SHOULDER, "+throw-grenade"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_DOWN, "weapondrop"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "weapprev"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_VIEW, "score"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_MENU, "cmd help"),
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

  if (std::wstring_view(name) == L"vid_mode")
  {
    static auto modes = std::array<predefined_int, 8>{
      predefined_int{ .label = L"640x480", .value = 8 },
      predefined_int{ .label = L"800x600", .value = 9 },
      predefined_int{ .label = L"1024x768", .value = 10 },
      predefined_int{ .label = L"1280x1024", .value = 11 },
      predefined_int{},
    };

    return modes.data();
  }

  return nullptr;
}

predefined_string*
  get_predefined_id_tech_2_map_command_line_settings(const wchar_t* base_dir, bool include_zip) noexcept;

predefined_string*
  get_predefined_string_command_line_settings(const wchar_t* name) noexcept
{
  if (name && std::wstring_view(name) == L"map")
  {
    return get_predefined_id_tech_2_map_command_line_settings(L"portals", false);
  }

  if (name && std::wstring_view(name) == L"preferred_exe")
  {
    static auto modes = std::array<predefined_string, 3>{
      predefined_string{ .label = L"Hexen 2 - Portal of Praevus (software rendered)", .value = L"H2MP.EXE" },
      predefined_string{ .label = L"GLHexen 2 - Portal of Praevus", .value = L"GLH2MP.EXE" },
      predefined_string{},
    };

    return modes.data();
  }

  return nullptr;
}

static std::string VirtualDriveLetter;
constexpr static std::string_view MissionPackDisc = "H2MP";
static auto* TrueGetLogicalDrives = GetLogicalDrives;
static auto* TrueGetLogicalDriveStringsA = GetLogicalDriveStringsA;
static auto* TrueGetDriveTypeA = GetDriveTypeA;
static auto* TrueGetVolumeInformationA = GetVolumeInformationA;
static auto* TrueCreateFileA = CreateFileA;
static auto* TrueRegQueryValueExA = RegQueryValueExA;

DWORD WINAPI WrappedGetLogicalDrives()
{
  auto result = TrueGetLogicalDrives();
  std::bitset<sizeof(DWORD) * 8> bits(result);

  int driveLetter = static_cast<int>('A');

  for (auto i = 2; i < bits.size(); ++i)
  {
    if (bits[i] == false)
    {
      driveLetter += i;
      bits[i] = true;

      if (VirtualDriveLetter.empty())
      {
        VirtualDriveLetter = static_cast<char>(driveLetter) + std::string(":\\");
      }
      break;
    }
  }


  return bits.to_ulong();
}

UINT WINAPI WrappedGetDriveTypeA(LPCSTR lpRootPathName)
{
  // the game doesn't call GetLogicalDrives, so we should call it ourselves
  static auto drive_types = WrappedGetLogicalDrives();
  if (lpRootPathName && VirtualDriveLetter[0] == lpRootPathName[0])
  {
    return DRIVE_CDROM;
  }

  return TrueGetDriveTypeA(lpRootPathName);
}

DWORD WINAPI WrappedGetLogicalDriveStringsA(DWORD nBufferLength, LPSTR lpBuffer)
{
  // the game doesn't call GetLogicalDrives, so we should call it ourselves
  static auto drive_types = WrappedGetLogicalDrives();

  if (!VirtualDriveLetter.empty())
  {
    std::string drives;
    drives.resize(TrueGetLogicalDriveStringsA(0, drives.data()));
    TrueGetLogicalDriveStringsA((DWORD)drives.size(), drives.data());

    drives.push_back('\0');
    drives.append(VirtualDriveLetter);

    DWORD size = drives.size() < nBufferLength ? (DWORD)drives.size() : nBufferLength;
    std::memcpy(lpBuffer, drives.data(), size);
    return size;
  }

  return TrueGetLogicalDriveStringsA(nBufferLength, lpBuffer);
}

BOOL WINAPI WrappedGetVolumeInformationA(
  LPCSTR lpRootPathName,
  LPSTR lpVolumeNameBuffer,
  DWORD nVolumeNameSize,
  LPDWORD lpVolumeSerialNumber,
  LPDWORD lpMaximumComponentLength,
  LPDWORD lpFileSystemFlags,
  LPSTR lpFileSystemNameBuffer,
  DWORD nFileSystemNameSize)
{
  if (lpRootPathName && lpRootPathName[0] == VirtualDriveLetter[0])
  {
    std::vector<char> data(nVolumeNameSize, '\0');
    std::copy(MissionPackDisc.begin(), MissionPackDisc.end(), data.begin());
    std::copy(data.begin(), data.end(), lpVolumeNameBuffer);
    return TRUE;
  }

  return TrueGetVolumeInformationA(lpRootPathName,
    lpVolumeNameBuffer,
    nVolumeNameSize,
    lpVolumeSerialNumber,
    lpMaximumComponentLength,
    lpFileSystemFlags,
    lpFileSystemNameBuffer,
    nFileSystemNameSize);
}

HANDLE __stdcall WrappedCreateFileA(LPCSTR lpFileName,
  DWORD dwDesiredAccess,
  DWORD dwShareMode,
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  DWORD dwCreationDisposition,
  DWORD dwFlagsAndAttributes,
  HANDLE hTemplateFile)
{
  if (lpFileName)
  {
    auto filename = std::string_view(lpFileName);

    if (filename.starts_with(VirtualDriveLetter))
    {
      static std::map<std::string, std::string> mappings;
      std::string new_filename = std::string(filename);
      new_filename.replace(0, VirtualDriveLetter.size(), "");
      auto item = mappings.emplace(filename, std::move(new_filename));
      return TrueCreateFileA(item.first->second.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
  }

  return TrueCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

LSTATUS __stdcall WrappedRegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
  if (lpValueName && std::string_view(lpValueName) == "RAID2" && lpType && lpData && lpcbData)
  {
    *lpType = REG_SZ;
    std::memcpy(lpData, "Santa needs a new sled!", 24);
    *lpcbData = 24;
    return ERROR_SUCCESS;
  }

  return TrueRegQueryValueExA(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

static std::array<std::pair<void**, void*>, 6> detour_functions{ { { &(void*&)TrueGetLogicalDrives, WrappedGetLogicalDrives },
  { &(void*&)TrueGetLogicalDriveStringsA, WrappedGetLogicalDriveStringsA },
  { &(void*&)TrueRegQueryValueExA, WrappedRegQueryValueExA },
  { &(void*&)TrueGetDriveTypeA, WrappedGetDriveTypeA },
  { &(void*&)TrueGetVolumeInformationA, WrappedGetVolumeInformationA },
  { &(void*&)TrueCreateFileA, WrappedCreateFileA } } };

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
      try
      {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        std::for_each(detour_functions.begin(), detour_functions.end(), [](auto& func) { DetourAttach(func.first, func.second); });

        DetourTransactionCommit();
      }
      catch (...)
      {
        return FALSE;
      }
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
    }
  }

  return TRUE;
}
}
