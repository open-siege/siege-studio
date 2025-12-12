
#ifndef SIEGE_RSC_VOLUME_HPP
#define SIEGE_RSC_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::rsc
{
  bool is_stream_supported(std::istream& stream);
  siege::platform::resource_reader make_resource_reader();
}// namespace siege::resource::rsc


#endif// SIEGE_RSC_VOLUME_HPP