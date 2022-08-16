#include <array>
#include <vector>
#include <fstream>
#include <utility>
#include <string>
#include <unordered_map>

#include "resources/cyclone_volume.hpp"

namespace studio::resources::cln
{
  namespace endian = boost::endian;


  bool cln_file_archive::is_supported(std::istream& stream)
  {
    endian::little_int32_t file_count;
    stream.read(reinterpret_cast<char*>(&file_count), sizeof(file_count));

    endian::little_int32_t folder_name_length;
    stream.read(reinterpret_cast<char*>(&folder_name_length), sizeof(folder_name_length));

    stream.seekg(-int(sizeof(file_count)) * 2, std::ios::cur);

    return file_count <= 6000 == folder_name_length <= 256;
  }

  bool cln_file_archive::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  struct file_entry
  {
    endian::little_uint32_t string_length;
    std::string filename;
    endian::little_uint32_t file_size;
    endian::little_uint32_t flags;
    endian::little_uint32_t offset;
  };

  auto get_file_entries(std::istream& stream)
  {
      std::vector<file_entry> entries;

      endian::little_int32_t file_count;
      stream.read(reinterpret_cast<char*>(&file_count), sizeof(file_count));

      entries.reserve(file_count);

      for (auto i = 0; i < file_count; i++)
      {
        auto& entry = entries.emplace_back();
        stream.read(reinterpret_cast<char*>(&entry.string_length), sizeof(entry.string_length));
        entry.filename.assign(entry.string_length, '\0');

        stream.read(reinterpret_cast<char*>(entry.filename.data()), entry.string_length);

        stream.read(reinterpret_cast<char*>(&entry.file_size), sizeof(entry.file_size));
        stream.read(reinterpret_cast<char*>(&entry.flags), sizeof(entry.flags));
        stream.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
      }

      return entries;
  }

  // Thanks https://en.cppreference.com/w/cpp/string/basic_string/replace
  std::size_t replace_all(std::string& inout, std::string_view what, std::string_view with)
  {
    std::size_t count{};
    for (std::string::size_type pos{};
         inout.npos != (pos = inout.find(what.data(), pos, what.length()));
         pos += with.length(), ++count) {
      inout.replace(pos, what.length(), with.data(), with.length());
    }
    return count;
  }

  auto normalise(std::string file_with_folder)
  {
    auto sep = (std::filesystem::path("parent") / "child").string();

    replace_all(sep, "parent", "");
    replace_all(sep, "child", "");

    replace_all(file_with_folder, "\\", sep);

    if (file_with_folder.back() == sep[0])
    {
      file_with_folder.pop_back();
    }

    return file_with_folder;
  }

  auto starts_with(std::string_view input, std::string_view value_to_find)
  {
    return input.rfind(value_to_find, 0) == 0;
  }

  std::vector<cln_file_archive::content_info> cln_file_archive::get_content_listing(std::istream& stream, const listing_query& query) const
  {
    thread_local std::unordered_map<std::string, std::vector<file_entry>> cache;
    auto cached_entries = cache.find(query.archive_path.string());

    if (cached_entries == cache.end())
    {
      cached_entries = cache.emplace(query.archive_path.string(), get_file_entries(stream)).first;
    }

    std::vector<cln_file_archive::content_info> results;

    if (cached_entries == cache.end())
    {
      return results;
    }

    auto& entries = cached_entries->second;

    results.reserve(16);

    if (query.archive_path == query.folder_path)
    {
      for (const auto& entry : entries)
      {
        if (!(entry.offset == 0 && entry.file_size == 0))
        {
          continue;
        }

        auto folder_name = normalise(entry.filename);

        studio::resources::folder_info info{};
        info.full_path = query.archive_path / folder_name;

        if (info.full_path.parent_path() == query.archive_path)
        {
          info.name = folder_name;

          results.emplace_back(info);
        }
      }

      return results;
    }

    auto folder_entry = std::find_if(entries.begin(), entries.end(), [&](const auto& entry) {
      if (entry.offset == 0 && entry.file_size == 0)
      {
        auto folder_name = normalise(entry.filename);

        auto full_path = query.archive_path / folder_name;

        return full_path == query.folder_path;
      }
      return false;
    });

    if (folder_entry != entries.end())
    {
      for (const auto& entry : entries)
      {
        auto filename = normalise(entry.filename);

        auto full_path = query.archive_path / filename;

        if (full_path.parent_path() == query.folder_path)
        {
          if (entry.offset == 0 && entry.file_size == 0)
          {
            studio::resources::folder_info temp{};
            temp.full_path = full_path;
            temp.name = full_path.filename().string();

            results.emplace_back(temp);
          }
          else if (entry.flags == 0)
          {
            file_info temp{};
            temp.size = entry.file_size;
            temp.offset = entry.offset;
            temp.filename = full_path.filename();
            temp.folder_path = full_path.parent_path();
            temp.compression_type = compression_type::none;

            results.emplace_back(temp);
          }
          else
          {
            file_info temp{};
            temp.size = entry.file_size;
            temp.offset = entry.offset;
            temp.compressed_size = entry.flags;
            temp.filename = full_path.filename();
            temp.folder_path = full_path.parent_path();
            temp.compression_type = compression_type::rle;

            results.emplace_back(temp);
          }
        }
      }
    }

    return results;
  }

  constexpr auto file_data_header = std::string_view("MAGIC_NUMBER");

  void cln_file_archive::set_stream_position(std::istream& stream, const studio::resources::file_info& info) const
  {
    if (std::size_t(stream.tellg()) == info.offset)
    {
      stream.seekg(file_data_header.length(), std::ios::cur);
    }
    else if (std::size_t(stream.tellg()) != info.offset + file_data_header.length())
    {
      stream.seekg(info.offset + file_data_header.length(), std::ios::beg);
    }
  }

  void cln_file_archive::extract_file_contents(std::istream& stream,
    const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>>) const
  {
    if (info.compression_type == compression_type::none)
    {
      auto current_position = stream.tellg();
      stream.seekg(0, std::ios::end);
      auto last_byte = static_cast<std::size_t>(stream.tellg());

      stream.seekg(current_position, std::ios::beg);

      set_stream_position(stream, info);

      const auto remaining_bytes = last_byte - info.offset - file_data_header.length();

      std::copy_n(std::istreambuf_iterator(stream),
        info.size > remaining_bytes ? remaining_bytes : info.size,
        std::ostreambuf_iterator(output));
    }
    else if (info.compressed_size.has_value())
    {
      set_stream_position(stream, info);
      std::vector<std::byte> temp(info.compressed_size.value(), std::byte{'\0'});

      stream.read(reinterpret_cast<char*>(temp.data()), info.compressed_size.value());

      if (info.compressed_size.value() % 2 != 0)
      {
        temp.emplace_back(std::byte{'\0'});
      }

      std::vector<std::byte> buffer;
      for (auto i = 0u; i < temp.size(); i += 2)
      {
        auto count = std::size_t(temp[i]);
        auto& value = temp[i + 1];
        buffer.assign(count, value);
        output.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
      }
    }
  }
}// namespace studio::resources::vol::three_space
