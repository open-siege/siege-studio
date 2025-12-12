#ifndef DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
#define DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::vol::trophy_bass
{
  bool is_stream_rbx(std::istream& stream);
  siege::platform::resource_reader make_rbx_resource_reader();

  bool is_stream_tbv(std::istream& stream);
  siege::platform::resource_reader make_tbv_resource_reader();
}// namespace siege::resource::vol::trophy_bass

#endif// DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
