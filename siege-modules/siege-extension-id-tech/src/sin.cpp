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
#include <siege/platform/win/com.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/platform/win/window_impl.hpp>
#include <detours.h>
#include "shared.hpp"
#include "GameExport.hpp"
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"

using hardware_context = siege::platform::hardware_context;
using game_action = siege::platform::game_action;
using keyboard_binding = siege::platform::keyboard_binding;
using mouse_binding = siege::platform::mouse_binding;
using controller_binding = siege::platform::controller_binding;

using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
namespace fs = std::filesystem;

extern "C" {

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .int_settings = { { L"gl_mode" } },
  .string_settings = { { L"name", L"connect", L"map", L"gl_driver" } },
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
  game_action{ game_action::digital, "+attack", u"Attack", u"Combat" },
  game_action{ game_action::digital, "weaponuse", u"Alt Attack", u"Combat" },
  game_action{ game_action::digital, "+melee-attack", u"Melee Attack", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weapprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "invnext", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "invuse", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "showinfo", u"Score", u"Interface" },
  game_action{ game_action::digital, "showhelp", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

constexpr static auto sin_aliases = std::array<std::array<std::string_view, 2>, 2>{ { { "+melee-attack", "use Fists; +attack" },
  { "-melee-attack", "-attack;weapprev;" } } };


extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };
extern auto keyboard_input_backends = std::array<const wchar_t*, 2>{ { L"user32" } };
extern auto mouse_input_backends = std::array<const wchar_t*, 2>{ { L"user32" } };
extern auto configuration_extensions = std::array<const wchar_t*, 2>{ { L".cfg" } };
extern auto template_configuration_paths = std::array<const wchar_t*, 3>{ { L"base/default.cfg", L"2015/default.cfg" } };
extern auto autoexec_configuration_paths = std::array<const wchar_t*, 2>{ { L"base/autoexec.cfg", L"2015/autoexec.cfg" } };
extern auto profile_configuration_paths = std::array<const wchar_t*, 3>{ { L"base/players/*/config.cfg", L"2015/players/*/config.cfg" } };

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

HRESULT apply_prelaunch_settings(const wchar_t* exe_path_str, siege::platform::game_command_line_args* args)
{
  if (exe_path_str == nullptr)
  {
    return E_POINTER;
  }

  if (args == nullptr)
  {
    return E_POINTER;
  }

  std::ofstream custom_bindings("base/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  for (auto& alias : sin_aliases)
  {
    config.emplace(siege::configuration::key_type({ "alias", alias[0] }), siege::configuration::key_type(alias[1]));
  }

  std::set<std::string> storage;

  bool enable_controller = false;
  for (auto& binding : args->action_bindings)
  {
    if (is_vkey_for_controller(binding.vkey))
    {
      enable_controller = true;
      auto setting = hardware_index_to_joystick_setting(binding.vkey, binding.hardware_index);

      if (setting)
      {
        const static auto controller_button_mapping = std::map<std::string_view, std::string_view>{
          { "+forward", "1" },
          { "+back", "1" },
          { "+lookup", "2" },
          { "+lookdown", "2" },
          { "+moveleft", "3" },
          { "+moveright", "3" },
          { "+left", "4" },
          { "+right", "4" },
          { "+moveup", "5" },
          { "+movedown", "5" }
        };

        try
        {
          auto action_name = std::string_view(binding.action_name.data());
          auto index = controller_button_mapping.at(action_name);

          auto free_mapping = std::find_if(args->controller_to_send_input_mappings.begin(), args->controller_to_send_input_mappings.end(), [](auto& mapping) {
            return mapping.from_vkey == 0;
          });

          if (free_mapping != args->controller_to_send_input_mappings.end())
          {
            constexpr static std::string_view lookup = "+lookup";
            constexpr static std::string_view lookdown = "+lookdown";
            auto keyboard = std::find_if(args->action_bindings.begin(), args->action_bindings.end(), [&](auto& existing) {
              return existing.action_name.data() == lookup && (existing.context == siege::platform::hardware_context::global || existing.context == siege::platform::hardware_context::keyboard);
            });

            // TODO fix extended code support for SendInput
            if (action_name == "+lookup" && keyboard != args->action_bindings.end())
            {
              free_mapping->from_vkey = binding.vkey;
              free_mapping->from_context = siege::platform::hardware_context::controller;
              free_mapping->to_vkey = keyboard->vkey;
              free_mapping->to_context = keyboard->context;
            }

            keyboard = std::find_if(args->action_bindings.begin(), args->action_bindings.end(), [&](auto& existing) {
              return existing.action_name.data() == lookdown && (existing.context == siege::platform::hardware_context::global || existing.context == siege::platform::hardware_context::keyboard);
            });
            
            if (action_name == "+lookdown" && keyboard != args->action_bindings.end())
            {
              free_mapping->from_vkey = binding.vkey;
              free_mapping->from_context = siege::platform::hardware_context::controller;
              free_mapping->to_vkey = keyboard->vkey;
              free_mapping->to_context = keyboard->context;
            }
          }

          config.emplace(siege::configuration::key_type({ "set", *setting }), siege::configuration::key_type(index));
        }
        catch (...)
        {
          // TODO handle unknown actions
        }
      }
      else
      {
        constexpr static auto button_names = std::array<std::string_view, 15>{ { "JOY1",
          "JOY2",
          "JOY3",
          "JOY4",
          "AUX5",
          "AUX6",
          "AUX7",
          "AUX8",
          "AUX9",
          "AUX10",
          "AUX11",
          "AUX12",
          "AUX13",
          "AUX14",
          "AUX15" } };

        if (binding.vkey == VK_GAMEPAD_DPAD_UP)
        {
          config.emplace(siege::configuration::key_type({ "bind", "AUX29" }), siege::configuration::key_type(binding.action_name.data()));
        }
        else if (binding.vkey == VK_GAMEPAD_DPAD_DOWN)
        {
          config.emplace(siege::configuration::key_type({ "bind", "AUX31" }), siege::configuration::key_type(binding.action_name.data()));
        }
        else if (binding.vkey == VK_GAMEPAD_DPAD_LEFT)
        {
          config.emplace(siege::configuration::key_type({ "bind", "AUX32" }), siege::configuration::key_type(binding.action_name.data()));
        }
        else if (binding.vkey == VK_GAMEPAD_DPAD_RIGHT)
        {
          config.emplace(siege::configuration::key_type({ "bind", "AUX30" }), siege::configuration::key_type(binding.action_name.data()));
        }
        else if (binding.vkey == VK_GAMEPAD_LEFT_TRIGGER || binding.vkey == VK_GAMEPAD_RIGHT_TRIGGER)
        {
          // TODO add a context check for "xbox" controllers
          auto action_name = std::string_view(binding.action_name.data());
          auto mouse = std::find_if(args->action_bindings.begin(), args->action_bindings.end(), [&](auto& existing) {
            return existing.action_name.data() == action_name && (existing.context == siege::platform::hardware_context::mouse);
          });

          if (mouse == args->action_bindings.end())
          {
            mouse = std::find_if(args->action_bindings.begin(), args->action_bindings.end(), [&](auto& existing) {
              return existing.action_name.data() == action_name && (existing.context == siege::platform::hardware_context::global);
            });
          }

          if (mouse != args->action_bindings.end())
          {
            auto free_mapping = std::find_if(args->controller_to_send_input_mappings.begin(), args->controller_to_send_input_mappings.end(), [](auto& mapping) {
              return mapping.from_vkey == 0;
            });

            free_mapping->from_vkey = binding.vkey;
            free_mapping->from_context = siege::platform::hardware_context::controller;
            free_mapping->to_vkey = mouse->vkey;
            free_mapping->to_context = mouse->context;
          }

          config.emplace(siege::configuration::key_type({ "bind", button_names[binding.hardware_index] }), siege::configuration::key_type(binding.action_name.data()));
        }
        else
        {
          config.emplace(siege::configuration::key_type({ "bind", button_names[binding.hardware_index] }), siege::configuration::key_type(binding.action_name.data()));
        }
      }
    }
    else
    {
      auto game_key = vkey_to_key(binding.vkey, binding.context);

      if (game_key)
      {
        auto iter = storage.emplace(*game_key);

        *binding.action_name.rbegin() = '\0';

        config.emplace(siege::configuration::key_type({ "bind", *iter.first }), siege::configuration::key_type(binding.action_name.data()));
      }
    }
  }

  if (enable_controller)
  {
    // engine bug - mouse needs to be enabled for the right analog stick to work
    config.emplace(siege::configuration::key_type({ "set", "in_mouse" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "set", "in_joystick" }), siege::configuration::key_type("1"));
    config.emplace(siege::configuration::key_type({ "set", "joy_advanced" }), siege::configuration::key_type("1"));
  }

  config.save(custom_bindings);

  auto iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name == nullptr; });

  if (iter != args->string_settings.end())
  {
    iter->name = L"exec";
    iter->value = L"siege_studio_inputs.cfg";
  }

  std::advance(iter, 1);
  iter->name = L"console";
  iter->value = L"1";

  return S_OK;
}

