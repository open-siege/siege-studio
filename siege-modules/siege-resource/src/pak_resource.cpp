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
#include <zlib.h>

#include <siege/resource/pak_resource.hpp>
#include <siege/platform/stream.hpp>

namespace fs = std::filesystem;

namespace siege::resource::pak
{
  namespace endian = siege::platform;
  using folder_info = siege::platform::folder_info;

  constexpr auto vampire_tag = platform::to_tag<4>({ 0x1a, 'V', 'P', 'K' });
  constexpr auto quake_tag = platform::to_tag<4>({ 'P', 'A', 'C', 'K' });
  constexpr auto anox_tag = platform::to_tag<4>({ 'A', 'D', 'A', 'T' });

  struct pak_file_entry
  {
    std::array<char, 56> path;
    endian::little_uint32_t offset;
    endian::little_uint32_t uncompressed_size;
  };

  struct daikatana_pak_file_entry
  {
    std::array<char, 56> path;
    endian::little_uint32_t offset;
    endian::little_uint32_t uncompressed_size;
    endian::little_uint32_t compressed_size;
    endian::little_uint32_t has_compression;
  };

  struct dat_file_entry
  {
    std::array<char, 128> path;
    endian::little_uint32_t offset;
    endian::little_uint32_t uncompressed_size;
    endian::little_uint32_t compressed_size;
    endian::little_uint32_t checksum;
  };

  using file_entry = std::variant<pak_file_entry, daikatana_pak_file_entry, dat_file_entry>;

  bool pak_resource_reader::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    if (!(tag == quake_tag || tag == anox_tag))
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
    platform::istream_pos_resetter resetter(stream);
    endian::little_uint32_t offset;
    endian::little_uint32_t file_count;
    endian::little_uint32_t file_buffer_size;
    bool is_vampire_pak = false;
    std::array<std::byte, 4> tag{};

    auto current_offset = stream.tellg();

    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    if (!(tag == quake_tag || tag == anox_tag))
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

    std::vector<file_entry> entries;

    stream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    stream.read(reinterpret_cast<char*>(&file_buffer_size), sizeof(file_buffer_size));

    auto entry_type = typeid(pak_file_entry).hash_code();

    if (tag == anox_tag)
    {
      entry_type = typeid(dat_file_entry).hash_code();
      file_count = file_buffer_size / sizeof(dat_file_entry);
    }
    else if ((file_buffer_size % sizeof(pak_file_entry)) == 0)
    {
      file_count = file_buffer_size / sizeof(pak_file_entry);
    }
    else if ((file_buffer_size % sizeof(daikatana_pak_file_entry)) == 0)
    {
      entry_type = typeid(daikatana_pak_file_entry).hash_code();
      file_count = file_buffer_size / sizeof(daikatana_pak_file_entry);
    }

    if (file_count == 0)
    {
      return std::vector<pak_resource_reader::content_info>{};
    }

    entries.reserve(file_count);

    stream.seekg(current_offset + offset, std::ios::beg);


    if (entry_type == typeid(pak_file_entry).hash_code())
    {
      for (auto i = 0; i < file_count; ++i)
      {
        pak_file_entry temp;
        stream.read((char*)&temp, sizeof(temp));
        entries.emplace_back(std::move(temp));
      }
    }
    else if (entry_type == typeid(daikatana_pak_file_entry).hash_code())
    {
      for (auto i = 0; i < file_count; ++i)
      {
        daikatana_pak_file_entry temp;
        stream.read((char*)&temp, sizeof(temp));
        entries.emplace_back(std::move(temp));
      }
    }
    else if (entry_type == typeid(dat_file_entry).hash_code())
    {
      for (auto i = 0; i < file_count; ++i)
      {
        dat_file_entry temp;
        stream.read((char*)&temp, sizeof(temp));
        entries.emplace_back(std::move(temp));
      }
    }

    std::map<fs::path, std::vector<file_entry*>> folders;

