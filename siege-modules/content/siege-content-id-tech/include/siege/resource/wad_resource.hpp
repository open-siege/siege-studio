
#ifndef DARKSTARDTSCONVERTER_WAD_VOLUME_HPP
#define DARKSTARDTSCONVERTER_WAD_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::wad
{
  bool is_stream_supported(std::istream& stream);
  siege::platform::resource_reader make_resource_reader();
}// namespace siege::resource::wad


#endif// DARKSTARDTSCONVERTER_WAD_VOLUME_HPP