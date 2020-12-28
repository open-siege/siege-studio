#ifndef DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
#define DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP

#include <array>
#include <vector>
#include <fstream>
#include <optional>
#include <utility>
#include <string>

#include "endian_arithmetic.hpp"

namespace trophy_bass::vol
{
  namespace endian = boost::endian;

  template<std::size_t Count>
  constexpr std::array<std::byte, Count> to_tag(const std::array<std::uint8_t, Count> values)
  {
    std::array<std::byte, Count> result{};

    for (auto i = 0u; i < values.size(); i++)
    {
      result[i] = std::byte{ values[i] };
    }
    return result;
  }

  constexpr auto vol_tag = to_tag<9>({ 'T', 'B', 'V', 'o', 'l', 'u', 'm', 'e', '\0' });

  constexpr auto header_tag = to_tag<12>({ 'R', 'i', 'c', 'h', 'R', 'a', 'y', 'l', '@', 'C', 'U', 'C' });


  struct header
  {
    endian::little_int16_t unknown;
    endian::little_uint16_t num_files;
    endian::little_int32_t unknown2;
    std::array<std::byte, 12> magic_string;
    std::array<std::byte, 12> padding;
  };

  struct file_header
  {
    endian::little_int32_t checksum;
    endian::little_int32_t offset;
  };

  struct file_info
  {
    std::array<char, 24> filename;
    endian::little_uint32_t file_size;
  };

  void get_data(std::basic_ifstream<std::byte>& raw_data)
  {
    std::array<std::byte, 9> tag{};
    raw_data.read(tag.data(), sizeof(tag));

    if (tag != vol_tag)
    {
      throw std::invalid_argument("The file data provided is not a valid TBV file.");
    }

    header volume_header{};

    raw_data.read(reinterpret_cast<std::byte*>(&volume_header), sizeof(volume_header));

    if (volume_header.magic_string != header_tag)
    {
      throw std::invalid_argument("The file data provided is not a valid TBV file.");
    }

    std::vector<file_header> headers;
    headers.reserve(volume_header.num_files);

    for (auto i = 0u; i < volume_header.num_files; ++i)
    {
      file_header header{};
      raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));
      headers.emplace_back(header);
    }

    for (auto i = 0u; i < volume_header.num_files; ++i)
    {
      file_info info{};
      raw_data.read(reinterpret_cast<std::byte*>(&info), sizeof(info));
      auto new_path = std::filesystem::path("volume") / info.filename.data();
      std::cout << std::string(info.filename.data()) << '\n';

      auto new_file = std::basic_ofstream<std::byte>{ new_path, std::ios::binary };

      auto current_position = raw_data.tellg();
      std::copy_n(std::istreambuf_iterator<std::byte>(raw_data),
                  info.file_size,
                  std::ostreambuf_iterator<std::byte>(new_file));

      raw_data.seekg(static_cast<int>(current_position) + info.file_size, std::ios::beg);
    }
  }
}

#endif//DARKSTARDTSCONVERTER_TROPHY_BASS_VOLUME_HPP
