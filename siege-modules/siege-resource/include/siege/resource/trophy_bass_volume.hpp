#ifndef DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
#define DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP

#include <siege/platform/archive_plugin.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::vol::trophy_bass
{
  struct rbx_file_archive final : siege::platform::archive_plugin
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

  struct tbv_file_archive final : siege::platform::archive_plugin
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

#endif//DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
