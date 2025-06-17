#ifndef DARKSTARDTSCONVERTER_ISO_VOLUME_HPP
#define DARKSTARDTSCONVERTER_ISO_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::iso
{
  bool stream_is_supported(std::istream& stream);
  std::vector<platform::content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);
  void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output);

  struct iso_resource_reader : siege::platform::resource_reader
  {
    iso_resource_reader() : resource_reader{ iso::stream_is_supported, iso::get_content_listing, nullptr, iso::extract_file_contents }
    {
    }
  };

}// namespace siege::resource::zip


#endif// DARKSTARDTSCONVERTER_ISO_VOLUME_HPP
