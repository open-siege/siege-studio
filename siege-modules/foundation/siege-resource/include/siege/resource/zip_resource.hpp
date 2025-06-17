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
  bool stream_is_supported(std::istream& stream);
  std::vector<platform::content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);
  void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output);


  struct zip_resource_reader : siege::platform::resource_reader
  {
    zip_resource_reader() : resource_reader{ zip::stream_is_supported, zip::get_content_listing, nullptr, zip::extract_file_contents }
    {
    }
  };
}// namespace siege::resource::zip


#endif// DARKSTARDTSCONVERTER_ZIP_VOLUME_HPP
