#ifndef OPEN_SIEGE_WINMM_HPP
#define OPEN_SIEGE_WINMM_HPP

#include <array>
#include <utility>

namespace winmm
{
  std::array<std::pair<void**, void*>, 5> GetWinmmDetours();
}

#endif// OPEN_SIEGE_WINMM_HPP
