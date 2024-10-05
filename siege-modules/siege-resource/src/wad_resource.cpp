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

#include <siege/resource/wad_resource.hpp>
#include <siege/platform/stream.hpp>

namespace fs = std::filesystem;

namespace siege::resource::wad
{
  namespace endian = siege::platform;
  using folder_info = siege::platform::folder_info;

  constexpr auto pod_tag = platform::to_tag<8>("PODFILE");

  struct pod_file_entry
  {
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
    endian::little_uint32_t string_offset;
    endian::little_uint32_t type;
    endian::little_uint32_t id2;
    std::uint32_t padding;
    std::uint32_t string_start;
    std::uint32_t string_end;
  };

  bool wad_resource_reader::is_supported(std::istream& stream)
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 8> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));


    return tag == pod_tag;
  }

  bool wad_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<wad_resource_reader::content_info> wad_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    std::array<std::byte, 8> tag{};

    auto current_offset = stream.tellg();

    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    if (tag != pod_tag)
    {
      return std::vector<wad_resource_reader::content_info>{};
    }

    endian::little_uint32_t file_count;
    endian::little_uint32_t offset;
    endian::little_uint32_t file_buffer_size;


    stream.seekg(4, std::ios::cur);
    stream.read(reinterpret_cast<char*>(&file_count), sizeof(file_count));
    stream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    stream.read(reinterpret_cast<char*>(&file_buffer_size), sizeof(file_buffer_size));

    if (file_count == 0)
    {
      return std::vector<wad_resource_reader::content_info>{};
    }

    std::vector<pod_file_entry> entries;
    entries.assign(file_count, {});

    
    auto entries_size = file_count * sizeof(pod_file_entry);
    stream.seekg(current_offset + offset, std::ios::beg);
    stream.read(reinterpret_cast<char*>(entries.data()), entries_size);


    auto left_over_space = (std::uint32_t)file_buffer_size - entries_size;
    std::string string_table(left_over_space, '\0');
    stream.read(string_table.data(), left_over_space);

    auto index = 0;
    
    std::vector<wad_resource_reader::content_info> results{};
    results.reserve(file_count);

    for (auto& entry : entries)
    {
      auto filename = std::string_view(string_table.data() + entry.string_offset - entries_size);

      if (filename == "damage_hit")
      {
        OutputDebugStringW(L"");
      }

      if (filename == "song01")
      {
        OutputDebugStringW(L"");
      }

      if (filename == "startwalls")
      {
        OutputDebugStringW(L"");
      }

      results.emplace_back(wad_resource_reader::file_info{
        .filename = std::string(filename) + "." + std::to_string((std::uint32_t)entry.type),
        .offset = entry.offset,
        .size = entry.size,
        .compression_type = siege::platform::compression_type::none,
        .folder_path = query.folder_path,
        .archive_path = query.archive_path });
    }

    return results;
  }

  void wad_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void wad_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage) const
  {
    set_stream_position(stream, info);

    std::copy_n(std::istreambuf_iterator(stream),
      info.size,
      std::ostreambuf_iterator(output));
  }
}// namespace siege::resource::wad
