// CWG - Colony Wars Geometry from Colony Wars - Red Sun

module;
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>

export module siege.content.bnd;

import std;

namespace siege::content::cwg
{
  namespace endian = siege::platform;

  export bool is_cwg(std::istream& stream)
  {
    return false;
  }

}// namespace siege::content::cwg