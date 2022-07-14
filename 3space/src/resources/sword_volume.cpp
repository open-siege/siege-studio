#include <array>
#include <vector>
#include <fstream>
#include <utility>
#include <string>

#include "resources/sword_volume.hpp"

namespace studio::resources::atd
{
  namespace endian = boost::endian;

  bool atd_file_archive::is_supported(std::basic_istream<std::byte>& stream)
  {
    endian::little_int32_t file_count;
    stream.read(reinterpret_cast<std::byte*>(&file_count), sizeof(file_count));

    stream.seekg(-int(sizeof(file_count)), std::ios::cur);

    return file_count <= 6000;
  }

  bool atd_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream) const
  {
    return is_supported(stream);
  }

  struct file_entry
  {
    endian::little_uint32_t offset;
    endian::little_uint32_t file_size;
    std::array<char, 80> filename;
  };

  std::vector<atd_file_archive::content_info> atd_file_archive::get_content_listing(std::basic_istream<std::byte>& stream, const listing_query& query) const
  {
    std::vector<atd_file_archive::content_info> results;

    endian::little_uint32_t file_count;
    stream.read(reinterpret_cast<std::byte*>(&file_count), sizeof(file_count));

    results.reserve(file_count);
    std::array<char, 81> temp_filename{'\0'};

    for (auto i = 0u; i < file_count; i++)
    {
      file_entry temp;
      stream.read(reinterpret_cast<std::byte*>(&temp), sizeof(temp));

      file_info info{};

      info.offset = temp.offset;
      info.size = temp.file_size;
      info.compression_type = compression_type::none;
      info.folder_path = query.folder_path;

      std::copy(temp.filename.begin(), temp.filename.end(), temp_filename.begin());

      info.filename = temp_filename.data();

      results.emplace_back(info);
    }

    return results;
  }

  void atd_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void atd_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info, std::basic_ostream<std::byte>& output) const
  {
    set_stream_position(stream, info);
    std::copy_n(std::istreambuf_iterator<std::byte>(stream),
      info.size,
      std::ostreambuf_iterator<std::byte>(output));

    stream.seekg(2, std::ios::cur);
  }
}// namespace studio::resources::vol::three_space
