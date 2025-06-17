
#ifndef DARKSTARDTSCONVERTER_PAK_VOLUME_HPP
#define DARKSTARDTSCONVERTER_PAK_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::pak
{
  bool stream_is_supported(std::istream& stream);
  std::vector<platform::content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);
  void set_stream_position(std::istream& stream, const siege::platform::file_info& info);
  void extract_file_contents(std::any& cache,
    std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output);

  struct pak_resource_reader : siege::platform::resource_reader
  {
    pak_resource_reader() : resource_reader{ pak::stream_is_supported, pak::get_content_listing, pak::set_stream_position, pak::extract_file_contents }
    {
    }
  };
}// namespace siege::resource::pak


#endif// DARKSTARDTSCONVERTER_SEVEN_PAK_VOLUME_HPP