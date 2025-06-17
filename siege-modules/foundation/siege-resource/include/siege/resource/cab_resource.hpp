#ifndef DARKSTARDTSCONVERTER_CAB_VOLUME_HPP
#define DARKSTARDTSCONVERTER_CAB_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::cab
{
  bool stream_is_supported(std::istream& stream);
  std::vector<platform::content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);
  void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output);

  struct cab_resource_reader : siege::platform::resource_reader
  {
    cab_resource_reader() : resource_reader{ cab::stream_is_supported, cab::get_content_listing, nullptr, cab::extract_file_contents }
    {
    }
  };
}// namespace siege::resource::cab


#endif// DARKSTARDTSCONVERTER_CAB_VOLUME_HPP