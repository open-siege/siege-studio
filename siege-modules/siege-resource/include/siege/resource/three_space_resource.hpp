#ifndef DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
#define DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::vol::three_space
{
  struct rmf_resource_reader final : siege::platform::resource_reader
  {
    static bool is_supported(std::istream& stream);

    bool stream_is_supported(std::istream& stream) const override;

    std::vector<std::variant<siege::platform::folder_info, siege::platform::file_info>> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query) const override;

    void set_stream_position(std::istream& stream, const siege::platform::file_info& info) const override;

    void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& outpu) const override;
  };

  struct dyn_resource_reader final : siege::platform::resource_reader
  {
    static bool is_supported(std::istream& stream);

    bool stream_is_supported(std::istream& stream) const override;

    std::vector<std::variant<siege::platform::folder_info, siege::platform::file_info>> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query) const override;

    void set_stream_position(std::istream& stream, const siege::platform::file_info& info) const override;

    void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output) const override;
  };

  struct vol_resource_reader final : siege::platform::resource_reader
  {
    static bool is_supported(std::istream& stream);

    bool stream_is_supported(std::istream& stream) const override;

    std::vector<std::variant<siege::platform::folder_info, siege::platform::file_info>> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query) const override;

    void set_stream_position(std::istream& stream, const siege::platform::file_info& info) const override;

    void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output) const override;
  };
}// namespace siege::resource::vol::three_space

#endif// DARKSTARDTSCONVERTER_THREE_SPACE_VOLUME_HPP
