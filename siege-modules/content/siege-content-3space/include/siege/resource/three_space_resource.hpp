#ifndef DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
#define DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::vol::three_space
{
  bool is_stream_rmf(std::istream& stream);
  siege::platform::resource_reader make_rmf_resource_reader();

  bool is_stream_dyn(std::istream& stream);
  siege::platform::resource_reader make_dyn_resource_reader();

  bool is_stream_vol(std::istream& stream);
  siege::platform::resource_reader make_vol_resource_reader();
}// namespace siege::resource::vol::three_space

#endif// DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
