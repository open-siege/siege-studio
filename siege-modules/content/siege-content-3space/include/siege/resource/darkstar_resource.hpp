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

  bool is_stream_supported(std::istream& stream);
  siege::platform::resource_reader make_resource_reader();

}// namespace siege::resource::vol::darkstar


#endif// DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
