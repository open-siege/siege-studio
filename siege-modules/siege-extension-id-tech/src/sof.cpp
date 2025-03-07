#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <sstream>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <siege/resource/pak_resource.hpp>
#include <siege/configuration/id_tech.hpp>
#include <detours.h>
#include "shared.hpp"
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"


const std::map<std::string_view, WORD>& get_key_to_vkey_mapping()
{
  const static std::map<std::string_view, WORD> mapping = {
    { "f1", VK_F1 },
    { "f2", VK_F2 },
    { "f3", VK_F3 },
    { "f4", VK_F4 },
    { "f5", VK_F5 },
    { "f6", VK_F6 },
    { "f7", VK_F7 },
    { "f8", VK_F8 },
    { "f9", VK_F9 },
    { "f10", VK_F10 },
    { "f11", VK_F11 },
    { "f12", VK_F12 },
    { "tab", VK_TAB },
    { "lctrl", VK_LCONTROL },
    { "rctrl", VK_RCONTROL },
    { "lshift", VK_LSHIFT },
    { "rshift", VK_RSHIFT },
    { "lalt", VK_LMENU },
    { "ralt", VK_LMENU },
    { "uparrow", VK_LEFT },
    { "downarrow", VK_DOWN },
    { "leftarrow", VK_LEFT },
    { "rightarrow", VK_RIGHT },
    { "enter", VK_RETURN },
    { "home", VK_HOME },
    { "ins", VK_INSERT },
    { "pause", VK_PAUSE },
    { "kp_ins", VK_INSERT },
    { "kp_pgdn", VK_PAUSE },
    { "pgdn", VK_NEXT },
    { "pgup", VK_PRIOR },
    { "caps", VK_CAPITAL },
    { "del", VK_DELETE },
    { "end", VK_END },
    { "kp_del", MAKEWORD(VK_PACKET, VK_DELETE) },
    { "kp_enter", MAKEWORD(VK_PACKET, VK_RETURN) },
    { "kp_downarrow", VK_NUMPAD2 },
    { "kp_end", VK_PAUSE },
    { "semicolon", ';' },
    { "backspace", VK_BACK },
    { "space", VK_SPACE },
    { "mouse1", VK_LBUTTON },
    { "mouse2", VK_RBUTTON },
    { "mouse3", VK_MBUTTON },
  };

  return mapping;
}

inline std::optional<WORD> key_to_vkey(std::string_view value)
{
  auto lower = siege::platform::to_lower(value);
  auto& mapping = get_key_to_vkey_mapping();

  auto iter = mapping.find(lower);

  if (iter != mapping.end())
  {
    return iter->second;
  }

  if (value.size() == 1 && (std::isalpha(value[0]) || std::isdigit(value[0]) || std::ispunct(value[0])))
  {
    return value[0];
  }

  return std::nullopt;
}

extern "C" {
using game_action = siege::platform::game_action;
using keyboard_binding = siege::platform::keyboard_binding;
using mouse_binding = siege::platform::mouse_binding;
using controller_binding = siege::platform::controller_binding;
namespace fs = std::filesystem;

using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .int_settings = { { L"gl_mode" } },
  .string_settings = { { L"name", L"connect", L"map", L"gl_driver" } }
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
  game_action{ game_action::digital, "use-plus-speciau", u"Special Action", u"Combat" },
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
extern auto template_configuration_paths = std::array<const wchar_t*, 3>{ { L"base/pak0.pak/default.cfg", L"base/default.cfg" } };
extern auto autoexec_configuration_paths = std::array<const wchar_t*, 2>{ { L"base/user/autoexec.cfg" } };
extern auto profile_configuration_paths = std::array<const wchar_t*, 4>{ { L"base/user/configs/CURRENT.cfg", L"base/user/configs/*.cfg", L"base/user/config.cfg" } };

extern void(__cdecl* ConsoleEvalCdecl)(const char*);

using namespace std::literals;

constexpr std::array<std::array<std::pair<std::string_view, std::size_t>, 3>, 1> verification_strings = { { std::array<std::pair<std::string_view, std::size_t>, 3>{ { { "exec"sv, std::size_t(0x20120494) },
  { "cmdlist"sv, std::size_t(0x2012049c) },
  { "cl_minfps"sv, std::size_t(0x2011e600) } } } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 3> function_name_ranges{ { { "-klook"sv, "centerview"sv },
  { "joy_advancedupdate"sv, "+mlook"sv },
  { "rejected_violence"sv, "print"sv } } };

constexpr static std::array<std::pair<std::string_view, std::string_view>, 1> variable_name_ranges{ { { "joy_yawsensitivity"sv, "in_mouse"sv } } };

inline void set_gog_exports()
{
  ConsoleEvalCdecl = (decltype(ConsoleEvalCdecl))0x200194f0;
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

HRESULT init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return E_POINTER;
  }

  std::error_code last_error;

  if (fs::exists("base\\configs\\DEFAULT_KEYS.cfg", last_error))
  {
    std::ifstream stream("base\\configs\\DEFAULT_KEYS.cfg", std::ios::binary);
    auto size = fs::file_size("base\\configs\\DEFAULT_KEYS.cfg", last_error);
    auto config = siege::configuration::id_tech::id_tech_2::load_config(stream, size);

    if (config)
    {
      std::string temp;
      int index = 0;
      for (auto& item : config->keys())
      {
        if (item.at(0) == "bind")
        {
          auto vkey = key_to_vkey(item.at(1));

          if (!vkey)
          {
            continue;
          }

          auto entry = config->find(item);
          if (entry.at(0).empty())
          {
            continue;
          }
          temp = entry.at(0);
          std::memcpy(binding->inputs[index].action_name.data(), temp.data(), temp.size());
          binding->inputs[index].virtual_key = *vkey;
          binding->inputs[index].input_type = keyboard_binding::action_binding::button;
        }
      }
    }
  }
  else if (fs::exists("base\\pak0.pak", last_error))
  {
    std::any cache;
    siege::resource::pak::pak_resource_reader reader;

    std::ifstream stream("base//pak0.pak", std::ios::binary);
    auto contents = reader.get_content_listing(cache, stream, { .archive_path = "base//pak0.pak", .folder_path = "base//pak0.pak" });
  }


  return S_OK;
}

predefined_int*
  get_predefined_int_command_line_settings(const wchar_t* name) noexcept
{
  if (name == nullptr)
  {
    return nullptr;
  }

  auto name_str = std::wstring_view(name);


  if (name_str == L"gl_mode")
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
    thread_local HHOOK hook;
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
      int index = 0;
      try
      {
        auto app_module = win32::module_ref(::GetModuleHandleW(nullptr));

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
        hook = ::SetWindowsHookExW(WH_GETMESSAGE, dispatch_input_to_cdecl_quake_2_console, self, ::GetCurrentThreadId());
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
