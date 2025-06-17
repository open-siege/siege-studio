#ifndef DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
#define DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::vol::three_space
{
  struct rmf_resource_reader : siege::platform::resource_reader
  {
    rmf_resource_reader();
    static bool stream_is_supported(std::istream& stream);

    static std::vector<std::variant<siege::platform::folder_info, siege::platform::file_info>> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);

    static void set_stream_position(std::istream& stream, const siege::platform::file_info& info);

    static void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& outpu);
  };

  struct dyn_resource_reader : siege::platform::resource_reader
  {
    dyn_resource_reader();
    
    static bool stream_is_supported(std::istream& stream);

    static std::vector<std::variant<siege::platform::folder_info, siege::platform::file_info>> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);

    static void set_stream_position(std::istream& stream, const siege::platform::file_info& info);

    static void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output);
  };

  struct vol_resource_reader : siege::platform::resource_reader
  {
    vol_resource_reader();

    static bool stream_is_supported(std::istream& stream);

    static std::vector<std::variant<siege::platform::folder_info, siege::platform::file_info>> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query);

    static void set_stream_position(std::istream& stream, const siege::platform::file_info& info);

    static void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output);
  };
}// namespace siege::resource::vol::three_space

#endif// DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
