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
#include <siege/resource/pak_resource.hpp>
#include <siege/resource/zip_resource.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>
#include "GetGameFunctionNames.hpp"
#include "id-tech-shared.hpp"

namespace fs = std::filesystem;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;

const std::map<std::string_view, std::pair<WORD, hardware_context>>& get_key_to_vkey_mapping()
{
  const static std::map<std::string_view, std::pair<WORD, hardware_context>> mapping = {
    { "f1", std::make_pair(VK_F1, hardware_context::keyboard) },
    { "f2", std::make_pair(VK_F2, hardware_context::keyboard) },
    { "f3", std::make_pair(VK_F3, hardware_context::keyboard) },
    { "f4", std::make_pair(VK_F4, hardware_context::keyboard) },
    { "f5", std::make_pair(VK_F5, hardware_context::keyboard) },
    { "f6", std::make_pair(VK_F6, hardware_context::keyboard) },
    { "f7", std::make_pair(VK_F7, hardware_context::keyboard) },
    { "f8", std::make_pair(VK_F8, hardware_context::keyboard) },
    { "f9", std::make_pair(VK_F9, hardware_context::keyboard) },
    { "f10", std::make_pair(VK_F10, hardware_context::keyboard) },
    { "f11", std::make_pair(VK_F11, hardware_context::keyboard) },
    { "f12", std::make_pair(VK_F12, hardware_context::keyboard) },
    { "tab", std::make_pair(VK_TAB, hardware_context::keyboard) },
    { "ctrl", std::make_pair(VK_CONTROL, hardware_context::keyboard) },
    { "alt", std::make_pair(VK_MENU, hardware_context::keyboard) },
    { "lctrl", std::make_pair(VK_LCONTROL, hardware_context::keyboard) },
    { "rctrl", std::make_pair(VK_RCONTROL, hardware_context::keyboard) },
    { "shift", std::make_pair(VK_SHIFT, hardware_context::keyboard) },
    { "lshift", std::make_pair(VK_LSHIFT, hardware_context::keyboard) },
    { "rshift", std::make_pair(VK_RSHIFT, hardware_context::keyboard) },
    { "lalt", std::make_pair(VK_LMENU, hardware_context::keyboard) },
    { "ralt", std::make_pair(VK_RMENU, hardware_context::keyboard) },
    { "uparrow", std::make_pair(VK_UP, hardware_context::keyboard) },
    { "downarrow", std::make_pair(VK_DOWN, hardware_context::keyboard) },
    { "leftarrow", std::make_pair(VK_LEFT, hardware_context::keyboard) },
    { "rightarrow", std::make_pair(VK_RIGHT, hardware_context::keyboard) },
    { "enter", std::make_pair(VK_RETURN, hardware_context::keyboard) },
    { "home", std::make_pair(VK_HOME, hardware_context::keyboard) },
    { "ins", std::make_pair(VK_INSERT, hardware_context::keyboard) },
    { "pause", std::make_pair(VK_PAUSE, hardware_context::keyboard) },
    { "pgdn", std::make_pair(VK_NEXT, hardware_context::keyboard) },
    { "pgup", std::make_pair(VK_PRIOR, hardware_context::keyboard) },
    { "caps", std::make_pair(VK_CAPITAL, hardware_context::keyboard) },
    { "del", std::make_pair(VK_DELETE, hardware_context::keyboard) },
    { "end", std::make_pair(VK_END, hardware_context::keyboard) },
    { "kp_ins", std::make_pair(VK_INSERT, hardware_context::keypad) },
    { "kp_pgdn", std::make_pair(VK_PAUSE, hardware_context::keypad) },
    { "kp_del", std::make_pair(VK_DELETE, hardware_context::keyboard) },
    { "kp_enter", std::make_pair(VK_RETURN, hardware_context::keypad) },
    { "kp_downarrow", std::make_pair(VK_NUMPAD2, hardware_context::keypad) },
    { "kp_end", std::make_pair(VK_END, hardware_context::keypad) },
    { "kp_numlock", std::make_pair(VK_NUMLOCK, hardware_context::keypad) },
    { "backspace", std::make_pair(VK_BACK, hardware_context::keyboard) },
    { "space", std::make_pair(VK_SPACE, hardware_context::keyboard) },
    { "mouse1", std::make_pair(VK_LBUTTON, hardware_context::mouse) },
    { "mouse2", std::make_pair(VK_RBUTTON, hardware_context::mouse) },
    { "mouse3", std::make_pair(VK_MBUTTON, hardware_context::mouse) },
    { "mwheelup", std::make_pair(VK_UP, hardware_context::mouse_wheel) },
    { "mwheeldown", std::make_pair(VK_DOWN, hardware_context::mouse_wheel) },
    { "semicolon", std::make_pair(VK_OEM_1, hardware_context::keyboard) },
    { ";", std::make_pair(VK_OEM_1, hardware_context::keyboard) },
    { "/", std::make_pair(VK_OEM_2, hardware_context::keyboard) },
    { "`", std::make_pair(VK_OEM_3, hardware_context::keyboard) },
    { "[", std::make_pair(VK_OEM_4, hardware_context::keyboard) },
    { "\\", std::make_pair(VK_OEM_5, hardware_context::keyboard) },
    { "]", std::make_pair(VK_OEM_6, hardware_context::keyboard) },
    { "'", std::make_pair(VK_OEM_6, hardware_context::keyboard) },
    { "-", std::make_pair(VK_OEM_MINUS, hardware_context::keyboard) },
    { "=", std::make_pair(VK_OEM_PLUS, hardware_context::keyboard) },
    { ",", std::make_pair(VK_OEM_COMMA, hardware_context::keyboard) },
    { ".", std::make_pair(VK_OEM_PERIOD, hardware_context::keyboard) },
    { ":", std::make_pair(VK_OEM_1, hardware_context::keyboard_shifted) },
    { "?", std::make_pair(VK_OEM_2, hardware_context::keyboard_shifted) },
    { "~", std::make_pair(VK_OEM_3, hardware_context::keyboard_shifted) },
    { "{", std::make_pair(VK_OEM_4, hardware_context::keyboard_shifted) },
    { "|", std::make_pair(VK_OEM_5, hardware_context::keyboard_shifted) },
    { "}", std::make_pair(VK_OEM_6, hardware_context::keyboard_shifted) },
    { "\"", std::make_pair(VK_OEM_6, hardware_context::keyboard_shifted) },
    { "_", std::make_pair(VK_OEM_MINUS, hardware_context::keyboard_shifted) },
    { "+", std::make_pair(VK_OEM_PLUS, hardware_context::keyboard_shifted) },
    { "<", std::make_pair(VK_OEM_COMMA, hardware_context::keyboard_shifted) },
    { "<", std::make_pair(VK_OEM_PERIOD, hardware_context::keyboard_shifted) },
  };

  return mapping;
}