    for (auto& entry : entries)
    {
      auto entry_path = std::visit([](auto& value) { return value.path.data(); }, entry);
      auto parent_path = fs::path(entry_path).make_preferred().parent_path();

      parent_path = parent_path == fs::path() ? query.archive_path : query.archive_path / parent_path;

      auto iter = folders.find(parent_path);

      if (iter == folders.end())
      {
        iter = folders.emplace(parent_path, std::vector<file_entry*>{}).first;
      }

      iter->second.emplace_back(&entry);

      if (parent_path.parent_path() != query.archive_path)
      {
        auto iter = folders.find(parent_path.parent_path());

        if (iter == folders.end())
        {
          iter = folders.emplace(parent_path.parent_path(), std::vector<file_entry*>{}).first;
        }
      }
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
          std::visit([&](auto& entry) {
            if constexpr (std::is_same_v<std::decay_t<decltype(entry)>, pak_file_entry>)
            {
              results.emplace_back(pak_resource_reader::file_info{
                .filename = fs::path(entry.path.data()).make_preferred().filename(),
                .offset = entry.offset,
                .size = entry.uncompressed_size,
                .compression_type = siege::platform::compression_type::none,
                .folder_path = query.folder_path,
                .archive_path = query.archive_path });
            }
            else
            {
              auto compression_type = entry.uncompressed_size == entry.compressed_size ? siege::platform::compression_type::none : siege::platform::compression_type::code_rle;

              if (tag == anox_tag)
              {
                compression_type = siege::platform::compression_type::lz77_huffman;
              }

              results.emplace_back(pak_resource_reader::file_info{
                .filename = fs::path(entry.path.data()).make_preferred().filename(),
                .offset = entry.offset,
                .size = entry.uncompressed_size,
                .compressed_size = entry.compressed_size,
                .compression_type = compression_type,
                .folder_path = query.folder_path,
                .archive_path = query.archive_path });
            }
          },
            *file);
        }
      }
    }

    return results;
  }

  void pak_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
    if (std::size_t(stream.tellg()) != info.offset)
    {
      stream.seekg(info.offset, std::ios::beg);
    }
  }

  void pak_resource_reader::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage) const
  {
    set_stream_position(stream, info);

    if (info.compressed_size && info.compression_type != platform::compression_type::none)
    {
      if (info.compression_type == platform::compression_type::code_rle)
      {
        std::vector<std::uint8_t> compressed_bytes(*info.compressed_size);

        stream.read((char*)compressed_bytes.data(), compressed_bytes.size());

        std::span<std::uint8_t> bytes(compressed_bytes);

        std::vector<char> output_buffer;
        output_buffer.reserve(info.size);

        // Thanks to https://gist.github.com/DanielGibson/8bde6241c93e5efe8b75e5e00d0b9858 for helping understand
        // the offset command as this would have taken much longer to figure out.
        while (!bytes.empty())
        {
          auto op_code = bytes.front();

          if (op_code == 255)
          {
          end_of_stream:
            break;
          }

          if (op_code >= 0 && op_code <= 63)
          {
            if (op_code + 1 > bytes.size())
            {
              break;
            }
          copy_multiple:
            auto data = bytes.subspan(1, op_code + 1);
            output_buffer.insert(output_buffer.end(), data.begin(), data.end());
            bytes = bytes.subspan(op_code + 2);
          }

          else if (op_code >= 64 && op_code <= 127)
          {
          repeat_zero:
            constexpr static auto distance = (127 / 2) - 1;
            output_buffer.insert(output_buffer.end(), op_code - distance, '\0');
            bytes = bytes.subspan(1);
          }

          else if (op_code >= 128 && op_code <= 191)
          {
          repeat_value:
            constexpr static auto distance = 127 - 1;
            output_buffer.insert(output_buffer.end(), op_code - distance, bytes[1]);
            bytes = bytes.subspan(2);
          }
          else if (op_code >= 192 && op_code <= 254)
          {
          copy_existing:
            constexpr static auto distance = (127 + 1) / 2 * 3;

            auto size = op_code - distance + 2;
            auto offset = bytes[1] + 2;

            if (size > output_buffer.size() || size > offset)
            {
              break;
            }

            if (offset > output_buffer.size())
            {
              break;
            }

            auto data = std::span(output_buffer).subspan(output_buffer.size() - offset, size);
            output_buffer.insert(output_buffer.end(), data.begin(), data.end());
            bytes = bytes.subspan(2);
          }
        }
        output.write(output_buffer.data(), output_buffer.size());
      }
      else
      {
        std::vector<char> compressed_buffer(*info.compressed_size);
        stream.read(compressed_buffer.data(), compressed_buffer.size());

        std::vector<char> output_buffer(info.size);

        z_stream temp{
          .next_in = (Bytef*)compressed_buffer.data(),
          .avail_in = compressed_buffer.size(),
          .next_out = (Bytef*)output_buffer.data(),
          .avail_out = output_buffer.size()
        };

        if (inflateInit_(&temp, ZLIB_VERSION, (int)sizeof(z_stream)) == Z_OK)
        {
          auto result = inflate(&temp, Z_FINISH);

          if (result == Z_OK || result == Z_STREAM_END)
          {
            output.write(output_buffer.data(), output_buffer.size());
          }
          inflateEnd(&temp);
        }
      }
    }
    else
    {
      std::copy_n(std::istreambuf_iterator(stream),
        info.size,
        std::ostreambuf_iterator(output));
    }
  }
}// namespace siege::resource::pak
