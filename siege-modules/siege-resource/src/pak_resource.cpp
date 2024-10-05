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

#include <siege/resource/pak_resource.hpp>
#include <siege/platform/stream.hpp>

namespace fs = std::filesystem;

template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace siege::resource::pak
{
  namespace endian = siege::platform;
  using folder_info = siege::platform::folder_info;

  constexpr auto vampire_tag = platform::to_tag<4>({ 0x1a, 'V', 'P', 'K' });
  constexpr auto quake_tag = platform::to_tag<4>({ 'P', 'A', 'C', 'K' });

  struct pak_file_entry
  {
    std::array<char, 56> path;
    endian::little_uint32_t offset;
    endian::little_uint32_t size;
  };

  bool pak_resource_reader::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    if (tag != quake_tag)
    {
      stream.seekg(34, std::ios::cur);
      stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));
      stream.seekg(-38, std::ios::cur);
      return tag == vampire_tag;
    }

    return true;
  }

  bool pak_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<pak_resource_reader::content_info> pak_resource_reader::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    endian::little_uint32_t offset;
    endian::little_uint32_t file_count;
    endian::little_uint32_t file_buffer_size;
    bool is_vampire_pak = false;
    std::array<std::byte, 4> tag{};

    auto current_offset = stream.tellg();

    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    if (tag != quake_tag)
    {
      stream.seekg(30, std::ios::cur);
      stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));
      is_vampire_pak = tag == vampire_tag;

      if (!is_vampire_pak)
      {
        return std::vector<pak_resource_reader::content_info>{};
      }
      stream.seekg(2, std::ios::cur);
    }

    std::vector<pak_file_entry> entries;

    stream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    stream.read(reinterpret_cast<char*>(&file_buffer_size), sizeof(file_buffer_size));
    file_count = file_buffer_size / sizeof(pak_file_entry);
    entries.assign(file_count, {});

    if (entries.empty())
    {
      return std::vector<pak_resource_reader::content_info>{};
    }

    stream.seekg(current_offset + offset, std::ios::beg);
    stream.read(reinterpret_cast<char*>(entries.data()), file_buffer_size);

    std::map<fs::path, std::vector<pak_file_entry*>> folders;

    for (auto& entry : entries)
    {
      auto parent_path = fs::path(entry.path.data()).make_preferred().parent_path();
      parent_path = parent_path == fs::path() ? query.archive_path : query.archive_path / parent_path;

      auto iter = folders.find(parent_path);

      if (iter == folders.end())
      {
        iter = folders.emplace(parent_path, std::vector<pak_file_entry*>{}).first;
      }

      iter->second.emplace_back(&entry);
    }

    std::vector<pak_resource_reader::content_info> results;
    results.reserve(file_count / folders.size());

    for (auto& folder : folders)
    {
      if (folder.first.parent_path() == query.folder_path)
      {
        results.emplace_back(pak_resource_reader::folder_info{
          .name = folder.first.filename().string(),
          .file_count = folder.second.size(),
          .full_path = folder.first,
          .archive_path = query.archive_path });
      }

      if (folder.first == query.folder_path)
      {
        for (auto* file : folder.second)
        {
          results.emplace_back(pak_resource_reader::file_info{
            .filename = fs::path(file->path.data()).make_preferred().filename(),
            .offset = file->offset,
            .size = file->size,
            .compression_type = siege::platform::compression_type::none,
            .folder_path = query.folder_path,
            .archive_path = query.archive_path });
        }
      }
    }

    return results;
  }

  void pak_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
  }

  void pak_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage) const
  {
  }
}// namespace siege::resource::pak
