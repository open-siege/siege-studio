#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"

namespace fs = std::filesystem;
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
  .int_settings = { { L"dedicated", L"vid_mode", L"in_joystick", L"autoweapon", L"win_noalttab", L"blood_level", L"s_khz" } },
  .string_settings = { { L"name", L"connect", L"map", L"game" } },
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .listen_setting = L"connect",
  .dedicated_setting = L"dedicated",
  .selected_game_setting = L"game"
};

extern auto game_actions = std::array<game_action, 32>{ {
  game_action{ game_action::analog, "+forward", u"Move Forward", u"Movement" },
  game_action{ game_action::analog, "+back", u"Move Backward", u"Movement" },
  game_action{ game_action::analog, "+moveleft", u"Strafe Left", u"Movement" },
  game_action{ game_action::analog, "+moveright", u"Strafe Right", u"Movement" },
  game_action{ game_action::analog, "+moveup", u"Jump", u"Movement" },
  game_action{ game_action::analog, "+movedown", u"Crouch", u"Movement" },
  game_action{ game_action::analog, "+creep", u"Creep", u"Movement" },
  game_action{ game_action::digital, "+speed", u"Run", u"Movement" },
  game_action{ game_action::analog, "+left", u"Turn Left", u"Aiming" },
  game_action{ game_action::analog, "+right", u"Turn Right", u"Aiming" },
  game_action{ game_action::analog, "+lookup", u"Look Up", u"Aiming" },
  game_action{ game_action::analog, "+lookdown", u"Look Down", u"Aiming" },
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "+defend", u"Activate Defensive Spell", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weaprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "defnext", u"Next Defensive Spell", u"Combat" },
  game_action{ game_action::digital, "defprev", u"Previous Defensive Spell", u"Combat" },
  game_action{ game_action::digital, "score", u"Score", u"Interface" },
  game_action{ game_action::digital, "menu_objectives", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
  game_action{ game_action::digital, "+lookaround", u"Lock Camera", u"Misc" },
} };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };
using namespace std::literals;

constexpr std::array<std::string_view, 2> verification_strings = std::array<std::string_view, 2>{ { "Quake2Main"sv,
  "quake2.dll"sv } };

