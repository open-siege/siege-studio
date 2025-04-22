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

inline std::optional<std::string> vkey_to_key(WORD vkey)
{
  auto& mapping = get_key_to_vkey_mapping();

  for (auto& kv : mapping)
  {
    if (kv.second == vkey)
    {
      return siege::platform::to_upper(kv.first);
    }
  }

  if (std::isalpha(vkey))
  {
    return std::string(1, (char)vkey);
  }

  return std::nullopt;
}

inline std::optional<std::string_view> hardware_index_to_joystick_setting(WORD vkey, WORD index)
{
  if (vkey < VK_GAMEPAD_LEFT_THUMBSTICK_UP)
  {
    return std::nullopt;
  }
  switch (index)
  {
  case 0:
    return "joy_advaxisx";
  case 1:
    return "joy_advaxisy";
  case 2:
    return "joy_advaxisz";
  case 3:
    return "joy_advaxisr";
  case 4:
    return "joy_advaxisu";
  case 5:
    return "joy_advaxisv";
  default:
    return std::nullopt;
  }
}

inline bool is_vkey_for_controller(WORD vkey)
{
  return vkey >= VK_GAMEPAD_A && vkey <= VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT;
}

inline bool is_vkey_for_mouse(WORD vkey)
{
  return vkey == VK_LBUTTON || vkey == VK_RBUTTON || vkey == VK_MBUTTON || vkey == VK_XBUTTON1 || vkey == VK_XBUTTON2;
}

inline bool is_vkey_for_keyboard(WORD vkey)
{
  return !is_vkey_for_mouse(vkey) && !is_vkey_for_controller(vkey);
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
  // fov
  .string_settings = { { L"name", L"connect", L"map", L"gl_driver" } }
  // skin
  // teamname
  // bestweap
  // welcome_mess
  // joy_name
  // set adr0 "" to adr19
  //
  //
  // bool settings
  // s_musicenabled
  // s_nosound

  // float settings
  // s_musicvolume
  // s_volume
};

extern auto game_actions = std::array<game_action, 33>{ {
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
  game_action{ game_action::digital, "+altattack", u"Alt Attack", u"Combat" },
  game_action{ game_action::digital, "+melee-attack", u"Melee Attack", u"Combat" },
  game_action{ game_action::digital, "+use-plus-special", u"Special Action", u"Combat" },
  game_action{ game_action::digital, "weapnext", u"Next Weapon", u"Combat" },
  game_action{ game_action::digital, "weapprev", u"Previous Weapon", u"Combat" },
  game_action{ game_action::digital, "itemnext", u"Next Item", u"Combat" },
  game_action{ game_action::digital, "itemuse", u"Use Item", u"Combat" },
  game_action{ game_action::digital, "weapondrop", u"Drop Weapon", u"Combat" },
  game_action{ game_action::digital, "score", u"Score", u"Interface" },
  game_action{ game_action::digital, "menu objectives", u"Objectives", u"Interface" },
  game_action{ game_action::digital, "+klook", u"Keyboard Look", u"Misc" },
  game_action{ game_action::digital, "+mlook", u"Mouse Look", u"Misc" },
} };

constexpr static auto sof_aliases = std::array<std::array<std::string_view, 2>, 8>{ { { "+melee-attack", "weaponselect 1; +attack" },
  { "-melee-attack", "weaponbestsafe; -attack" },
  { "+use-plus-special1", "+weaponextra1; +use" },
  { "-use-plus-special1", "-weaponextra1; -use; alias +use-plus-special +use-plus-special2; alias -use-plus-special -use-plus-special2" },
  { "+use-plus-special2", "+weaponextra2; +use" },
  { "-use-plus-special2", "-weaponextra2; -use; alias +use-plus-special +use-plus-special1; alias -use-plus-special -use-plus-special1" },
  { "+use-plus-special", "+use-plus-special1" },
  { "-use-plus-special", "-use-plus-special1" } } };

