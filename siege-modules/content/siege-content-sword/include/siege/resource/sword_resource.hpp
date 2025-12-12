#ifndef DARKSTARDTSCONVERTER_SWORD_VOLUME_HPP
#define DARKSTARDTSCONVERTER_SWORD_VOLUME_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::atd
{
  bool is_stream_supported(std::istream& stream);
  siege::platform::resource_reader make_resource_reader();
}// namespace siege::resource::atd

#endif// DARKSTARDTSCONVERTER_SWORD_VOLUME_HPP
