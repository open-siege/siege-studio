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

std::errc is_id_tech_2_0_input_mapping_valid(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping)
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

  if (mapping->context == hardware_context::mouse)
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
      "+jump",
      "+movedown",
    } };

    if (!std::ranges::any_of(axis_only, [&](auto& item) { return item == mapping->action_name; }) && mapping->hardware_input_type == siege::platform::controller_input_type::axis)
    {
      return std::errc::not_supported;
    }
  }

  return std::errc{};
}
}