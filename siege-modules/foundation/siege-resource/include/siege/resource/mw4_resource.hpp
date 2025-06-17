
#ifndef SIEGE_MW4_RESOURCE_HPP
#define SIEGE_MW4_RESOURCE_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::mw4
{
  struct mw4_resource_reader : siege::platform::resource_reader
  {
    mw4_resource_reader();
    static bool stream_is_supported(std::istream& stream);
    static std::vector<content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);
    static void set_stream_position(std::istream& stream, const siege::platform::file_info& info);
    static void extract_file_contents(std::any& cache, std::istream& stream,
      const siege::platform::file_info& info,
      std::ostream& output);
  };

}// namespace siege::resource::mw4


#endif// SIEGE_MW4_RESOURCE_HPP