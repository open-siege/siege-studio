#ifndef DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP
#define DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP

#include <fstream>
#include <vector>
#include <array>
#include <utility>
#include <string>
#include "endian_arithmetic.hpp"

namespace darkstar::vol
{
  namespace endian = boost::endian;
  using namespace std::literals;

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

  enum class compression_type : std::uint8_t
  {
    none,
    rle,
    lz,
    lzh
  };

  struct file_info
  {
    std::string filename;
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
    compression_type compression_type;
  };

  struct volume_header
  {
    std::array<std::byte, 4> file_tag;
    endian::little_uint32_t footer_offset;
  };

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


  std::size_t get_file_list_offsets(std::basic_ifstream<std::byte>& raw_data)
  {
    volume_header header;

    raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

    if (header.file_tag == vol_file_tag)
    {
      normal_footer footer;
      raw_data.seekg(header.footer_offset, std::ios_base::beg);
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      return footer.file_list_size;
    }
    else if (header.file_tag == alt_vol_file_tag)
    {
      alternative_footer footer;
      raw_data.seekg(header.footer_offset, std::ios_base::beg);
      raw_data.read(reinterpret_cast<std::byte*>(&footer), sizeof(footer));

      return footer.file_list_size;
    }
    else
    {
      throw std::invalid_argument("The file provided is not a valid Darkstar VOL file.");
    }
  }

  std::vector<std::string> get_file_names(std::basic_ifstream<std::byte>& raw_data)
  {
    auto buffer_size = get_file_list_offsets(raw_data);
    std::vector<char> raw_chars(buffer_size);

    if ((buffer_size) % 2 != 0)
    {
      raw_data.seekg(1, std::ios::cur);
    }

    raw_data.read(reinterpret_cast<std::byte*>(raw_chars.data()), raw_chars.size());

    std::vector<std::string> results;

    std::size_t index = 0;

    while (index < raw_chars.size())
    {
      results.emplace_back(raw_chars.data() + index);

      index += results.back().size() + 1;
    }

    return results;
  }

  std::vector<file_info> get_file_metadata(std::basic_ifstream<std::byte>& raw_data)
  {
    auto filenames = get_file_names(raw_data);
    file_index_header header;
    raw_data.read(reinterpret_cast<std::byte*>(&header), sizeof(header));

    std::vector<std::byte> raw_bytes(header.index_size);
    raw_data.read(raw_bytes.data(), raw_bytes.size());

    std::size_t index = 0;

    std::vector<file_info> results;
    results.reserve(filenames.size());

    while (index < raw_bytes.size())
    {
      file_header file;
      std::copy(raw_bytes.data() + index, raw_bytes.data() + index + sizeof(file_header), reinterpret_cast<std::byte*>(&file));

      file_info info;

      info.filename = std::move(filenames[results.size()]);
      info.size = file.size;
      info.offset = file.offset;
      info.compression_type = file.compression_type;
      results.emplace_back(info);
      index += sizeof(file_header);
    }

    return results;
  }
}// namespace darkstar


#endif//DARKSTARDTSCONVERTER_DARKSTAR_VOLUME_HPP

