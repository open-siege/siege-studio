#include <array>
#include <vector>
#include <fstream>
#include <utility>
#include <string>

#include <siege/resource/sword_resource.hpp>

namespace siege::resource::atd
{
  namespace endian = siege::platform;

  bool atd_resource_reader::is_supported(std::istream& stream)
  {
    endian::little_int32_t file_count;
    stream.read(reinterpret_cast<char*>(&file_count), sizeof(file_count));

    stream.seekg(-int(sizeof(file_count)), std::ios::cur);

    return file_count <= 6000;
  }

  bool atd_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  struct file_entry
  {
    endian::little_uint32_t offset;
    endian::little_uint32_t file_size;
    std::array<char, 80> filename;
  };

  std::vector<atd_resource_reader::content_info> atd_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::vector<atd_resource_reader::content_info> results;

    endian::little_uint32_t file_count;
    stream.read(reinterpret_cast<char*>(&file_count), sizeof(file_count));

    results.reserve(file_count);
    std::array<char, 81> temp_filename{'\0'};

    for (auto i = 0u; i < file_count; i++)
    {
      file_entry temp;
      stream.read(reinterpret_cast<char*>(&temp), sizeof(temp));

      file_info info{};

      info.offset = temp.offset;
      info.size = temp.file_size;
      info.compression_type = platform::compression_type::none;
      info.folder_path = query.folder_path;

      std::copy(temp.filename.begin(), temp.filename.end(), temp_filename.begin());

      info.filename = temp_filename.data();

      results.emplace_back(info);
    }

    return results;
  }

  void atd_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void atd_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>>) const
  {
    set_stream_position(stream, info);
    std::copy_n(std::istreambuf_iterator(stream),
      info.size,
      std::ostreambuf_iterator(output));

    stream.seekg(2, std::ios::cur);
  }
}// namespace siege::resource::vol::three_space
