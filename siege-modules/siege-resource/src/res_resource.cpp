// STARTREK.RES + SLUS_009.24

// would only be able to list files and potentially identify file headers
// RIFF CDXA fmt data (first 40 bytes)

// SLUS_009.24
// Start 25096 FILMS\CREDITS.STR 
// FILMS\OUTRO3.STR
// End 34292 TRK\KJ_FA.TRK       

// 2352
// File start FF FF FF FF (FF FF FF 00) + 12 bytes
// DSM - 01 00 00 0f 00 04 00 00
// TRK - TREK
// STR - 60 01 01 80 - 2048 sectors
// FRONTEND.OVL - 74 66 07 80 74 66 07 80 74 66 07 80 
// GAME.OVL - CARD FR 00 TYC0 CK 00 VALK 2 00 00 00
// 262 144
// TIM
//constexpr file_tag four_bit_image = platform::to_tag<8>({ 0x10, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 });
//constexpr file_tag eight_bit_image = platform::to_tag<8>({ 0x10, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00 });
//constexpr file_tag sixteen_bit_image = platform::to_tag<8>({ 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 });


#include <array>
#include <siege/platform/shared.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/endian_arithmetic.hpp>
#include <siege/platform/resource.hpp>
#include <siege/resource/res_resource.hpp>

namespace siege::resource::res
{
  namespace endian = siege::platform;

  // actually the number of files in the file
  constexpr static auto riff_tag = platform::to_tag<4>("RIFF");
  constexpr static auto cdxa_tag = platform::to_tag<4>("CDXA");
  constexpr static auto fmt_tag = platform::to_tag<4>({ 'f', 'm', 't', 0x20 });
  constexpr static auto data_tag = platform::to_tag<4>("data");

  struct res_header
  {
    std::array<std::byte, 4> riff_header;
    endian::little_uint32_t riff_size;
    std::array<std::byte, 4> type;
    std::array<std::byte, 4> format_header;
    endian::little_uint32_t format_size;
    std::array<std::byte, 16> format_data;
    std::array<std::byte, 4> data_header;
    endian::little_uint32_t data_size;
  };


  bool res_resource_reader::is_supported(std::istream& stream)
  {
    auto path = siege::platform::get_stream_path(stream);

    if (path)
    {
      platform::istream_pos_resetter resetter(stream);

      if (path->extension() == ".res" || path->extension() == ".RES")
      {
        res_header header{};
        stream.read((char*)&header, sizeof(res_header));

        if (header.riff_header == riff_tag && header.type == cdxa_tag && header.format_header == fmt_tag && header.data_header == data_tag)
        {
          return true;
        }
      }
    }

    return false;
  }

  bool res_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<res_resource_reader::content_info> res_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<res_resource_reader::content_info> results;


    res_header header{};
    stream.read((char*)&header, sizeof(res_header));

    /*
        std::filesystem::path filename;
    std::size_t offset;
    std::size_t size;
    std::optional<std::size_t> compressed_size;
    siege::platform::compression_type compression_type;
    std::filesystem::path folder_path;
    std::filesystem::path archive_path;
    std::any metadata;

    */
    if (header.riff_header == riff_tag && header.type == cdxa_tag && header.format_header == fmt_tag && header.data_header == data_tag)
    {
      results.emplace_back(res_resource_reader::file_info{
        .filename = std::filesystem::path(query.archive_path.filename()).replace_extension(".bin"),
        .offset = std::size_t(stream.tellg()),
        .size = std::size_t(header.data_size),
        .folder_path = query.folder_path,
        .archive_path = query.archive_path,
      });
    }

    return results;
  }

  void res_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void res_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>>) const
  {
    std::vector<char> data;
    data.resize(2048);
    for (auto i = 0; i < info.size; i += 2352)
    {
      stream.seekg(info.offset + i, std::ios::beg);
      stream.seekg(24, std::ios::cur);
      stream.read(data.data(), data.size());
      output.write(data.data(), data.size());
      stream.seekg(4 + 276, std::ios::cur);
    }
  }
}// namespace siege::resource::res