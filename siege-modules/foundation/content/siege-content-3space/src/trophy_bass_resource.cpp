#include <array>
#include <vector>
#include <fstream>
#include <utility>
#include <string>
#include <siege/resource/trophy_bass_resource.hpp>

namespace siege::resource::vol::trophy_bass
{
  namespace endian = siege::platform;

  constexpr auto tbv_tag = platform::to_tag<9>({ 'T', 'B', 'V', 'o', 'l', 'u', 'm', 'e', '\0' });

  constexpr auto rbx_tag = platform::to_tag<4>({ 0x9e, 0x9a, 0xa9, 0x0b });

  constexpr auto rbx_padding = platform::to_tag<4>({ 0x00, 0x00, 0x00, 0x00 });

  constexpr auto header_tag = platform::to_tag<12>({ 'R', 'i', 'c', 'h', 'R', 'a', 'y', 'l', '@', 'C', 'U', 'C' });

  constexpr auto header_alt_tag = platform::to_tag<12>({ 'R', 'i', 'c', 'h', 'R', 'a', 'y', 'l', '@', 'D', 'Y', 'N' });

  constexpr auto header_alt_tag2 = platform::to_tag<12>({ 'R', 'i', 'c', 'h', 'R', 'a', 'y', 'l', '@', 'D', 'y', 'n' });

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

  using folder_info = siege::platform::folder_info;

  rbx_resource_reader::rbx_resource_reader() : resource_reader{ stream_is_supported, get_content_listing, set_stream_position, extract_file_contents }
  {
  }

  bool rbx_resource_reader::stream_is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == rbx_tag;
  }

  std::vector<std::variant<folder_info, siege::platform::file_info>> rbx_resource_reader::get_content_listing(std::any&, std::istream& stream, const platform::listing_query& query)
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<siege::platform::file_info> results;

    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    if (tag != rbx_tag)
    {
      throw std::invalid_argument("The file data provided is not a valid RBX file.");
    }

    endian::little_uint32_t num_files;

    stream.read(reinterpret_cast<char*>(&num_files), sizeof(num_files));

    std::array<std::byte, 4> padding{};
    stream.read(reinterpret_cast<char*>(&padding), sizeof(padding));

    if (padding != rbx_padding)
    {
      stream.seekg(-int(sizeof(padding)), std::ios::cur);
    }

    results.reserve(num_files);

    std::array<char, sizeof(rbx_file_header::filename) + 1> temp{ '\0' };

    for (auto i = 0u; i < num_files; ++i)
    {
      rbx_file_header header{};
      stream.read(reinterpret_cast<char*>(&header), sizeof(header));

      siege::platform::file_info info{};

      std::copy(header.filename.begin(), header.filename.end(), temp.begin());
      info.filename = temp.data();
      info.compression_type = siege::platform::compression_type::none;
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
      stream.read(reinterpret_cast<char*>(&file_size), sizeof(file_size));

      info.size = file_size;
    }

    std::vector<std::variant<folder_info, siege::platform::file_info>> final_results;
    final_results.reserve(results.size());

    std::transform(results.begin(), results.end(), std::back_inserter(final_results), [&](auto& value) {
      value.folder_path = query.folder_path;

      return value;
    });

    return final_results;
  }

  void rbx_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info)
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

  void rbx_resource_reader::extract_file_contents(std::any&, std::istream& stream, const siege::platform::file_info& info, std::ostream& output)
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator(stream),
      info.size,
      std::ostreambuf_iterator(output));
  }

  tbv_resource_reader::tbv_resource_reader() : resource_reader{ stream_is_supported, get_content_listing, set_stream_position, extract_file_contents }
  {
  }

  bool tbv_resource_reader::stream_is_supported(std::istream& stream)
  {
    std::array<std::byte, 9> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == tbv_tag;
  }

  std::vector<rbx_resource_reader::content_info> tbv_resource_reader::get_content_listing(std::any&, std::istream& stream, const platform::listing_query& query)
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<siege::platform::file_info> results;

    std::array<std::byte, 9> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    if (tag != tbv_tag)
    {
      throw std::invalid_argument("The file data provided is not a valid TBV file.");
    }

    header volume_header{};

    stream.read(reinterpret_cast<char*>(&volume_header), sizeof(volume_header));

    if (!(volume_header.magic_string == header_tag || volume_header.magic_string == header_alt_tag || volume_header.magic_string == header_alt_tag2))
    {
      throw std::invalid_argument("The file data provided is not a valid TBV file.");
    }

    results.reserve(volume_header.num_files);

    for (auto i = 0u; i < volume_header.num_files; ++i)
    {
      tbv_file_header header{};
      stream.read(reinterpret_cast<char*>(&header), sizeof(header));

      siege::platform::file_info info{};
      info.compression_type = siege::platform::compression_type::none;
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
      stream.read(reinterpret_cast<char*>(&info), sizeof(info));

      std::copy(info.filename.begin(), info.filename.end(), temp.begin());

      header.filename = temp.data();
      header.size = info.file_size;
    }

    std::vector<std::variant<folder_info, siege::platform::file_info>> final_results;
    final_results.reserve(results.size());

    std::transform(results.begin(), results.end(), std::back_inserter(final_results), [&](auto& value) {
      value.folder_path = query.folder_path;

      return value;
    });

    return final_results;
  }

  void tbv_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info)
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

  void tbv_resource_reader::extract_file_contents(std::any&, std::istream& stream, const siege::platform::file_info& info, std::ostream& output)
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator(stream),
      info.size,
      std::ostreambuf_iterator(output));
  }
}// namespace siege::resource::vol::trophy_bass
