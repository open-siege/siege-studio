#ifndef DARKSTARDTSCONVERTER_SEVEN_ZIP_VOLUME_HPP
#define DARKSTARDTSCONVERTER_SEVEN_ZIP_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::zip
{
  struct seven_zip_resource_reader final : siege::platform::resource_reader
  {
    seven_zip_resource_reader();
    static bool stream_is_supported(std::istream& stream);
    static std::vector<content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);
    static void extract_file_contents(std::any& cache, std::istream& stream,
      const siege::platform::file_info& info,
      std::ostream& output);
  };

}// namespace siege::resource::zip


#endif// DARKSTARDTSCONVERTER_SEVEN_ZIP_VOLUME_HPP
