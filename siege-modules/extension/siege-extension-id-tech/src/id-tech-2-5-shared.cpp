#include <cstring>
#include <cstdint>
#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>
#include <thread>
#include <string_view>
#include <fstream>
#include <cassert>
#include <siege/platform/win/file.hpp>
#include <siege/platform/win/window_module.hpp>
#include <siege/resource/pak_resource.hpp>
#include <siege/resource/common_resources.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>

#include "id-tech-shared.hpp"


extern "C" {

std::errc is_id_tech_2_5_input_mapping_valid(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping)
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

  if (mapping->vkey == VK_LSHIFT || mapping->vkey == VK_RSHIFT)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == VK_LCONTROL || mapping->vkey == VK_RCONTROL)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == VK_LMENU || mapping->vkey == VK_RMENU)
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

    if (mapping->action_name == std::array<char, 32>{})
    {
      return std::errc{};
    }

    constexpr static std::array<std::array<char, 32>, 10> axis_only{ {
      "+forward",
      "+back",
      "+left",
      "+right",
      "+moveleft",
      "+moveright",
      "+lookup",
      "+lookdown",
      "+moveup",
      "+movedown",
    } };


    if (!std::ranges::any_of(axis_only, [&](auto& item) { return item == mapping->action_name; }) && mapping->hardware_input_type == controller_input_type::axis)
    {
      return std::errc::not_supported;
    }
  }

  return std::errc{};
}

std::errc is_sof_input_mapping_valid(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping)
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

  if (mapping->vkey == VK_SHIFT)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == VK_CONTROL)
  {
    return std::errc::not_supported;
  }

  if (mapping->vkey == VK_MENU)
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

    if (mapping->action_name == std::array<char, 32>{})
    {
      return std::errc{};
    }

    constexpr static std::array<std::array<char, 32>, 10> axis_only{ {
      "+forward",
      "+back",
      "+left",
      "+right",
      "+moveleft",
      "+moveright",
      "+lookup",
      "+lookdown",
      "+moveup",
      "+movedown",
    } };


    if (!std::ranges::any_of(axis_only, [&](auto& item) { return item == mapping->action_name; }) && mapping->hardware_input_type == controller_input_type::axis)
    {
      return std::errc::not_supported;
    }
  }

  return std::errc{};
}

using game_command_line_caps = siege::platform::game_command_line_caps;
using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
namespace fs = std::filesystem;
extern game_command_line_caps command_line_caps;
using namespace std::literals;

// TODO
// Copy glide files to the game folder - use to-be-implemented shared list of detected items
const wchar_t** format_command_line(const siege::platform::game_command_line_args* args, std::uint32_t* new_size)
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

  // Should always be first or else hosting won't work
  auto connect_iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) {
    return setting.name && setting.value && setting.value[0] != '\0' && std::wstring_view(setting.name) == command_line_caps.ip_connect_setting;
  });

  auto deathmatch_iter = std::find_if(args->int_settings.begin(), args->int_settings.end(), [](auto& setting) {
    return setting.name && setting.value && setting.name == L"deathmatch"sv;
  });

  auto map_iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) {
    return setting.name && setting.value && setting.name == L"map"sv;
  });

  if (connect_iter != args->string_settings.end() && map_iter != args->string_settings.end() && deathmatch_iter == args->int_settings.end())
  {
    string_args.emplace_back(L"+set");
    string_args.emplace_back(L"deathmatch");
    string_args.emplace_back(L"1");
  }

  if (connect_iter != args->string_settings.end())
  {
    string_args.emplace_back(L"+connect");
    string_args.emplace_back(connect_iter->value);
  }

  for (auto& setting : args->string_settings)
  {
    if (!setting.name)
    {
      continue;
    }

    if (std::wstring_view(setting.name) == command_line_caps.ip_connect_setting)
    {
      continue;
    }

    if (std::wstring_view(setting.name) == command_line_caps.preferred_exe_setting)
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

    if (std::wstring_view(setting.name) == L"map")
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

  for (auto& setting : args->int_settings)
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

  for (auto& flag : args->flags)
  {
    if (!flag)
    {
      continue;
    }

    string_args.emplace_back(flag);
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