std::optional<std::pair<WORD, hardware_context>> key_to_vkey(std::string_view value)
{
  auto lower = siege::platform::to_lower(value);
  auto& mapping = get_key_to_vkey_mapping();

  auto iter = mapping.find(lower);

  if (iter != mapping.end())
  {
    return iter->second;
  }

  if (value.size() == 1 && (std::isalpha(value[0]) || std::isdigit(value[0])))
  {
    return std::make_pair((WORD)std::toupper(value[0]), hardware_context::keyboard);
  }

  return std::nullopt;
}

std::optional<std::string> vkey_to_key(WORD vkey, hardware_context context)
{
  auto& mapping = get_key_to_vkey_mapping();

  for (auto& kv : mapping)
  {
    if (kv.second.first == vkey && kv.second.second == context)
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

std::optional<std::string_view> hardware_index_to_button_name_id_tech_2_0(WORD index)
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

  if (index > button_names.size())
  {
    return std::nullopt;
  }
  return button_names[index];
}

std::optional<std::string_view> hardware_index_to_button_name_id_tech_3_0(WORD index)
{
  constexpr static auto button_names = std::array<std::string_view, 15>{ { "JOY1",
    "JOY2",
    "JOY3",
    "JOY4",
    "JOY5",
    "JOY6",
    "JOY7",
    "JOY8",
    "JOY9",
    "JOY10",
    "JOY11",
    "JOY12",
    "JOY13",
    "JOY14",
    "JOY15" } };

  if (index > button_names.size())
  {
    return std::nullopt;
  }
  return button_names[index];
}

std::optional<std::string_view> dpad_name_id_tech_2_0(WORD vkey)
{
  switch (vkey)
  {
  case VK_GAMEPAD_DPAD_UP:
    return "AUX29";
  case VK_GAMEPAD_DPAD_DOWN:
    return "AUX29";
  case VK_GAMEPAD_DPAD_LEFT:
    return "AUX29";
  case VK_GAMEPAD_DPAD_RIGHT:
    return "AUX29";
  default:
    return std::nullopt;
  }
}
std::optional<std::string_view> dpad_name_id_tech_3_0(WORD vkey)
{
  switch (vkey)
  {
  case VK_GAMEPAD_DPAD_UP:
    return "JOY24";
  case VK_GAMEPAD_DPAD_DOWN:
    return "JOY25";
  case VK_GAMEPAD_DPAD_LEFT:
    return "JOY27";
  case VK_GAMEPAD_DPAD_RIGHT:
    return "JOY26";
  default:
    return std::nullopt;
  }
}


std::optional<std::string_view> hardware_index_to_joystick_axis_id_tech_3_0(WORD vkey, WORD index)
{
  if (vkey >= VK_GAMEPAD_A && vkey <= VK_GAMEPAD_LEFT_SHOULDER)
  {
    return std::nullopt;
  }

  if (vkey >= VK_GAMEPAD_DPAD_UP && vkey <= VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON)
  {
    return std::nullopt;
  }

  bool is_positive = vkey == VK_GAMEPAD_LEFT_THUMBSTICK_UP || vkey == VK_GAMEPAD_RIGHT_THUMBSTICK_UP || vkey == VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT || vkey == VK_GAMEPAD_LEFT_TRIGGER;
  switch (index)
  {
  case 0: {
    return is_positive ? "UPARROW" : "DOWNARROW";
  }
  case 1: {
    return is_positive ? "RIGHTARROW" : "LEFTARROW";
  }
  case 2: {
    return is_positive ? "JOY17" : "JOY16";
  }
  case 3: {
    return is_positive ? "JOY18" : "JOY19";
  }
  default:
    return std::nullopt;
  }
}

std::optional<std::string_view> hardware_index_to_joystick_axis_id_tech_2_5(WORD vkey, WORD index)
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

std::optional<std::string_view> hardware_index_to_joystick_axis_id_tech_2_0(WORD vkey, WORD index)
{
  if (vkey < VK_GAMEPAD_LEFT_THUMBSTICK_UP)
  {
    return std::nullopt;
  }
  switch (index)
  {
  case 0:
    return "joyadvaxisx";
  case 1:
    return "joyadvaxisy";
  case 2:
    return "joyadvaxisz";
  case 3:
    return "joyadvaxisr";
  case 4:
    return "joyadvaxisu";
  case 5:
    return "joyadvaxisv";
  default:
    return std::nullopt;
  }
}

std::optional<siege::configuration::text_game_config> load_config_from_pak(fs::path real_file_path, std::wstring pak_path, std::wstring pak_folder_path)
{
  std::error_code last_error;
  if (fs::exists(real_file_path, last_error))
  {
    std::ifstream stream(real_file_path, std::ios::binary);
    auto size = fs::file_size(real_file_path, last_error);
    return siege::configuration::id_tech::id_tech_2::load_config(stream, size);
  }
  else if (fs::exists(pak_path, last_error))
  {
    std::any cache;
    siege::resource::pak::pak_resource_reader reader;

    std::ifstream stream(pak_path, std::ios::binary);
    auto contents = reader.get_content_listing(cache, stream, { .archive_path = pak_path, .folder_path = pak_folder_path });

    auto default_keys = std::find_if(contents.begin(), contents.end(), [&](auto& info) {
      if (auto* file_info = std::get_if<siege::platform::resource_reader::file_info>(&info))
      {
        return file_info->filename.wstring() == real_file_path.filename();
      }

      return false;
    });

    if (default_keys != contents.end())
    {
      std::stringstream temp;
      reader.extract_file_contents(cache, stream, std::get<siege::platform::resource_reader::file_info>(*default_keys), temp);
      auto size = (std::size_t)temp.tellp();
      temp.seekg(0);
      return siege::configuration::id_tech::id_tech_2::load_config(temp, size);
    }
  }

  return std::nullopt;
}

std::optional<siege::configuration::text_game_config> load_config_from_pk3(fs::path real_file_path, std::wstring pak_path, std::wstring pak_folder_path)
{
  std::error_code last_error;
  if (fs::exists(real_file_path, last_error))
  {
    std::ifstream stream(real_file_path, std::ios::binary);
    auto size = fs::file_size(real_file_path, last_error);
    return siege::configuration::id_tech::id_tech_2::load_config(stream, size);
  }
  else if (fs::exists(pak_path, last_error))
  {
    std::any cache;
    siege::resource::zip::zip_resource_reader reader;

    std::ifstream stream(pak_path, std::ios::binary);
    auto contents = reader.get_content_listing(cache, stream, { .archive_path = pak_path, .folder_path = pak_folder_path });

    auto default_keys = std::find_if(contents.begin(), contents.end(), [&](auto& info) {
      if (auto* file_info = std::get_if<siege::platform::resource_reader::file_info>(&info))
      {
        return file_info->filename.wstring() == real_file_path.filename();
      }

      return false;
    });

    if (default_keys != contents.end())
    {
      std::stringstream temp;
      reader.extract_file_contents(cache, stream, std::get<siege::platform::resource_reader::file_info>(*default_keys), temp);
      auto size = (std::size_t)temp.tellp();
      temp.seekg(0);
      return siege::configuration::id_tech::id_tech_2::load_config(temp, size);
    }
  }

  return std::nullopt;
}

void load_mouse_bindings(siege::configuration::text_game_config& config, siege::platform::mouse_binding& binding)
{
  std::string temp;
  int index = 0;
  for (auto& item : config.keys())
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

      auto entry = config.find(item);
      if (entry.at(0).empty())
      {
        continue;
      }
      temp = entry.at(0);

      if (temp.size() - 1 > binding.inputs[index].action_name.size())
      {
        temp.resize(binding.inputs[index].action_name.size() - 1);
      }

      temp = siege::platform::to_lower(temp);

      // TODO handle mouse wheel too
      std::memcpy(binding.inputs[index].action_name.data(), temp.data(), temp.size());
      binding.inputs[index].virtual_key = vkey->first;
      binding.inputs[index].input_type = siege::platform::mouse_binding::action_binding::button;
      binding.inputs[index].context = siege::platform::mouse_context::mouse;
      index++;

      if (index > binding.inputs.size())
      {
        break;
      }
    }
  }
}

