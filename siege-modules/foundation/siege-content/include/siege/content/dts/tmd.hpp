#ifndef SIEGE_TMD_HPP
#define SIEGE_TMP_HPP

#include <siege/platform/endian_arithmetic.hpp>

namespace siege::content::tmd
{
  namespace endian = siege::platform;

  struct tmd_vertex
  {
    endian::little_uint16_t x;
    endian::little_uint16_t y;
    endian::little_uint16_t z;
    endian::little_uint16_t padding;
  };
}// namespace siege::content::tmd

#endif// !SIEGE_TMD_HPP
