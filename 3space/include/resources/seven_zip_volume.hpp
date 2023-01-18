#ifndef DARKSTARDTSCONVERTER_SEVEN_ZIP_VOLUME_HPP
#define DARKSTARDTSCONVERTER_SEVEN_ZIP_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include "archive_plugin.hpp"
#include "endian_arithmetic.hpp"

namespace studio::resources::zip
{
  struct seven_zip_file_archive : studio::resources::archive_plugin
  {
    static bool is_supported(std::istream& stream);

    bool stream_is_supported(std::istream& stream) const override;
    std::vector<content_info> get_content_listing(std::istream& stream, const listing_query& query) const override;
    void set_stream_position(std::istream& stream, const studio::resources::file_info& info) const override;
    void extract_file_contents(std::istream& stream,
      const studio::resources::file_info& info,
      std::ostream& output,
      std::optional<std::reference_wrapper<batch_storage>> = std::nullopt) const override;
  };

}// namespace studio::resources::zip


#endif// DARKSTARDTSCONVERTER_SEVEN_ZIP_VOLUME_HPP
