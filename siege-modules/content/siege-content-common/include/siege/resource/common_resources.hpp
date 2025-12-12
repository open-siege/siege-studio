#ifndef DARKSTARDTSCONVERTER_ZIP_VOLUME_HPP
#define DARKSTARDTSCONVERTER_ZIP_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::zip
{
  bool is_stream_zip(std::istream& stream);
  bool is_stream_7zip(std::istream& stream);
  siege::platform::resource_reader make_zip_resource_reader();
  siege::platform::resource_reader make_7zip_resource_reader();
}// namespace siege::resource::zip

namespace siege::resource::cab
{
  bool is_stream_supported(std::istream& stream);
  siege::platform::resource_reader make_resource_reader();
}// namespace siege::resource::cab

namespace siege::resource::iso
{
  bool is_stream_supported(std::istream& stream);
  siege::platform::resource_reader make_resource_reader();
}// namespace siege::resource::iso

#endif// DARKSTARDTSCONVERTER_ZIP_VOLUME_HPP
