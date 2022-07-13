#include <array>
#include <vector>
#include <fstream>
#include <utility>
#include <string>
#include <cstdlib>

#include "resources/cyclone_volume.hpp"

namespace studio::resources::cln
{
  namespace endian = boost::endian;


  bool cln_file_archive::is_supported(std::basic_istream<std::byte>& stream)
  {
    endian::little_int32_t file_count;
    stream.read(reinterpret_cast<std::byte*>(&file_count), sizeof(file_count));

    endian::little_int32_t folder_name_length;
    stream.read(reinterpret_cast<std::byte*>(&folder_name_length), sizeof(folder_name_length));

    stream.seekg(-int(sizeof(file_count)) * 2, std::ios::cur);

    return file_count <= 6000 == folder_name_length <= 256;
  }

  bool cln_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream) const
  {
    return is_supported(stream);
  }

  struct file_entry
  {
    endian::little_uint32_t string_length;
    std::string filename;
    endian::little_uint32_t file_size;
    endian::little_uint32_t is_folder;
    endian::little_uint32_t offset;
  };

  auto get_file_entries(std::basic_istream<std::byte>& stream)
  {
      std::vector<file_entry> entries;

      endian::little_int32_t file_count;
      stream.read(reinterpret_cast<std::byte*>(&file_count), sizeof(file_count));

      entries.reserve(file_count);

      for (auto i = 0; i < file_count; i++)
      {
        auto& entry = entries.emplace_back();
        stream.read(reinterpret_cast<std::byte*>(&entry.string_length), sizeof(entry.string_length));
        entry.filename.assign(entry.string_length, '\0');

        stream.read(reinterpret_cast<std::byte*>(entry.filename.data()), entry.string_length);

        stream.read(reinterpret_cast<std::byte*>(&entry.file_size), sizeof(entry.file_size));
        stream.read(reinterpret_cast<std::byte*>(&entry.is_folder), sizeof(entry.is_folder));
        stream.read(reinterpret_cast<std::byte*>(&entry.offset), sizeof(entry.offset));
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

  std::vector<cln_file_archive::content_info> cln_file_archive::get_content_listing(std::basic_istream<std::byte>& stream, const listing_query& query) const
  {
    auto entries = get_file_entries(stream);

    std::vector<cln_file_archive::content_info> results;
    results.reserve(16);

    if (query.archive_path == query.folder_path)
    {
      for (const auto& entry : entries)
      {
        if (entry.is_folder != 1)
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
      if (entry.is_folder == 1)
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
          if (entry.is_folder == 1)
          {
            studio::resources::folder_info temp{};
            temp.full_path = full_path;
            temp.name = full_path.filename().string();

            results.emplace_back(temp);
          }
          else
          {
            file_info temp{};
            temp.size = entry.file_size;
            temp.offset = entry.offset;
            temp.filename = full_path.filename();
            temp.folder_path = full_path.parent_path();
            temp.compression_type = compression_type::none;

            results.emplace_back(temp);
          }
        }
      }
    }

    return results;
  }

  constexpr auto file_data_header = std::string_view("MAGIC_NUMBER");

  void cln_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const
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

  void cln_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info, std::basic_ostream<std::byte>& output) const
  {
    auto current_position = stream.tellg();
    stream.seekg(0, std::ios::end);
    auto last_byte = static_cast<std::size_t>(stream.tellg());

    stream.seekg(current_position, std::ios::beg);

    set_stream_position(stream, info);

    const auto remaining_bytes = last_byte - info.offset - file_data_header.length();

    std::copy_n(std::istreambuf_iterator<std::byte>(stream),
      info.size > remaining_bytes ? remaining_bytes : info.size,
      std::ostreambuf_iterator<std::byte>(output));
  }
}// namespace studio::resources::vol::three_space
