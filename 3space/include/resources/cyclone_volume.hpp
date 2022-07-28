#ifndef DARKSTARDTSCONVERTER_CYCLONE_VOLUME_HPP
#define DARKSTARDTSCONVERTER_CYCLONE_VOLUME_HPP

#include "archive_plugin.hpp"
#include "endian_arithmetic.hpp"

namespace studio::resources::cln
{
  struct cln_file_archive : studio::resources::archive_plugin
  {
    static bool is_supported(std::basic_istream<std::byte>& stream);

    bool stream_is_supported(std::basic_istream<std::byte>& stream) const override;

    std::vector<content_info> get_content_listing(std::basic_istream<std::byte>& stream, const listing_query& query) const override;

    void set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const override;

    void extract_file_contents(std::basic_istream<std::byte>& stream,
      const studio::resources::file_info& info,
      std::basic_ostream<std::byte>& output,
      std::optional<std::reference_wrapper<batch_storage>> = std::nullopt) const override;
  };
}// namespace trophy_bass::vol

#endif//DARKSTARDTSCONVERTER_CYCLONE_VOLUME_HPP
