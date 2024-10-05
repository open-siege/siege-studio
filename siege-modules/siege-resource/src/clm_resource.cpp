#include <fstream>
#include <filesystem>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <map>

#include <siege/resource/clm_resource.hpp>
#include <siege/platform/stream.hpp>
#include <siege/platform/wave.hpp>

namespace fs = std::filesystem;

namespace siege::resource::clm
{
  namespace endian = siege::platform;

  constexpr auto clm_tag = platform::to_tag<26>("OP2 Clump File Version 1.0");

  struct clm_file_entry
  {
    std::array<char, 8> name;
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
  };

  bool clm_resource_reader::is_supported(std::istream& stream)
  {
    std::array<std::byte, 26> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == clm_tag;
  }

  bool clm_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<clm_resource_reader::content_info> clm_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    std::array<std::byte, 26> tag{};

    auto current_offset = stream.tellg();

    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    std::vector<clm_resource_reader::content_info> results;

    if (tag != clm_tag)
    {
      return results;
    }

    siege::platform::wave::format_header wave_header;
    endian::little_uint16_t file_count;
    endian::little_uint32_t file_count_prefix;
    stream.read(reinterpret_cast<char*>(&file_count), sizeof(file_count));
    stream.seekg(4, std::ios::cur);
    stream.read(reinterpret_cast<char*>(&wave_header), sizeof(wave_header));
    stream.seekg(8, std::ios::cur);
    stream.read(reinterpret_cast<char*>(&file_count_prefix), sizeof(file_count_prefix));

    if (file_count == file_count_prefix)
    {
      std::vector<clm_file_entry> entries(file_count, {});
      stream.read(reinterpret_cast<char*>(entries.data()), sizeof(clm_file_entry) * entries.size());

      for (auto& entry : entries)
      {
        std::string name = *entry.name.rbegin() == '\0' ? std::string(entry.name.data()) : std::string(entry.name.data(), entry.name.size()); 

        results.emplace_back(clm_resource_reader::file_info{
          .filename = name + ".wav",
          .offset = entry.offset,
          .size = entry.size,
          .compression_type = siege::platform::compression_type::none,
          .folder_path = query.folder_path,
          .archive_path = query.archive_path,
          .metadata = wave_header });
      }
    }

    return results;
  }

  void clm_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void clm_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage) const
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator(stream),
      info.size,
      std::ostreambuf_iterator(output));
  }
}// namespace siege::resource::clm