void load_keyboard_bindings(siege::configuration::text_game_config& config, siege::platform::keyboard_binding& binding)
{
  std::string temp;
  int index = 0;
  for (auto& item : config.keys())
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

      auto entry = config.find(item);
      if (entry.at(0).empty())
      {
        continue;
      }
      temp = entry.at(0);

      if (temp.size() - 1 > binding.inputs[index].action_name.size())
      {
        temp.resize(binding.inputs[index].action_name.size() - 1);
      }

      std::memcpy(binding.inputs[index].action_name.data(), temp.data(), temp.size());
      binding.inputs[index].virtual_key = vkey->first;
      binding.inputs[index].input_type = siege::platform::keyboard_binding::action_binding::button;
      binding.inputs[index].context = (siege::platform::keyboard_context)vkey->second;

      index++;

      if (index > binding.inputs.size())
      {
        break;
      }
    }
  }
}

void upsert_mouse_defaults(const std::span<siege::platform::game_action> game_actions, const std::span<std::pair<WORD, std::string_view>> actions, siege::platform::mouse_binding& binding)
{
  for (auto action_str : actions)
  {
    auto action = std::find_if(game_actions.begin(), game_actions.end(), [&](auto& action) { return std::string_view(action.action_name.data()) == action_str.second; });
    if (action == game_actions.end())
    {
      continue;
    }

    auto existing = std::find_if(binding.inputs.begin(), binding.inputs.end(), [&](auto& input) { return input.virtual_key == action_str.first; });

    if (existing != binding.inputs.end())
    {

      std::memcpy(existing->action_name.data(), action->action_name.data(), action->action_name.size());
      existing->input_type = siege::platform::mouse_binding::action_binding::button;
      existing->virtual_key = action_str.first;
      continue;
    }

    auto first_available = std::find_if(binding.inputs.begin(), binding.inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

    if (first_available == binding.inputs.end())
    {
      continue;
    }

    std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
    first_available->input_type = siege::platform::mouse_binding::action_binding::button;
    first_available->virtual_key = action_str.first;
  }
}

void upsert_keyboard_defaults(const std::span<siege::platform::game_action> game_actions, const std::span<std::pair<WORD, std::string_view>> actions, siege::platform::keyboard_binding& binding, bool ignore_case)
{
  for (auto action_str : actions)
  {
    auto action = std::find_if(game_actions.begin(), game_actions.end(), [&](auto& action) { return std::string_view(action.action_name.data()) == action_str.second; });

    if (action == game_actions.end())
    {
      continue;
    }

    auto existing = std::find_if(binding.inputs.begin(), binding.inputs.end(), [&](auto& input) { return input.virtual_key == action_str.first; });

    if (!ignore_case && existing == binding.inputs.end() && std::isalpha(action_str.first))
    {
      existing = std::find_if(binding.inputs.begin(), binding.inputs.end(), [&](auto& input) { return input.virtual_key == (std::uint16_t)std::tolower(action_str.first); });
    }

    if (existing != binding.inputs.end())
    {

      std::memcpy(existing->action_name.data(), action->action_name.data(), action->action_name.size());
      existing->input_type = siege::platform::keyboard_binding::action_binding::button;
      existing->virtual_key = action_str.first;
      continue;
    }


    auto first_available = std::find_if(binding.inputs.begin(), binding.inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

    if (first_available == binding.inputs.end())
    {
      continue;
    }

    std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());
    first_available->input_type = siege::platform::keyboard_binding::action_binding::button;
    first_available->virtual_key = action_str.first;
  }
}

void append_controller_defaults(const std::span<siege::platform::game_action> game_actions, const std::span<std::pair<WORD, std::string_view>> actions, siege::platform::controller_binding& binding)
{
  for (auto& controller_default : actions)
  {
    auto first_available = std::find_if(binding.inputs.begin(), binding.inputs.end(), [](auto& input) { return input.action_name[0] == '\0'; });

    if (first_available == binding.inputs.end())
    {
      break;
    }
    auto action = std::find_if(game_actions.begin(), game_actions.end(), [&](auto& action) { return std::string_view(action.action_name.data()) == controller_default.second; });

    if (action == game_actions.end())
    {
      continue;
    }
    std::memcpy(first_available->action_name.data(), action->action_name.data(), action->action_name.size());

    first_available->input_type = action->type == action->analog ? siege::platform::controller_binding::action_binding::axis : siege::platform::controller_binding::action_binding::button;
    first_available->virtual_key = controller_default.first;
  }
}

bool save_bindings_to_config(siege::platform::game_command_line_args& args, siege::configuration::text_game_config& config, mapping_context context)
{
  static std::set<std::string> storage;
  bool enable_controller = false;
  for (auto& binding : args.action_bindings)
  {
    if (is_vkey_for_controller(binding.vkey))
    {
      enable_controller = true;
      auto setting = context.index_to_axis(binding.vkey, binding.hardware_index);

      if (setting)
      {
        if (binding.vkey == VK_GAMEPAD_LEFT_TRIGGER || binding.vkey == VK_GAMEPAD_RIGHT_TRIGGER && context.supports_triggers_as_buttons)
        {
          goto button_section;
        }
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

          if (context.axis_set_prefix == "bind")
          {
            config.emplace(siege::configuration::key_type({ context.axis_set_prefix, *setting }), siege::configuration::key_type(action_name));
          }
          else if (!context.axis_set_prefix.empty())
          {
            config.emplace(siege::configuration::key_type({ context.axis_set_prefix, *setting }), siege::configuration::key_type(index));
          }
          else
          {
            config.emplace(siege::configuration::key_type({ *setting }), siege::configuration::key_type(index));
          }
        }
        catch (...)
        {
          // TODO handle unknown actions
        }
      }
      else
      {
      button_section:
        auto dpad_name = context.dpad_name(binding.vkey);
        if (binding.vkey == VK_GAMEPAD_DPAD_UP && dpad_name)
        {
          config.emplace(siege::configuration::key_type({ "bind", *dpad_name }), siege::configuration::key_type(binding.action_name.data()));
        }
        else if (binding.vkey == VK_GAMEPAD_DPAD_DOWN && dpad_name)
        {
          config.emplace(siege::configuration::key_type({ "bind", *dpad_name }), siege::configuration::key_type(binding.action_name.data()));
        }
        else if (binding.vkey == VK_GAMEPAD_DPAD_LEFT && dpad_name)
        {
          config.emplace(siege::configuration::key_type({ "bind", *dpad_name }), siege::configuration::key_type(binding.action_name.data()));
        }
        else if (binding.vkey == VK_GAMEPAD_DPAD_RIGHT && dpad_name)
        {
          config.emplace(siege::configuration::key_type({ "bind", *dpad_name }), siege::configuration::key_type(binding.action_name.data()));
        }
        else if (binding.vkey == VK_GAMEPAD_LEFT_TRIGGER || binding.vkey == VK_GAMEPAD_RIGHT_TRIGGER)
        {
          if (binding.context == hardware_context::controller_xbox && !context.supports_triggers_as_buttons)
          {
            auto action_name = std::string_view(binding.action_name.data());
            auto mouse = std::find_if(args.action_bindings.begin(), args.action_bindings.end(), [&](auto& existing) {
              return existing.action_name.data() == action_name && (existing.context == siege::platform::hardware_context::mouse);
            });

            if (mouse == args.action_bindings.end())
            {
              mouse = std::find_if(args.action_bindings.begin(), args.action_bindings.end(), [&](auto& existing) {
                return existing.action_name.data() == action_name && (existing.context == siege::platform::hardware_context::keyboard || existing.context == siege::platform::hardware_context::keypad || existing.context == siege::platform::hardware_context::global);
              });
            }

            if (auto free_mapping = std::find_if(args.controller_to_send_input_mappings.begin(), args.controller_to_send_input_mappings.end(), [](auto& mapping) {
                  return mapping.from_vkey == 0;
                });
              free_mapping != args.controller_to_send_input_mappings.end() && mouse != args.action_bindings.end())
            {
              free_mapping->from_vkey = binding.vkey;
              free_mapping->from_context = binding.context;
              free_mapping->to_vkey = mouse->vkey;
              free_mapping->to_context = mouse->context;
            }
          }
          else if (binding.context == hardware_context::controller_xbox && context.supports_triggers_as_buttons)
          {
            auto axis = context.index_to_axis(binding.vkey, binding.hardware_index);
            if (axis)
            {
              config.emplace(siege::configuration::key_type({ "bind", *axis }), siege::configuration::key_type(binding.action_name.data()));
            }
          }
          else if (binding.context != hardware_context::controller_xbox)
          {
            auto button = context.index_to_button(binding.hardware_index);
            if (button)
            {
              config.emplace(siege::configuration::key_type({ "bind", *button }), siege::configuration::key_type(binding.action_name.data()));
            }
          }
        }
        else
        {
          auto button = context.index_to_button(binding.hardware_index);
          if (button)
          {
            config.emplace(siege::configuration::key_type({ "bind", *button }), siege::configuration::key_type(binding.action_name.data()));
          }
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

  return enable_controller;
}

void bind_axis_to_send_input(siege::platform::game_command_line_args& args, std::string_view source, std::string_view target)
{
  for (auto& binding : args.action_bindings)
  {
    if (!is_vkey_for_controller(binding.vkey))
    {
      continue;
    }

    auto action_name = std::string_view(binding.action_name.data());

    if (action_name != source)
    {
      continue;
    }

    auto axis = hardware_index_to_joystick_axis_id_tech_2_5(binding.vkey, binding.hardware_index);

    if (!axis)
    {
      continue;
    }

    auto free_mapping = std::find_if(args.controller_to_send_input_mappings.begin(), args.controller_to_send_input_mappings.end(), [](auto& mapping) {
      return mapping.from_vkey == 0;
    });

    if (free_mapping == args.controller_to_send_input_mappings.end())
    {
      continue;
    }


    auto keyboard = std::find_if(args.action_bindings.begin(), args.action_bindings.end(), [&](auto& existing) {
      return existing.action_name.data() == target && (existing.context == siege::platform::hardware_context::global || existing.context == siege::platform::hardware_context::keyboard);
    });

    if (keyboard != args.action_bindings.end())
    {
      free_mapping->from_vkey = binding.vkey;
      free_mapping->from_context = binding.context;
      free_mapping->to_vkey = keyboard->vkey;
      free_mapping->to_context = keyboard->context;
      break;
    }
  }
}

extern "C" {
extern void(__cdecl* ConsoleEvalCdecl)(const char*) = nullptr;
extern void(__fastcall* ConsoleEvalFastcall)(const char*) = nullptr;
extern void(__stdcall* ConsoleEvalStdcall)(const char*) = nullptr;

HRESULT apply_dpi_awareness(const wchar_t* exe_path_str)
{
  if (exe_path_str == nullptr)
  {
    return E_POINTER;
  }

  std::error_code last_error;

  auto exe_path = fs::path(exe_path_str);

  HKEY current_user = nullptr;
  if (::RegOpenCurrentUser(KEY_WRITE, &current_user) == 0)
  {
    std::wstring compat = L"~ HIGHDPIAWARE";
    HKEY compat_key = nullptr;
    if (::RegOpenKeyExW(current_user, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", 0, KEY_WRITE, &compat_key) == 0)
    {
      ::RegSetValueExW(compat_key, exe_path_str, 0, REG_SZ, (BYTE*)compat.data(), compat.size() * 2);
      ::RegCloseKey(compat_key);
    }

    ::RegCloseKey(current_user);
  }

  return S_OK;
}

predefined_string*
  get_predefined_id_tech_2_map_command_line_settings(const wchar_t* base_dir, bool include_zip) noexcept
{
  static std::vector<std::wstring> storage;
  static std::vector<predefined_string> results;

  if (!results.empty())
  {
    return results.data();
  }

  try
  {
    std::error_code errc{};

    if (fs::is_directory(base_dir, errc))
    {
      std::vector<fs::path> pak_files;
      pak_files.reserve(16);

      auto maps_dir = fs::path(base_dir) / "maps";

      for (auto const& dir_entry : std::filesystem::directory_iterator{ base_dir })
      {
        if ((include_zip && dir_entry.path().extension() == L".zip" || include_zip && dir_entry.path().extension() == L".ZIP") || (dir_entry.path().extension() == L".dat" || dir_entry.path().extension() == L".DAT") || dir_entry.path().extension() == L".pak" || dir_entry.path().extension() == L".PAK")
        {
          pak_files.emplace_back(dir_entry.path());
        }
      }

      if (fs::is_directory(maps_dir, errc))
      {
        for (auto const& file_iter : std::filesystem::directory_iterator{ maps_dir })
        {
          if (!file_iter.is_directory() && (file_iter.path().extension() == ".bsp" || file_iter.path().extension() == ".BSP"))
          {
            if (file_iter.path().parent_path() == maps_dir)
            {
              storage.emplace_back(file_iter.path().stem());
            }
            else
            {
              auto parent = fs::relative(file_iter.path().parent_path(), maps_dir);
              std::wstring final_name = (parent / file_iter.path().stem()).wstring();

              while (final_name.contains(fs::path::preferred_separator))
              {
                final_name = final_name.replace(final_name.find(fs::path::preferred_separator), 1, std::wstring(L"/"));
              }

              storage.emplace_back(std::move(final_name));
            }
          }
        }
      }

      storage.reserve(pak_files.size() * pak_files.size());

      std::for_each(pak_files.begin(), pak_files.end(), [](auto& dir_entry) {
        std::any cache;
        std::ifstream stream(dir_entry, std::ios::binary);

        std::optional<siege::platform::resource_reader> reader;

        if (siege::resource::pak::stream_is_supported(stream))
        {
          reader.emplace(siege::resource::pak::pak_resource_reader());
        }
        else if (siege::resource::zip::stream_is_supported(stream))
        {
          reader.emplace(siege::resource::zip::zip_resource_reader());
        }
        else
        {
          return;
        }

        auto contents = reader->get_content_listing(cache, stream, { .archive_path = dir_entry, .folder_path = dir_entry / "maps" });

        for (auto& content : contents)
        {
          if (auto* folder_info = std::get_if<siege::platform::resource_reader::folder_info>(&content); folder_info)
          {
            contents.append_range(reader->get_content_listing(cache, stream, { .archive_path = dir_entry, .folder_path = folder_info->full_path }));
          }
        }

        if (storage.capacity() == 0)
        {
          storage.reserve(contents.size());
        }

        for (auto& content : contents)
        {
          if (std::get_if<siege::platform::folder_info>(&content) != nullptr)
          {
            continue;
          }
          auto& file_info = std::get<siege::platform::file_info>(content);

          if (file_info.filename.extension() == ".bsp" || file_info.filename.extension() == ".BSP")
          {
            auto temp = fs::relative(file_info.folder_path, file_info.archive_path);

            if (temp.string() == "maps" || temp.string().starts_with("maps/") || temp.string().starts_with("maps\\"))
            {
              temp = temp.string().replace(0, 5, "");
            }

            if (temp.string() == "" || temp.string() == "/" || temp.string() == "\\")
            {
              storage.emplace_back(file_info.filename.stem().wstring());
            }
            else
            {
              std::wstring final_name = (temp / file_info.filename.stem()).wstring();

              while (final_name.contains(std::filesystem::path::preferred_separator))
              {
                final_name = final_name.replace(final_name.find(std::filesystem::path::preferred_separator), 1, std::wstring(L"/"));
              }

              storage.emplace_back(std::move(final_name));
            }
          }
        }
      });
    }

    results.emplace_back(predefined_string{
      .label = L"No map",
      .value = L"" });

    for (auto& string : storage)
    {
      results.emplace_back(predefined_string{
        .label = string.c_str(),
        .value = string.c_str() });
    }

    results.emplace_back(predefined_string{});

    return results.data();
  }
  catch (...)
  {
    return nullptr;
  }
}


predefined_string*
  get_predefined_id_tech_3_map_command_line_settings(const wchar_t* base_dir) noexcept
{
  static std::vector<std::wstring> storage;
  static std::vector<predefined_string> results;

  if (!results.empty())
  {
    return results.data();
  }

  try
  {
    std::error_code errc{};

    if (fs::is_directory(base_dir, errc))
    {
      std::vector<fs::path> pak_files;
      pak_files.reserve(16);
      for (auto const& dir_entry : std::filesystem::directory_iterator{ base_dir })
      {
        if (dir_entry.path().extension() == L".zip" || dir_entry.path().extension() == L".ZIP"
            || dir_entry.path().extension() == L".pk3" || dir_entry.path().extension() == L".PK3"
            || dir_entry.path().extension() == L".hwp" || dir_entry.path().extension() == L".HWP")
        {
          pak_files.emplace_back(dir_entry.path());
        }
      }

      std::for_each(pak_files.begin(), pak_files.end(), [](auto& dir_entry) {
        std::any cache;
        std::ifstream stream(dir_entry, std::ios::binary);

        siege::resource::zip::zip_resource_reader reader;

        if (!reader.stream_is_supported(stream))
        {
          return;
        }

        auto contents = reader.get_content_listing(cache, stream, { .archive_path = dir_entry, .folder_path = dir_entry / "maps" });

        for (auto& content : contents)
        {
          if (auto* folder_info = std::get_if<siege::platform::resource_reader::folder_info>(&content); folder_info)
          {
            contents.append_range(reader.get_content_listing(cache, stream, { .archive_path = dir_entry, .folder_path = folder_info->full_path }));
          }
        }

        if (storage.capacity() == 0)
        {
          storage.reserve(contents.size());
        }

        for (auto& content : contents)
        {
          if (std::get_if<siege::platform::folder_info>(&content) != nullptr)
          {
            continue;
          }
          auto& file_info = std::get<siege::platform::file_info>(content);

          if (file_info.filename.extension() == ".bsp" || file_info.filename.extension() == ".BSP")
          {

            auto temp = fs::relative(file_info.folder_path, file_info.archive_path);

            if (temp.string() == "maps")
            {
              temp = "";
            }
            else if (temp.string().starts_with("maps/") || temp.string().starts_with("maps\\"))
            {
              temp = temp.string().replace(0, 5, "");
            }

            if (temp.string() == "" || temp.string() == "/" || temp.string() == "\\")
            {
              storage.emplace_back(file_info.filename.stem().wstring());
            }
            else
            {
              std::wstring final_name = (temp / file_info.filename.stem()).wstring();

              while (final_name.contains(std::filesystem::path::preferred_separator))
              {
                final_name = final_name.replace(final_name.find(std::filesystem::path::preferred_separator), 1, std::wstring(L"/"));
              }

              storage.emplace_back(std::move(final_name));
            }
          }
        }
      });
    }

    results.emplace_back(predefined_string{
      .label = L"No map",
      .value = L"" });

    for (auto& string : storage)
    {
      results.emplace_back(predefined_string{
        .label = string.c_str(),
        .value = string.c_str() });
    }

    results.emplace_back(predefined_string{});

    return results.data();
  }
  catch (...)
  {
    return nullptr;
  }
}

LRESULT CALLBACK dispatch_input_to_game_console(int code, WPARAM wParam, LPARAM lParam, void (*process_movement_keydown)(MSG* message, void (*console_eval)(const char*)), void (*console_eval)(const char*));

void process_quake_movement_keydown(MSG* message, void (*console_eval)(const char*));
void process_quake_3_movement_keydown(MSG* message, void (*console_eval)(const char*));

LRESULT CALLBACK dispatch_input_to_cdecl_quake_2_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_movement_keydown, ConsoleEvalCdecl);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

void do_console_eval_cdecl_quake_1(const char* text)
{
  thread_local std::string temp;
  temp.assign(text);
  temp.append(1, '\n');
  ConsoleEvalCdecl(temp.c_str());
}

LRESULT CALLBACK dispatch_input_to_cdecl_quake_1_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_movement_keydown, do_console_eval_cdecl_quake_1);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

LRESULT CALLBACK dispatch_input_to_cdecl_quake_3_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_3_movement_keydown, do_console_eval_cdecl_quake_1);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

void do_console_eval_stdcall_quake_3(const char* text)
{
  thread_local std::string temp;
  temp.assign(text);
  temp.append(1, '\n');
  ConsoleEvalCdecl(temp.c_str());
}

LRESULT CALLBACK dispatch_input_to_stdcall_quake_3_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalCdecl)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_3_movement_keydown, do_console_eval_stdcall_quake_3);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

void do_console_eval_fastcall(const char* text)
{
  ConsoleEvalFastcall(text);
}

LRESULT CALLBACK dispatch_input_to_fastcall_quake_2_console(int code, WPARAM wParam, LPARAM lParam)
{
  if (ConsoleEvalFastcall)
  {
    return dispatch_input_to_game_console(code, wParam, lParam, process_quake_movement_keydown, do_console_eval_fastcall);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

WORD get_extended_state(BYTE vkey)
{
  std::array<WORD, 256>* extended_states = nullptr;
  auto window = ::GetFocus();
  if (window && ::IsWindowUnicode(window))
  {
    extended_states = (std::array<WORD, 256>*)::GetPropW(window, L"ExtendedVkStates");
  }
  else if (window)
  {
    extended_states = (std::array<WORD, 256>*)::GetPropA(window, "ExtendedVkStates");
  }

  if (extended_states)
  {
    return (*extended_states)[vkey];
  }

  return 0;
}

void process_quake_3_movement_keydown(MSG* message, void (*console_eval)(const char*))
{
  constexpr static std::uint16_t uint_max = -1;
  constexpr static std::uint16_t uint_half = uint_max / 2;
  if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_LEFT)
  {
    auto state = get_extended_state(message->wParam);

    std::stringstream command;
    command << "cl_run " << (state > uint_half ? "1" : "0");
    console_eval(command.str().c_str());
    console_eval("+moveleft");
  }
  else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT)
  {
    auto state = get_extended_state(message->wParam);
    std::stringstream command;
    command << "cl_run " << (state > uint_half ? "1" : "0");
    console_eval(command.str().c_str());
    console_eval("+moveright");
  }
  else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_UP)
  {
    auto state = get_extended_state(message->wParam);
    std::stringstream command;
    command << "cl_run " << (state > uint_half ? "1" : "0");
    console_eval(command.str().c_str());
    console_eval("+forward");
  }
  else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_DOWN)
  {
    auto state = get_extended_state(message->wParam);
    std::stringstream command;
    command << "cl_run " << (state > uint_half ? "1" : "0");
    console_eval(command.str().c_str());
    console_eval("+back");
  }

  if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
    std::stringstream command;
    command << "cl_yawspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+left");
  }
  else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
    std::stringstream command;
    command << "cl_yawspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+right");
  }
  else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_UP)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
    std::stringstream command;
    command << "cl_pitchspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+lookup");
  }
  else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
    std::stringstream command;
    command << "cl_pitchspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+lookdown");
  }
}