extern auto controller_input_backends = std::array<const wchar_t*, 2>{ { L"winmm" } };
extern auto keyboard_input_backends = std::array<const wchar_t*, 2>{ { L"dinput" } };
extern auto mouse_input_backends = std::array<const wchar_t*, 2>{ { L"dinput" } };
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

  std::ofstream custom_bindings("user/siege_studio_inputs.cfg", std::ios::binary | std::ios::trunc);

  siege::configuration::text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

  for (auto& alias : sof_aliases)
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
            constexpr static std::string_view mlook = "+mlook";
            auto keyboard = std::find_if(args->action_bindings.begin(), args->action_bindings.end(), [&](auto& existing) {
              return existing.action_name.data() == mlook && (existing.context == siege::platform::hardware_context::global || existing.context == siege::platform::hardware_context::keyboard);
            });

            // TODO fix extended code support for SendInput
            if (action_name == "+lookup" && keyboard != args->action_bindings.end())
            {
              free_mapping->from_vkey = binding.vkey;
              free_mapping->from_context = siege::platform::hardware_context::controller;
              free_mapping->to_vkey = keyboard->vkey;
              free_mapping->to_context = keyboard->context;
            }
            else if (action_name == "+lookdown" && keyboard != args->action_bindings.end())
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
      auto game_key = vkey_to_key(binding.vkey);

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
          binding->inputs[index].virtual_key = *vkey;
          binding->inputs[index].input_type = mouse_binding::action_binding::button;
          binding->inputs[index].context = siege::platform::mouse_context::mouse;
          index++;

          if (index > binding->inputs.size())
          {
            break;
          }
        }
      }

      auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

      if (first_available != binding->inputs.end())
      {
        // TODO make this safer
        // TODO make this also update existing values
        auto action = std::find_if(game_actions.begin(), game_actions.end(), [](auto& action) { return std::string_view(action.action_name.data()) == "+altattack"; });
        std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
        first_available->input_type = mouse_binding::action_binding::button;
        first_available->virtual_key = VK_RBUTTON;

        std::advance(first_available, 1);

        action = std::find_if(game_actions.begin(), game_actions.end(), [](auto& action) { return std::string_view(action.action_name.data()) == "+use-plus-special"; });
        std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
        first_available->input_type = mouse_binding::action_binding::button;
        first_available->virtual_key = VK_MBUTTON;
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
          binding->inputs[index].virtual_key = *vkey;
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

      auto first_available = std::find_if(binding->inputs.begin(), binding->inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

      if (first_available != binding->inputs.end())
      {
        // TODO make this safer
        // TODO make this also update existing values
        auto action = std::find_if(game_actions.begin(), game_actions.end(), [](auto& action) { return std::string_view(action.action_name.data()) == "+melee-attack"; });
        std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
        first_available->input_type = keyboard_binding::action_binding::button;
        first_available->virtual_key = 'F';

        std::advance(first_available, 1);
        action = std::find_if(game_actions.begin(), game_actions.end(), [](auto& action) { return std::string_view(action.action_name.data()) == "+use-plus-special"; });
        std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
        first_available->input_type = keyboard_binding::action_binding::button;
        first_available->virtual_key = VK_RETURN;

        std::advance(first_available, 1);
        action = std::find_if(game_actions.begin(), game_actions.end(), [](auto& action) { return std::string_view(action.action_name.data()) == "itemuse"; });
        std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
        first_available->input_type = keyboard_binding::action_binding::button;
        first_available->virtual_key = 'G';

        std::advance(first_available, 1);
        action = std::find_if(game_actions.begin(), game_actions.end(), [](auto& action) { return std::string_view(action.action_name.data()) == "+moveup"; });
        std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
        first_available->input_type = keyboard_binding::action_binding::button;
        first_available->virtual_key = VK_SPACE;

        std::advance(first_available, 1);
        action = std::find_if(game_actions.begin(), game_actions.end(), [](auto& action) { return std::string_view(action.action_name.data()) == "+movedown"; });
        std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
        first_available->input_type = keyboard_binding::action_binding::button;
        first_available->virtual_key = VK_LCONTROL;
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
      { "+altattack", VK_GAMEPAD_LEFT_TRIGGER },
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
      { "+use-plus-special", VK_GAMEPAD_X },
      { "weapnext", VK_GAMEPAD_Y },
      { "itemnext", VK_GAMEPAD_LEFT_SHOULDER },
      { "itemuse", VK_GAMEPAD_RIGHT_SHOULDER },
      { "weapondrop", VK_GAMEPAD_DPAD_DOWN },
      { "weapprev", VK_GAMEPAD_DPAD_LEFT },
      { "weapnext", VK_GAMEPAD_DPAD_RIGHT },
      { "score", VK_GAMEPAD_VIEW },
      { "menu objectives", VK_GAMEPAD_MENU },
    };

    for (auto& controller_default : controller_defaults)
    {
      auto action = std::find_if(game_actions.begin(), game_actions.end(), [&](auto& action) { return std::string_view(action.action_name.data()) == controller_default.first; });
      std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());

      first_available->input_type = action->type == action->analog ? controller_binding::action_binding::axis : controller_binding::action_binding::button;
      first_available->virtual_key = controller_default.second;
      std::advance(first_available, 1);
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

/*
      constexpr static auto heretic2_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 10> {{
        {playstation::l2, "+defend" },
        {playstation::l1, "defprev" },
        {playstation::r1, "defnext"},
        {playstation::circle, "+creep"},
        {playstation::square, "+action"},
        {playstation::start, "menu_objectives"},
        {playstation::select, "menu_city_map"}
    }};

    constexpr static auto heretic2_aliases = std::array<std::array<std::string_view, 2>, 2> {{
            {"+melee-attack", "use staff; +attack"},
            {"-melee-attack", "weapprev; -attack"}
    }};
*/
