#ifndef DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
#define DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include "archive_plugin.hpp"
#include "endian_arithmetic.hpp"

namespace studio::resources::vol::darkstar
{
  enum class compression_type : std::uint8_t
  {
    none,
    rle,
    lz,
    lzh
  };

  struct volume_file_info
  {
    std::string filename;
    std::int32_t size;
    std::optional<std::int32_t> compressed_size;
    compression_type compression_type;
    std::unique_ptr<std::basic_istream<std::byte>> stream;
  };

  void create_vol_file(std::basic_ostream<std::byte>& output, const std::vector<volume_file_info>& files);

  struct vol_file_archive : studio::resources::archive_plugin
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
}// namespace darkstar::vol


#endif//DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
