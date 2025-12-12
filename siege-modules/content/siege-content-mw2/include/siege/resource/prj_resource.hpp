
#ifndef SIEGE_PRJ_RESOURCE_HPP
#define SIEGE_PRJ_RESOURCE_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::prj
{
  bool is_stream_supported(std::istream& stream);
  siege::platform::resource_reader make_resource_reader();
}// namespace siege::resource::prj


#endif// SIEGE_PRJ_RESOURCE_HPP