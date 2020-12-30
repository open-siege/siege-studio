#ifndef DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
#define DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>

#include "archive.hpp"
#include "endian_arithmetic.hpp"

namespace darkstar::vol
{
  namespace endian = boost::endian;
  using namespace std::literals;

  // TODO add some checks for these items
  constexpr auto vol_index_tag = "voli"sv;
  constexpr auto vol_string_tag = "vols"sv;
  constexpr auto vol_block_tag = "vblk"sv;

  using file_tag = std::array<std::byte, 4>;

  constexpr file_tag to_tag(const std::array<std::uint8_t, 4> values)
  {
    file_tag result{};

    for (auto i = 0u; i < values.size(); i++)
    {
      result[i] = std::byte{ values[i] };
    }
    return result;
  }

  constexpr auto vol_file_tag = to_tag({ ' ', 'V', 'O', 'L' });
  constexpr auto alt_vol_file_tag = to_tag({ 'P', 'V', 'O', 'L' });
  constexpr auto old_vol_file_tag = to_tag({ 'V', 'O', 'L', ' ' });

  enum class compression_type : std::uint8_t
  {
    none,
    rle,
    lz,
    lzh
  };

  enum class volume_version
  {
    three_space_vol,
    darkstar_pvol,
    darkstar_vol
  };

  struct file_info
  {
    std::string filename;
    std::uint32_t offset;
    std::uint32_t size;
    compression_type compression_type;
  };

  struct volume_header
  {
    std::array<std::byte, 4> file_tag;
    endian::little_uint32_t footer_offset;
  };

  struct old_volume_header
  {
    std::array<std::byte, 4> file_tag;
    endian::little_uint24_t footer_offset;
    std::byte padding;
  };

  static_assert(sizeof(volume_header) == sizeof(old_volume_header));

  struct normal_footer
  {
    std::array<std::byte, 4> string_header_tag;
    endian::little_uint32_t dummy2;
    std::array<std::byte, 4> dummy3;
    endian::little_uint32_t dummy4;
    std::array<std::byte, 4> dummy5;
    endian::little_uint32_t file_list_size;
  };

  struct alternative_footer
  {
    std::array<std::byte, 4> string_header_tag;
    endian::little_uint32_t file_list_size;
  };

  struct old_footer
  {
    std::array<std::byte, 4> header_tag;
    endian::little_uint24_t header_size;
    std::byte padding;
    std::array<std::byte, 4> string_header_tag;
    endian::little_uint24_t buffer_size;
    std::byte padding2;
    endian::little_uint24_t file_list_size;
    std::byte padding3;
  };

  static_assert(sizeof(alternative_footer) * 2 + sizeof(std::array<std::byte, 4>)
                == sizeof(old_footer));

  struct file_index_header
  {
    std::array<std::byte, 4> index_tag;
    endian::little_uint32_t index_size;
  };

  struct file_header
  {
    endian::little_uint32_t id;
    endian::little_uint32_t name_empty_space;
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
    compression_type compression_type;
  };

  static_assert(sizeof(file_header) == sizeof(std::array<std::byte, 17>));

  struct old_file_header
  {
    endian::little_uint32_t id;
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
    std::uint8_t padding;
    std::uint8_t compression_type;
  };

  static_assert(sizeof(old_file_header) == sizeof(std::array<std::byte, 14>));

  std::tuple<volume_version, std::size_t, std::optional<std::size_t>> get_file_list_offsets(std::basic_istream<std::byte>& raw_data);

  std::pair<volume_version, std::vector<std::string>> get_file_names(std::basic_istream<std::byte>& raw_data);

  std::vector<file_info> get_file_metadata(std::basic_istream<std::byte>& raw_data);

  void extract_files(std::basic_istream<std::byte>& volume, std::string_view volume_filename, std::string_view output_dir, file_info& some_file);

  struct vol_file_archive : shared::archive::file_archive
  {
    using folder_info = shared::archive::folder_info;

    bool stream_is_supported(std::basic_istream<std::byte>& stream) override
    {
      std::array<std::byte, 4> tag{};
      stream.read(tag.data(), sizeof(tag));

      stream.seekg(-int(sizeof(tag)), std::ios::cur);

      return tag == vol_file_tag || tag == alt_vol_file_tag || tag == old_vol_file_tag;
    }

    std::vector<std::variant<folder_info, shared::archive::file_info>> get_content_info(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) override
    {
      std::vector<std::variant<folder_info, shared::archive::file_info>> results;

      auto raw_results = get_file_metadata(stream);

      results.reserve(raw_results.size());

      std::transform(raw_results.begin(), raw_results.end(), std::back_inserter(results), [&](const file_info& value) {
        shared::archive::file_info info{};
        info.filename = value.filename;
        info.offset = value.offset;
        info.size = value.size;
        info.compression_type = shared::archive::compression_type(value.compression_type);
        info.folder_path = archive_or_folder_path;
        return info;
      });

      return results;
    }

    void set_stream_position(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info) override
    {
      // TODO this actually needs to skip passed the header before the file contents
      if (int(stream.tellg()) != info.offset)
      {
        stream.seekg(info.offset, std::ios::beg);
      }
    }

    void extract_file_contents(std::basic_istream<std::byte>& stream, const shared::archive::file_info& info, std::basic_ostream<std::byte>& output) override
    {
      set_stream_position(stream, info);

      std::copy_n(std::istreambuf_iterator<std::byte>(stream),
        info.size,
        std::ostreambuf_iterator<std::byte>(output));
    }
  };
}// namespace darkstar::vol


#endif//DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