HRESULT init_mouse_inputs(mouse_binding* binding)
{
  if (binding == nullptr)
  {
    return E_POINTER;
  }
  auto config = load_config_from_pak(L"base\\default.cfg", L"base/pak0.sin", L"base/pak0.sin/default.cfg");

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

        if (!(item.at(1).starts_with("mouse") || item.at(1).starts_with("MOUSE")))
        {
          continue;
        }

        auto entry = config->find(item);
        if (entry.at(0).empty())
        {
          continue;
        }
        temp = entry.at(0);

        if (temp.size() - 1 > binding->inputs[index].action_name.size())
        {
          temp.resize(binding->inputs[index].action_name.size() - 1);
        }

        temp = siege::platform::to_lower(temp);

        std::memcpy(binding->inputs[index].action_name.data(), temp.data(), temp.size());
        binding->inputs[index].virtual_key = vkey->first;
        binding->inputs[index].input_type = mouse_binding::action_binding::button;
        binding->inputs[index].context = siege::platform::mouse_context::mouse;
        index++;

        if (index > binding->inputs.size())
        {
          break;
        }
      }
    }
    std::array<std::pair<WORD, std::string_view>, 1> actions{
      { std::make_pair<WORD, std::string_view>(VK_RBUTTON, "+altattack") }
    };

    for (auto action_str : actions)
    {
      auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

      if (first_available == binding->inputs.end())
      {
        break;
      }
      // TODO make this also update existing values
      auto action = std::find_if(game_actions.begin(), game_actions.end(), [&](auto& action) { return std::string_view(action.action_name.data()) == action_str.second; });
      if (action == game_actions.end())
      {
        continue;
      }
      std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
      first_available->input_type = mouse_binding::action_binding::button;
      first_available->virtual_key = action_str.first;
    }
  }

  return S_OK;
}

