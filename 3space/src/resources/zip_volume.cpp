#include <fstream>
#include <filesystem>
#include <vector>
#include <utility>
#include <zip.h>

#include "resources/zip_volume.hpp"

namespace studio::resources::zip
{
  using folder_info = studio::resources::folder_info;

  constexpr auto file_record_tag = shared::to_tag<4>({ 'P', 'K', 0x03, 0x04 });
  constexpr auto folder_record_tag = shared::to_tag<4>({ 'P', 'K', 0x01, 0x02 });
  constexpr auto end_record_tag = shared::to_tag<4>({ 'P', 'K', 0x05, 0x06 });

  bool zip_file_archive::is_supported(std::basic_istream<std::byte>& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(tag.data(), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == file_record_tag || tag == folder_record_tag || tag == end_record_tag;
  }

  bool zip_file_archive::stream_is_supported(std::basic_istream<std::byte>& stream) const
  {
    return is_supported(stream);
  }

  std::vector<zip_file_archive::content_info> zip_file_archive::get_content_listing(std::basic_istream<std::byte>& stream, std::filesystem::path archive_or_folder_path) const
  {
    if (!std::filesystem::exists(archive_or_folder_path))
    {
      return {};
    }

    std::vector<zip_file_archive::content_info> results;
    zip_t* zip_file;


    int err = 0;
    zip_file = zip_open(archive_or_folder_path.string().c_str(), 0, &err);

    auto entry_count = zip_get_num_entries(zip_file, 0);

    results.reserve(entry_count);

    for (decltype(entry_count) i = 0; i < entry_count; ++i)
    {
      struct zip_stat st;
      zip_stat_init(&st);
      zip_stat_index(zip_file, i, 0, &st);

      file_info temp{};

      temp.size = st.size;
      temp.filename = st.name;
      temp.folder_path = archive_or_folder_path;
      temp.compression_type = compression_type::lz;
      results.emplace_back(temp);
    }

    zip_close(zip_file);

    return results;
  }

  void zip_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const
  {

  }

  void zip_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info, std::basic_ostream<std::byte>& output) const
  {
    zip_t* archive;


    int err = 0;
    archive = zip_open(info.folder_path.string().c_str(), 0, &err);


    zip_file *entry = zip_fopen(archive, info.filename.string().c_str(), 0);

    std::vector<std::byte> contents(info.size);

    zip_fread(entry, contents.data(), info.size);

    std::copy_n(contents.data(),
      info.size,
      std::ostreambuf_iterator<std::byte>(output));

    zip_fclose(entry);

    zip_close(archive);
  }
}// namespace darkstar::vol
