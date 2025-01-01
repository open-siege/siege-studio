#ifndef DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
#define DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include <siege/platform/resource.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::resource::vol::darkstar
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
    darkstar::compression_type compression_type;
    std::unique_ptr<std::istream> stream;
  };

  void create_vol_file(std::ostream& output, const std::vector<volume_file_info>& files);

  struct vol_resource_reader final : siege::platform::resource_reader
  {
    static bool is_supported(std::istream& stream);

    bool stream_is_supported(std::istream& stream) const override;
    std::vector<content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query) const override;
    void set_stream_position(std::istream& stream, const siege::platform::file_info& info) const override;
    void extract_file_contents(std::any& cache, std::istream& stream,
      const siege::platform::file_info& info,
      std::ostream& output) const override;
  };
}// namespace darkstar::vol


#endif//DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