HRESULT init_keyboard_inputs(keyboard_binding* binding)
{
  if (binding == nullptr)
  {
    return E_POINTER;
  }

  auto config = load_config_from_pak(L"base\\default.cfg", L"base/pak0.sin", L"base/pak0.sin/default.cfg");

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

        if (item.at(1).starts_with("mouse") || item.at(1).starts_with("MOUSE"))
        {
          continue;
        }

        auto entry = config->find(item);
        if (entry.at(0).empty())
        {
          continue;
        }
        temp = entry.at(0);

        if (temp.size() - 1 > binding->inputs[index].action_name.size())
        {
          temp.resize(binding->inputs[index].action_name.size() - 1);
        }

        std::memcpy(binding->inputs[index].action_name.data(), temp.data(), temp.size());
        binding->inputs[index].virtual_key = vkey->first;
        binding->inputs[index].input_type = keyboard_binding::action_binding::button;
        binding->inputs[index].context = siege::platform::keyboard_context::keyboard;

        if (item.at(1).starts_with("kp_") || item.at(1).starts_with("KP_"))
        {
          binding->inputs[index].context = siege::platform::keyboard_context::keypad;
        }

        index++;

        if (index > binding->inputs.size())
        {
          break;
        }
      }
    }

    std::array<std::pair<WORD, std::string_view>, 5> actions{
      {
        std::make_pair<WORD, std::string_view>('F', "+melee-attack"),
        std::make_pair<WORD, std::string_view>(VK_RETURN, "+use"),
        std::make_pair<WORD, std::string_view>('G', "itemuse"),
        std::make_pair<WORD, std::string_view>(VK_SPACE, "+moveup"),
        std::make_pair<WORD, std::string_view>(VK_LCONTROL, "+movedown"),
      }
    };

    for (auto action_str : actions)
    {
      auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

      if (first_available == binding->inputs.end())
      {
        break;
      }

      auto action = std::find_if(game_actions.begin(), game_actions.end(), [&](auto& action) { return std::string_view(action.action_name.data()) == action_str.second; });

      if (action == game_actions.end())
      {
        continue;
      }

      std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
      first_available->input_type = keyboard_binding::action_binding::button;
      first_available->virtual_key = action_str.first;
    }
  }
  return S_OK;
}

