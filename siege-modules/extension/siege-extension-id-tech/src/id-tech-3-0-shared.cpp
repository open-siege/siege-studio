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
}