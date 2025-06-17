
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
  struct rsc_resource_reader : siege::platform::resource_reader
  {
    rsc_resource_reader();
    static bool stream_is_supported(std::istream& stream);
    static std::vector<content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);
    static void set_stream_position(std::istream& stream, const siege::platform::file_info& info);
    static void extract_file_contents(std::any&, std::istream& stream, const siege::platform::file_info& info, std::ostream& output);
  };

}// namespace siege::resource::rsc


#endif// SIEGE_RSC_VOLUME_HPP