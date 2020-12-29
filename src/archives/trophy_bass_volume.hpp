#ifndef DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
#define DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>
#include <string>

#include "archive.hpp"
#include "endian_arithmetic.hpp"

namespace trophy_bass::vol
{
  namespace endian = boost::endian;

  constexpr auto tbv_tag = shared::to_tag<9>({ 'T', 'B', 'V', 'o', 'l', 'u', 'm', 'e', '\0' });

  constexpr auto rbx_tag = shared::to_tag<4>({ 0x9e, 0x9a, 0xa9, 0x0b });

  constexpr auto rbx_padding = shared::to_tag<4>({ 0x00, 0x00, 0x00, 0x00 });

  constexpr auto header_tag = shared::to_tag<12>({ 'R', 'i', 'c', 'h', 'R', 'a', 'y', 'l', '@', 'C', 'U', 'C' });

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

  struct tbv_archive_info
  {
    std::vector<std::pair<tbv_file_header, tbv_file_info>> headers;
  };

  struct rbx_file_header
  {
    std::array<char, 12> filename;
    endian::little_int32_t offset;
  };

  struct rbx_archive_info
  {
    std::vector<std::pair<rbx_file_header, endian::little_uint32_t>> headers;
  };

  tbv_archive_info get_tbv_file_info(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 9> tag{};
    raw_data.read(tag.data(), sizeof(tag));

    if (tag != tbv_tag)
    {
      throw std::invalid_argument("The file data provided is not a valid TBV file.");
    }

    header volume_header{};

    raw_data.read(reinterpret_cast<std::byte*>(&volume_header), sizeof(volume_header));

    if (volume_header.magic_string != header_tag)
    {
      throw std::invalid_argument("The file data provided is not a valid TBV file.");
    }

    tbv_archive_info result{};
    result.headers.reserve(volume_header.num_files);

    for (auto i = 0u; i < volume_header.num_files; ++i)
    {
      tbv_file_header header{};
      raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));
      result.headers.emplace_back(std::make_pair(header, tbv_file_info{}));
    }

    std::sort(result.headers.begin(), result.headers.end(), [] (const auto& a, const auto& b)  {
           return a.first.offset < b.first.offset;
    });

    for (auto i = 0u; i < volume_header.num_files; ++i)
    {
      auto& header = result.headers[i];

      if (int(raw_data.tellg()) != header.first.offset)
      {
        raw_data.seekg(header.first.offset, std::ios::beg);
      }

      raw_data.read(reinterpret_cast<std::byte*>(&header.second), sizeof(header.second));
    }

    return result;
  }

  rbx_archive_info get_rbx_data(std::basic_istream<std::byte>& raw_data)
  {
    std::array<std::byte, 4> tag{};
    raw_data.read(tag.data(), sizeof(tag));

    if (tag != rbx_tag)
    {
      throw std::invalid_argument("The file data provided is not a valid RBX file.");
    }

    endian::little_uint32_t num_files;

    raw_data.read(reinterpret_cast<std::byte*>(&num_files), sizeof(num_files));

    std::array<std::byte, 4> padding{};
    raw_data.read(reinterpret_cast<std::byte*>(&padding), sizeof(padding));

    if (padding != rbx_padding)
    {
      raw_data.seekg(-int(sizeof(padding)), std::ios::cur);
    }

    rbx_archive_info result{};
    result.headers.reserve(num_files);

    for (auto i = 0u; i < num_files; ++i)
    {
      rbx_file_header header{};
      raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));
      result.headers.emplace_back(std::make_pair(header, 0));
    }

    std::sort(result.headers.begin(), result.headers.end(), [] (const auto& a, const auto& b)  {
      return a.first.offset < b.first.offset;
    });

    for (auto i = 0u; i < num_files; ++i)
    {
      auto& info = result.headers[i];

      if (int(raw_data.tellg()) != info.first.offset)
      {
        raw_data.seekg(info.first.offset, std::ios::beg);
      }

      raw_data.read(reinterpret_cast<std::byte*>(&info.second), sizeof(info.second));
    }

    return result;
  }

  struct rbx_file_archive : shared::archive::file_archive
  {
    using folder_info = shared::archive::folder_info;

    bool stream_is_supported(std::basic_istream<std::byte>& stream) override
    {
      std::array<std::byte, 4> tag{};
      stream.read(tag.data(), sizeof(tag));

      stream.seekg(-int(sizeof(tag)), std::ios::cur);

      return tag == rbx_tag;
    }

    std::vector<std::variant<folder_info, shared::archive::file_info>> get_content_info(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) override
    {
      std::vector<std::variant<folder_info, shared::archive::file_info>> results;

      auto raw_results = get_rbx_data(stream);

      results.reserve(raw_results.headers.size());

      std::array<char, sizeof(rbx_file_header::filename) + 1> temp {'\0'};

      for (auto& header : raw_results.headers)
      {
        shared::archive::file_info file{};
        file.size = header.second;
        file.offset = header.first.offset;
        file.compression_type = shared::archive::compression_type::none;

        std::copy(header.first.filename.begin(), header.first.filename.end(), temp.begin());
        file.filename = temp.data();
        file.folder_path = archive_or_folder_path;

        results.emplace_back(file);
      }

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

  struct tbv_file_archive : shared::archive::file_archive
  {
    using folder_info = shared::archive::folder_info;

    bool stream_is_supported(std::basic_istream<std::byte>& stream) override
    {
      std::array<std::byte, 9> tag{};
      stream.read(tag.data(), sizeof(tag));

      stream.seekg(-int(sizeof(tag)), std::ios::cur);

      return tag == tbv_tag;
    }

    std::vector<std::variant<folder_info, shared::archive::file_info>> get_content_info(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) override
    {
      std::vector<std::variant<folder_info, shared::archive::file_info>> results;

      auto raw_results = get_tbv_file_info(stream);

      results.reserve(raw_results.headers.size());

      std::array<char, sizeof(tbv_file_info::filename) + 1> temp {'\0'};

      for (auto& header : raw_results.headers)
      {
        shared::archive::file_info file{};
        file.size = header.second.file_size;
        file.offset = header.first.offset;
        file.compression_type = shared::archive::compression_type::none;

        std::copy(header.second.filename.begin(), header.second.filename.end(), temp.begin());
        file.filename = temp.data();
        file.folder_path = archive_or_folder_path;

        results.emplace_back(file);
      }

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
}// namespace trophy_bass::vol

#endif//DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
