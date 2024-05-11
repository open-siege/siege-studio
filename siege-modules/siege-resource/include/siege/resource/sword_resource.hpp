#ifndef DARKSTARDTSCONVERTER_SWORD_VOLUME_HPP
#define DARKSTARDTSCONVERTER_SWORD_VOLUME_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::atd
{
  struct atd_resource_reader final : siege::platform::resource_reader
  {
    static bool is_supported(std::istream& stream);

    bool stream_is_supported(std::istream& stream) const override;

    std::vector<content_info> get_content_listing(std::istream& stream, const platform::listing_query& query) const override;

    void set_stream_position(std::istream& stream, const siege::platform::file_info& info) const override;

    void extract_file_contents(std::istream& stream,
      const siege::platform::file_info& info,
      std::ostream& output,
      std::optional<std::reference_wrapper<platform::batch_storage>> = std::nullopt) const override;
  };
}// namespace trophy_bass::vol

#endif//DARKSTARDTSCONVERTER_SWORD_VOLUME_HPP
