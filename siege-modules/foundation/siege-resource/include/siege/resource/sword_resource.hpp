#ifndef DARKSTARDTSCONVERTER_SWORD_VOLUME_HPP
#define DARKSTARDTSCONVERTER_SWORD_VOLUME_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::atd
{
  struct atd_resource_reader final : siege::platform::resource_reader
  {
    atd_resource_reader();
    static bool stream_is_supported(std::istream& stream);

    static std::vector<content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);

    static void set_stream_position(std::istream& stream, const siege::platform::file_info& info);

    static void extract_file_contents(std::any& cache, std::istream& stream,
      const siege::platform::file_info& info,
      std::ostream& output);
  };
}// namespace trophy_bass::vol

#endif//DARKSTARDTSCONVERTER_SWORD_VOLUME_HPP
