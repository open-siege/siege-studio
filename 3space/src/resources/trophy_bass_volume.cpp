#include <array>
#include <vector>
#include <fstream>
#include <utility>
#include <string>
#include "trophy_bass_volume.hpp"

namespace studio::resources::vol::trophy_bass
{
  namespace endian = boost::endian;

  constexpr auto tbv_tag = shared::to_tag<9>({ 'T', 'B', 'V', 'o', 'l', 'u', 'm', 'e', '\0' });

  constexpr auto rbx_tag = shared::to_tag<4>({ 0x9e, 0x9a, 0xa9, 0x0b });

  constexpr auto rbx_padding = shared::to_tag<4>({ 0x00, 0x00, 0x00, 0x00 });

  constexpr auto header_tag = shared::to_tag<12>({ 'R', 'i', 'c', 'h', 'R', 'a', 'y', 'l', '@', 'C', 'U', 'C' });

  constexpr auto header_alt_tag = shared::to_tag<12>({ 'R', 'i', 'c', 'h', 'R', 'a', 'y', 'l', '@', 'D', 'Y', 'N' });

  constexpr auto header_alt_tag2 = shared::to_tag<12>({ 'R', 'i', 'c', 'h', 'R', 'a', 'y', 'l', '@', 'D', 'y', 'n' });

  struct header
  {
    endian::little_int16_t unknown;
    endian::little_uint16_t num_files;
    endian::little_int32_t unknown2;
    std::array<std::byte, 12> magic_string;
    std::array<std::byte, 12> padding;
  };

  struct tbv_file_header
  {
    endian::little_int32_t checksum;
    endian::little_int32_t offset;
  };

  struct tbv_file_info
  {
    std::array<char, 24> filename;
    endian::little_uint32_t file_size;
  };

  struct rbx_file_header
  {
    std::array<char, 12> filename;
    endian::little_int32_t offset;
  };

  using folder_info = studio::resources::folder_info;

  bool rbx_file_archive::is_supported(std::basic_istream<std::byte>& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(tag.data(), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == rbx_tag;
  }

  bool rbx_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream) const
  {
    return is_supported(stream);
  }

  std::vector<std::variant<folder_info, studio::resources::file_info>> rbx_file_archive::get_content_listing(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) const
  {
    std::vector<studio::resources::file_info> results;

    std::array<std::byte, 4> tag{};
    stream.read(tag.data(), sizeof(tag));

    if (tag != rbx_tag)
    {
      throw std::invalid_argument("The file data provided is not a valid RBX file.");
    }

    endian::little_uint32_t num_files;

    stream.read(reinterpret_cast<std::byte*>(&num_files), sizeof(num_files));

    std::array<std::byte, 4> padding{};
    stream.read(reinterpret_cast<std::byte*>(&padding), sizeof(padding));

    if (padding != rbx_padding)
    {
      stream.seekg(-int(sizeof(padding)), std::ios::cur);
    }

    results.reserve(num_files);

    std::array<char, sizeof(rbx_file_header::filename) + 1> temp{ '\0' };

    for (auto i = 0u; i < num_files; ++i)
    {
      rbx_file_header header{};
      stream.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

      studio::resources::file_info info{};

      std::copy(header.filename.begin(), header.filename.end(), temp.begin());
      info.filename = temp.data();
      info.compression_type = studio::resources::compression_type::none;
      info.offset = header.offset;

      results.emplace_back(info);
    }

    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
      return a.offset < b.offset;
    });

    for (auto i = 0u; i < num_files; ++i)
    {
      auto& info = results[i];

      if (std::size_t(stream.tellg()) != info.offset)
      {
        stream.seekg(info.offset, std::ios::beg);
      }

      endian::little_uint32_t file_size{};
      stream.read(reinterpret_cast<std::byte*>(&file_size), sizeof(file_size));

      info.size = file_size;
    }

    std::vector<std::variant<folder_info, studio::resources::file_info>> final_results;
    final_results.reserve(results.size());

    std::transform(results.begin(), results.end(), std::back_inserter(final_results), [&](auto& value) {
      value.folder_path = archive_or_folder_path;

      return value;
    });

    return final_results;
  }

  void rbx_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const
  {
    if (std::size_t(stream.tellg()) == info.offset)
    {
      stream.seekg(sizeof(endian::little_int32_t), std::ios::cur);
    }
    else if (std::size_t(stream.tellg()) != info.offset + sizeof(tbv_file_info))
    {
      stream.seekg(info.offset + sizeof(endian::little_int32_t), std::ios::beg);
    }
  }

  void rbx_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info, std::basic_ostream<std::byte>& output) const
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator<std::byte>(stream),
      info.size,
      std::ostreambuf_iterator<std::byte>(output));
  }

  bool tbv_file_archive::is_supported(std::basic_istream<std::byte>& stream)
  {
    std::array<std::byte, 9> tag{};
    stream.read(tag.data(), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == tbv_tag;
  }

  bool tbv_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream) const
  {
    return is_supported(stream);
  }

  std::vector<rbx_file_archive::content_info> tbv_file_archive::get_content_listing(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) const
  {
    std::vector<studio::resources::file_info> results;

    std::array<std::byte, 9> tag{};
    stream.read(tag.data(), sizeof(tag));

    if (tag != tbv_tag)
    {
      throw std::invalid_argument("The file data provided is not a valid TBV file.");
    }

    header volume_header{};

    stream.read(reinterpret_cast<std::byte*>(&volume_header), sizeof(volume_header));

    if (!(volume_header.magic_string == header_tag ||
          volume_header.magic_string == header_alt_tag ||
          volume_header.magic_string == header_alt_tag2))
    {
      throw std::invalid_argument("The file data provided is not a valid TBV file.");
    }

    results.reserve(volume_header.num_files);

    for (auto i = 0u; i < volume_header.num_files; ++i)
    {
      tbv_file_header header{};
      stream.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

      studio::resources::file_info info{};
      info.compression_type = studio::resources::compression_type::none;
      info.offset = header.offset;

      results.emplace_back(info);
    }

    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
      return a.offset < b.offset;
    });

    std::array<char, sizeof(tbv_file_info::filename) + 1> temp{ '\0' };

    for (auto i = 0u; i < volume_header.num_files; ++i)
    {
      auto& header = results[i];

      if (std::size_t(stream.tellg()) != header.offset)
      {
        stream.seekg(header.offset, std::ios::beg);
      }
      tbv_file_info info{};
      stream.read(reinterpret_cast<std::byte*>(&info), sizeof(info));

      std::copy(info.filename.begin(), info.filename.end(), temp.begin());

      header.filename = temp.data();
      header.size = info.file_size;
    }

    std::vector<std::variant<folder_info, studio::resources::file_info>> final_results;
    final_results.reserve(results.size());

    std::transform(results.begin(), results.end(), std::back_inserter(final_results), [&](auto& value) {
      value.folder_path = archive_or_folder_path;

      return value;
    });

    return final_results;
  }

  void tbv_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const
  {
    if (std::size_t(stream.tellg()) == info.offset)
    {
      stream.seekg(sizeof(tbv_file_info), std::ios::cur);
    }
    else if (std::size_t(stream.tellg()) != info.offset + sizeof(tbv_file_info))
    {
      stream.seekg(info.offset + sizeof(tbv_file_info), std::ios::beg);
    }
  }

  void tbv_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info, std::basic_ostream<std::byte>& output) const
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator<std::byte>(stream),
      info.size,
      std::ostreambuf_iterator<std::byte>(output));
  }
}// namespace trophy_bass::vol