void process_quake_movement_keydown(MSG* message, void (*console_eval)(const char*))
{
  constexpr static std::uint16_t uint_max = -1;
  if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_LEFT)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;

    std::stringstream command;
    command << "cl_sidespeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+moveleft");
  }
  else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;
    std::stringstream command;
    command << "cl_sidespeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+moveright");
  }
  else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_UP)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;
    std::stringstream command;
    command << "cl_forwardspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+forward");
  }
  else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_DOWN)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 160;
    std::stringstream command;
    command << "cl_forwardspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+back");
  }

  if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
    std::stringstream command;
    command << "cl_yawspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+left");
  }
  else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
    std::stringstream command;
    command << "cl_yawspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+right");
  }
  else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_UP)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
    std::stringstream command;
    command << "cl_pitchspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+lookup");
  }
  else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN)
  {
    auto state = ((float)get_extended_state(message->wParam) / (float)uint_max) * 100;
    std::stringstream command;
    command << "cl_pitchspeed " << (int)state;
    console_eval(command.str().c_str());
    console_eval("+lookdown");
  }
}

LRESULT CALLBACK dispatch_input_to_game_console(int code, WPARAM wParam, LPARAM lParam, void (*process_movement_keydown)(MSG* message, void (*console_eval)(const char*)), void (*console_eval)(const char*))
{
  if (code == HC_ACTION && wParam == PM_REMOVE)
  {
    auto* message = (MSG*)lParam;

    if (message->message == WM_KEYDOWN)
    {
      process_movement_keydown(message, console_eval);
    }
    else if (message->message == WM_KEYUP)
    {
      if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_LEFT)
      {
        console_eval("-moveleft");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT)
      {
        console_eval("-moveright");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_UP)
      {
        console_eval("-forward");
      }
      else if (message->wParam == VK_GAMEPAD_LEFT_THUMBSTICK_DOWN)
      {
        console_eval("-back");
      }

      if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT)
      {
        console_eval("-left");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT)
      {
        console_eval("-right");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_UP)
      {
        console_eval("-lookup");
      }
      else if (message->wParam == VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN)
      {
        console_eval("-lookdown");
      }
    }
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}
}