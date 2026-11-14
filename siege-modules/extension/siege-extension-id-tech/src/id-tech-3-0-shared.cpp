module;
#include <cassert>
export module siege.extension.id_tech.v3_0;

import std;

import siege.configuration.id_tech;
import siege.configuration.shared;
import siege.extension.id_tech;
import siege.extension.shared;
import siege.platform;
import siege.platform.shared;
import siege.resource.cab;
import siege.resource.iso;
import siege.resource.pak;
import siege.resource.zip;

using vk = siege::platform::vkey;

extern "C" {

std::errc is_id_tech_3_input_mapping_valid(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping)
{
  using namespace siege::platform;
  if (mapping == nullptr)
  {
    return std::errc::bad_address;
  }

  if (caps == nullptr)
  {
    return std::errc::bad_address;
  }

  if (mapping->mapping_size < sizeof(input_mapping_ex))
  {
    return std::errc::bad_message;
  }

  if (caps->caps_size < sizeof(hardware_context_caps))
  {
    return std::errc::bad_message;
  }

  if (mapping->vkey == vk::left_shift || mapping->vkey == vk::right_shift)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == vk::left_control || mapping->vkey == vk::right_control)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == vk::left_alt || mapping->vkey == vk::right_alt)
  {
    return std::errc::not_supported;
  }

  if (caps->context == hardware_context::mouse)
  {
    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 0
        && !(mapping->action_name == std::array<char, 32>{ "+left" } || mapping->action_name == std::array<char, 32>{ "+right" }))
    {
      return std::errc::not_supported;
    }
    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 1
        && !(mapping->action_name == std::array<char, 32>{ "+lookup" } || mapping->action_name == std::array<char, 32>{ "+lookdown" }))
    {
      return std::errc::not_supported;
    }
    return std::errc{};
  }

  if (is_for_controller(caps->context))
  {
    if (caps->hardware_index != 0)
    {
      return std::errc::not_supported;
    }

    if (caps->axis_count < 6 && mapping->action_name == std::array<char, 32>{})
    {
      return std::errc{};
    }

    if (mapping->hardware_index <= 3 && mapping->action_name == std::array<char, 32>{})
    {
      return std::errc{};
    }

    if (caps->axis_count >= 6)
    {
      if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 4
          && !(mapping->action_name == std::array<char, 32>{ "+left" } || mapping->action_name == std::array<char, 32>{ "+right" }))
      {
        return std::errc::not_supported;
      }
      if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 5
          && !(mapping->action_name == std::array<char, 32>{ "+lookup" } || mapping->action_name == std::array<char, 32>{ "+lookdown" }))
      {
        return std::errc::not_supported;
      }
    }
    else if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index > 3)
    {
      return std::errc::not_supported;
    }
  }

  return std::errc{};
}

std::errc is_moh_input_mapping_valid(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping)
{
  using namespace siege::platform;
  if (mapping == nullptr)
  {
    return std::errc::bad_address;
  }

  if (caps == nullptr)
  {
    return std::errc::bad_address;
  }

  if (mapping->mapping_size < sizeof(input_mapping_ex))
  {
    return std::errc::bad_message;
  }

  if (caps->caps_size < sizeof(hardware_context_caps))
  {
    return std::errc::bad_message;
  }

  if (mapping->vkey == vk::shift)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == vk::control)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == vk::alt)
  {
    return std::errc::not_supported;
  }

  if (caps->context == hardware_context::mouse)
  {
    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 0
        && (mapping->action_name == std::array<char, 32>{ "+left" } || mapping->action_name == std::array<char, 32>{ "+right" }))
    {
      return std::errc::not_supported;
    }
    if (mapping->hardware_input_type == controller_input_type::axis && mapping->hardware_index == 1
        && (mapping->action_name == std::array<char, 32>{ "+lookup" } || mapping->action_name == std::array<char, 32>{ "+lookdown" }))
    {
      return std::errc::not_supported;
    }
    return std::errc{};
  }

  if (is_for_controller(caps->context))
  {
    if (caps->hardware_index != 0)
    {
      return std::errc::not_supported;
    }

    if (mapping->hardware_input_type == controller_input_type::axis)
    {
      if (mapping->hardware_index == 0
          && !(mapping->action_name == std::array<char, 32>{ "+moveleft" } || mapping->action_name == std::array<char, 32>{ "+moveright" }))
      {
        return std::errc::not_supported;
      }

      if (mapping->hardware_index == 1
          && !(mapping->action_name == std::array<char, 32>{ "+forward" } || mapping->action_name == std::array<char, 32>{ "+back" }))
      {
        return std::errc::not_supported;
      }

      if (mapping->hardware_index == 2
          && !(mapping->action_name == std::array<char, 32>{ "+left" } || mapping->action_name == std::array<char, 32>{ "+right" }))
      {
        return std::errc::not_supported;
      }

      if (mapping->hardware_index == 3
          && !(mapping->action_name == std::array<char, 32>{ "+lookup" } || mapping->action_name == std::array<char, 32>{ "+lookdown" }))
      {
        return std::errc::not_supported;
      }
    }
  }

  return std::errc{};
}

