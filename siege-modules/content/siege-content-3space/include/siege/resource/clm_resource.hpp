
#ifndef DARKSTARDTSCONVERTER_CLM_VOLUME_HPP
#define DARKSTARDTSCONVERTER_CLM_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::clm
{
  bool is_stream_supported(std::istream& stream);
  siege::platform::resource_reader make_resource_reader();
}// namespace siege::resource::clm


#endif// DARKSTARDTSCONVERTER_SEVEN_PAK_VOLUME_HPP