std::errc executable_is_supported(const wchar_t* filename) noexcept
{
  if (filename && std::filesystem::path(filename).wstring().contains(L"Heretic2"))
  {
    return siege::executable_is_supported(filename, verification_strings);
  }

  return std::errc::not_supported;
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
  std::error_code last_error;
  fs::create_directory("user", last_error);

  std::ofstream custom_bindings("user/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  bool enable_controller = save_bindings_to_config(*args, config);

  if (enable_controller)
  {
    // engine bug - mouse needs to be enabled for the right analog stick to work
    config.emplace(siege::configuration::key_type({ "set", "in_mouse" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "set", "joy_advanced" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "set", "joy_sidesensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "set", "joy_pitchsensitivity" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "joy_advancedupdate" }), siege::configuration::key_type{});
  }

  config.save(custom_bindings);

  bind_axis_to_send_input(*args, "+lookup", "+mlook");
  bind_axis_to_send_input(*args, "+lookdown", "+mlook");

  auto iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name == nullptr; });

  if (iter != args->string_settings.end())
  {
    iter->name = L"exec";
    iter->value = L"siege_studio_inputs.cfg";
  }

  std::advance(iter, 1);
  iter->name = L"console";
  iter->value = L"1";

  return std::errc{};
}

std::errc init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }
  auto config = load_config_from_pak(L"base\\Default.cfg", L"base/Htic2-0.pak", L"base/Htic2-0.pak");

  if (config)
  {
    load_mouse_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 2> actions{
    { std::make_pair<WORD, std::string_view>(VK_RBUTTON, "+defend"),
      std::make_pair<WORD, std::string_view>(VK_MBUTTON, "defnext") }
  };

  upsert_mouse_defaults(game_actions, actions, *binding);

  std::array<std::pair<WORD, std::string_view>, 2> mouse_wheel_actions{
    { std::make_pair<WORD, std::string_view>(VK_UP, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_DOWN, "weaprev") }
  };

  upsert_mouse_defaults(game_actions, mouse_wheel_actions, *binding, mouse_context::mouse_wheel);

  return std::errc{};
}

std::errc init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return std::errc::bad_address;
  }

  auto config = load_config_from_pak(L"base\\Default.cfg", L"base/Htic2-0.pak", L"base/Htic2-0.pak");

  if (config)
  {
    load_keyboard_bindings(*config, *binding);
  }

  std::array<std::pair<WORD, std::string_view>, 12> actions{
    {
      std::make_pair<WORD, std::string_view>('W', "+forward"),
      std::make_pair<WORD, std::string_view>('A', "+moveleft"),
      std::make_pair<WORD, std::string_view>('S', "+back"),
      std::make_pair<WORD, std::string_view>('D', "+moveright"),
      std::make_pair<WORD, std::string_view>('F', "+defend"),
      std::make_pair<WORD, std::string_view>('Z', "defprev"),
      std::make_pair<WORD, std::string_view>('X', "defnext"),
      std::make_pair<WORD, std::string_view>(VK_RETURN, "+defend"),
      std::make_pair<WORD, std::string_view>(VK_SPACE, "+moveup"),
      std::make_pair<WORD, std::string_view>(VK_LCONTROL, "+movedown"),
      std::make_pair<WORD, std::string_view>(VK_TAB, "score"),
      std::make_pair<WORD, std::string_view>('C', "+creep"),
    }
  };

  upsert_keyboard_defaults(game_actions, actions, *binding, true);

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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_TRIGGER, "+defend"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_A, "+moveup"),
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
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON, "+lookaround"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_X, "inven"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_Y, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_LEFT_SHOULDER, "defnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_LEFT, "weapprev"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_DPAD_RIGHT, "weapnext"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_VIEW, "score"),
      std::make_pair<WORD, std::string_view>(VK_GAMEPAD_MENU, "menu_objectives"),
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

  if (name_str == L"vid_mode")
  {
    static auto modes = std::array<predefined_int, 8>{
      predefined_int{ .label = L"640x480", .value = 3 },
      predefined_int{ .label = L"800x600", .value = 4 },
      predefined_int{ .label = L"960x720", .value = 5 },
      predefined_int{ .label = L"1024x768", .value = 6 },
      predefined_int{ .label = L"1152x864", .value = 7 },
      predefined_int{ .label = L"1280x960", .value = 8 },
      predefined_int{ .label = L"1600x1200", .value = 9 },
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
    return get_predefined_id_tech_2_map_command_line_settings(L"base", false);
  }

  if (name && std::wstring_view(name) == L"vid_ref")
  {
    static auto modes = std::array<predefined_string, 3>{
      predefined_string{ .label = L"OpenGL", .value = L"gl" },
      predefined_string{ .label = L"Software", .value = L"soft" },
      predefined_string{},
    };

    return modes.data();
  }

  return nullptr;
}

static std::string VirtualDriveLetter;
constexpr static std::string_view Heretic2Disc = "HERETIC_II";
static auto* TrueGetLogicalDrives = GetLogicalDrives;
static auto* TrueGetDriveTypeA = GetDriveTypeA;
static auto* TrueGetVolumeInformationA = GetVolumeInformationA;
static auto* TrueCreateFileA = CreateFileA;

DWORD WINAPI WrappedGetLogicalDrives()
{
  auto result = TrueGetLogicalDrives();
  std::bitset<sizeof(DWORD) * 8> bits(result);

  int driveLetter = static_cast<int>('a');

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
    std::copy(Heretic2Disc.begin(), Heretic2Disc.end(), data.begin());
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

static std::array<std::pair<void**, void*>, 4> detour_functions{ { { &(void*&)TrueGetLogicalDrives, WrappedGetLogicalDrives },
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
        auto handle_a = ::GetModuleHandleW(L"quake2.dll");
        auto handle_b = ::GetModuleHandleW(L"H2Common.dll");

        if (!(handle_a && handle_b))
        {
          return FALSE;
        }

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