using game_command_line_caps = siege::platform::game_command_line_caps;
namespace fs = std::filesystem;
using namespace std::literals;
extern game_command_line_caps command_line_caps;

// TODO
// Specify controller cfg in the command line
const wchar_t** format_command_line_ex(const siege::platform::game_command_line_args_ex* args, std::uint32_t* new_size)
{
  if (!args)
  {
    return nullptr;
  }

  if (!new_size)
  {
    return nullptr;
  }

  static std::vector<std::wstring> string_args;
  string_args.clear();

  for (auto& setting : std::span(args->string_settings, args->string_settings_size))
  {
    if (!setting.name)
    {
      continue;
    }

    if (!setting.value)
    {
      continue;
    }

    if (!setting.value[0])
    {
      continue;
    }

    if (std::wstring_view(setting.name) == command_line_caps.ip_connect_setting)
    {
      string_args.emplace_back(L"+connect");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) == L"map")
    {
      string_args.emplace_back(L"+map");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) == L"exec")
    {
      string_args.emplace_back(L"+exec");
      string_args.emplace_back(setting.value);
    }
    else
    {
      string_args.emplace_back(L"+set");
      string_args.emplace_back(setting.name);
      string_args.emplace_back(setting.value);
    }
  }

  for (auto& setting : std::span(args->int_settings, args->int_settings_size))
  {
    if (!setting.name)
    {
      continue;
    }

    try
    {
      auto name_str = std::wstring_view(setting.name);

      string_args.emplace_back(L"+set");
      string_args.emplace_back(name_str);
      string_args.emplace_back(std::to_wstring(setting.value));
    }
    catch (...)
    {
    }
  }

  for (auto& flag : std::span(args->flags, args->flags_size))
  {
    if (!flag)
    {
      continue;
    }

    string_args.emplace_back(flag);
  }

  if (string_args.empty())
  {
    return nullptr;
  }

  static std::vector<const wchar_t*> raw_args;
  raw_args.resize(string_args.size());
  *new_size = (std::uint32_t)string_args.size();

  std::transform(string_args.begin(), string_args.end(), raw_args.begin(), [](const std::wstring& value) {
    return value.c_str();
  });

  return raw_args.data();
}
}

export void unbind_joystick_for_id_tech_3_0_config(siege::configuration::text_game_config& config, mapping_context context = q3_mapping_context{})
{
  assert(context.index_to_axis != nullptr);
  assert(context.index_to_button != nullptr);
  using siege::configuration::key_type;
  constexpr std::size_t after_comment = 1;

  config.upsert(key_type({ "unbind", "JOY17" }), key_type{}, after_comment);
  config.upsert(key_type({ "unbind", "JOY16" }), key_type{}, after_comment);
  config.upsert(key_type({ "unbind", "JOY18" }), key_type{}, after_comment);
  config.upsert(key_type({ "unbind", "JOY19" }), key_type{}, after_comment);

  // dpad
  config.upsert(key_type({ "unbind", "JOY24" }), key_type{}, after_comment);
  config.upsert(key_type({ "unbind", "JOY25" }), key_type{}, after_comment);
  config.upsert(key_type({ "unbind", "JOY27" }), key_type{}, after_comment);
  config.upsert(key_type({ "unbind", "JOY26" }), key_type{}, after_comment);
  for (auto i = 0; i < 15; ++i)
  {
    auto index = context.index_to_button(static_cast<std::uint16_t>(i));
    assert(index.has_value());
    config.upsert(key_type({ "unbind", *index }), key_type{}, after_comment);
  }
}

export std::errc init_id_tech_3_0_game_settings(siege::platform::common_game_settings* args, const game_settings_files& files)
{
  if (args == nullptr)
  {
    return std::errc::bad_address;
  }

  auto append_from_config = [&](siege::configuration::text_game_config& config) {
    for (auto key : config.keys())
    {
      if (key.at(0) == "bind" || key.at(0) == "unbind" || key.at(0) == "unbindall" || key.at(0) == "alias" || key.at(0) == "//")
      {
        continue;
      }

      auto real_key = key.at(0) == "set" || key.at(0) == "seta" ? key.at(1) : key.at(0);

      if (real_key.empty())
      {
        continue;
      }

      for (auto& entry : config.entries_for(key))
      {
        if (!args->append_setting(real_key, entry.value.at(0)))
        {
          return false;
        }
      }
    }

    return true;
  };

  if (auto config = load_config_from_pk3(
        files.packed_default_cfg,
        std::wstring(files.packed_default_cfg_pak),
        std::wstring(files.packed_default_cfg_pak_folder)))
  {
    if (!append_from_config(*config))
    {
      return std::errc{};
    }
  }

  if (auto config = load_config_from_file(files.loose_user_cfg))
  {
    if (!append_from_config(*config))
    {
      return std::errc{};
    }
  }

  return std::errc{};
}
