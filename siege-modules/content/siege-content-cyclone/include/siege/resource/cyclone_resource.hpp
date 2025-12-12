#ifndef DARKSTARDTSCONVERTER_CYCLONE_VOLUME_HPP
#define DARKSTARDTSCONVERTER_CYCLONE_VOLUME_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::cln
{
  bool is_stream_supported(std::istream& stream);
  siege::platform::resource_reader make_resource_reader();
}// namespace trophy_bass::vol

#endif//DARKSTARDTSCONVERTER_CYCLONE_VOLUME_HPP
