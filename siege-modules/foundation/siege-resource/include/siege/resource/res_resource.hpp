
#ifndef SIEGE_RES_RESOURCE_HPP
#define SIEGE_RES_RESOURCE_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::res
{
  struct res_resource_reader final : siege::platform::resource_reader
  {
    static bool is_supported(std::istream& stream);

    bool stream_is_supported(std::istream& stream) const override;
    std::vector<content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query) const override;
    void set_stream_position(std::istream& stream, const siege::platform::file_info& info) const override;
    void extract_file_contents(std::any& cache, std::istream& stream,
      const siege::platform::file_info& info,
      std::ostream& output) const override;
  };

}// namespace siege::resource::rsc


#endif// SIEGE_RES_RESOURCE_HPP