HRESULT init_controller_inputs(controller_binding* binding)
{
  if (binding == nullptr)
  {
    return E_POINTER;
  }

  auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

  if (first_available != binding->inputs.end())
  {
    const static std::map<std::string_view, WORD> controller_defaults = {
      { "+attack", VK_GAMEPAD_RIGHT_TRIGGER },
      { "weaponuse", VK_GAMEPAD_LEFT_TRIGGER },
      { "+moveup", VK_GAMEPAD_A },
      { "+movedown", VK_GAMEPAD_B },
      { "+speed", VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON },
      { "+forward", VK_GAMEPAD_LEFT_THUMBSTICK_UP },
      { "+back", VK_GAMEPAD_LEFT_THUMBSTICK_DOWN },
      { "+moveleft", VK_GAMEPAD_LEFT_THUMBSTICK_LEFT },
      { "+moveright", VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT },
      { "+left", VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT },
      { "+right", VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT },
      { "+lookup", VK_GAMEPAD_RIGHT_THUMBSTICK_UP },
      { "+lookdown", VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN },
      { "+melee-attack", VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON },
      { "+use", VK_GAMEPAD_X },
      { "weapnext", VK_GAMEPAD_Y },
      { "invnext", VK_GAMEPAD_LEFT_SHOULDER },
      { "invuse", VK_GAMEPAD_RIGHT_SHOULDER },
      { "weapondrop", VK_GAMEPAD_DPAD_DOWN },
      { "weapprev", VK_GAMEPAD_DPAD_LEFT },
      { "weapnext", VK_GAMEPAD_DPAD_RIGHT },
      { "showinfo", VK_GAMEPAD_VIEW },
      { "showhelp", VK_GAMEPAD_MENU },
    };

    for (auto& controller_default : controller_defaults)
    {
      auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

      if (first_available == binding->inputs.end())
      {
        break;
      }
      auto action = std::find_if(game_actions.begin(), game_actions.end(), [&](auto& action) { return std::string_view(action.action_name.data()) == controller_default.first; });

      if (action == game_actions.end())
      {
        continue;
      }
      std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());

      first_available->input_type = action->type == action->analog ? controller_binding::action_binding::axis : controller_binding::action_binding::button;
      first_available->virtual_key = controller_default.second;
    }
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
