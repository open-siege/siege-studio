#ifndef DARKSTAR_SYSTEM_HPP
#define DARKSTAR_SYSTEM_HPP

#include <array>
#include <utility>

namespace core
{
  std::array<std::pair<void**, void*>, 1> GetSystemDetours();
}

#endif