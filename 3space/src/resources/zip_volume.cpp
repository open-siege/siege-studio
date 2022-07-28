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

  static auto process_zip_stream = [](void *userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd) -> zip_int64_t
  {
    auto* og_stream = reinterpret_cast<std::basic_istream<std::byte>*>(userdata);
    switch (cmd)
    {
    case ZIP_SOURCE_OPEN:
      return 0;
    case ZIP_SOURCE_READ:
    {
      if (og_stream->eof())
      {
        return 0;
      }

      og_stream->read(reinterpret_cast<std::byte*>(data), len);

      return len;
    }
    case ZIP_SOURCE_CLOSE:
      return 0;
    case ZIP_SOURCE_STAT:
    {
      zip_stat_t *st = reinterpret_cast<zip_stat_t*>(data);
      zip_stat_init(st);
      auto current_pos = og_stream->tellg();

      og_stream->seekg(0, std::ios::end);

      st->comp_size = std::size_t(og_stream->tellg());
      st->size = st->comp_size;
      st->comp_method = ZIP_CM_STORE;
      st->encryption_method = ZIP_EM_NONE;
      st->valid = ZIP_STAT_MTIME | ZIP_STAT_SIZE | ZIP_STAT_COMP_SIZE | ZIP_STAT_COMP_METHOD | ZIP_STAT_ENCRYPTION_METHOD;

      og_stream->seekg(current_pos, std::ios::beg);

      return sizeof(zip_stat_t);
    }
    case ZIP_SOURCE_ERROR:
      return 0;
    case ZIP_SOURCE_SEEK:
    {
      zip_error_t error;
      zip_source_args_seek* seek_data = ZIP_SOURCE_GET_ARGS(zip_source_args_seek, data, len, &error);

      if (seek_data->whence == SEEK_SET)
      {
        og_stream->seekg(seek_data->offset, std::ios_base::beg);
      }
      else if (seek_data->whence == SEEK_CUR)
      {
        og_stream->seekg(seek_data->offset, std::ios_base::cur);
      }
      else if (seek_data->whence == SEEK_END)
      {
        og_stream->seekg(seek_data->offset, std::ios_base::end);
      }
      return 0;
    }
    case ZIP_SOURCE_TELL:
      return og_stream->tellg();
    case ZIP_SOURCE_SUPPORTS:
      return zip_source_make_command_bitmap(ZIP_SOURCE_OPEN,
        ZIP_SOURCE_READ,
        ZIP_SOURCE_CLOSE,
        ZIP_SOURCE_STAT,
        ZIP_SOURCE_ERROR,
        ZIP_SOURCE_SEEK,
        ZIP_SOURCE_TELL,
        ZIP_SOURCE_SUPPORTS);
    default:
      return -1;
    }
  };

  std::vector<zip_file_archive::content_info> zip_file_archive::get_content_listing(std::basic_istream<std::byte>& stream, const listing_query& query) const
  {
    zip_error_t src_error;
    auto* source = zip_source_function_create(process_zip_stream, &stream, &src_error);

    std::vector<zip_file_archive::content_info> results;
    zip_t* zip_file;


    zip_error_t err;
    zip_file = zip_open_from_source(source, 0, &err);

    auto entry_count = zip_get_num_entries(zip_file, 0);

    if (entry_count == -1)
    {
      return results;
    }

    results.reserve(entry_count);

    for (decltype(entry_count) i = 0; i < entry_count; ++i)
    {
      struct zip_stat st;
      zip_stat_init(&st);
      zip_stat_index(zip_file, i, 0, &st);

      file_info temp{};

      temp.size = st.size;
      temp.filename = st.name;
      temp.folder_path = query.folder_path;
      temp.compression_type = compression_type::lz;

      results.emplace_back(temp);
    }

    zip_close(zip_file);

    return results;
  }

  void zip_file_archive::set_stream_position(std::basic_istream<std::byte>& stream, const studio::resources::file_info& info) const
  {

  }

  void zip_file_archive::extract_file_contents(std::basic_istream<std::byte>& stream,
    const studio::resources::file_info& info,
    std::basic_ostream<std::byte>& output,
    std::optional<std::reference_wrapper<batch_storage>> storage) const
  {
    std::shared_ptr<zip_t> archive;

    auto create_archive = [&]() {
      zip_error_t src_error;
      auto* source = zip_source_function_create(process_zip_stream, &stream, &src_error);

      zip_error_t err;
      return std::shared_ptr<zip_t>(zip_open_from_source(source, 0, &err), zip_close);
    };

    if (storage.has_value())
    {
      auto zip_iter = storage.value().get().temp.find("zip_instance");

      if (zip_iter == storage.value().get().temp.end())
      {
        archive = create_archive();
        zip_iter = storage.value().get().temp.emplace("zip_instance", std::static_pointer_cast<void>(archive)).first;
      }

      archive = std::static_pointer_cast<zip_t>(std::get<std::shared_ptr<void>>(zip_iter->second));
    }
    else
    {
      archive = create_archive();
    }

    using file_ptr = std::unique_ptr<zip_file, void(*)(zip_file*)>;

    file_ptr entry = file_ptr(zip_fopen(archive.get(), info.filename.string().c_str(), 0), [](zip_file* file){ zip_fclose(file); });

    std::vector<std::byte> contents(info.size);

    zip_fread(entry.get(), contents.data(), info.size);

    std::copy_n(contents.data(),
      info.size,
      std::ostreambuf_iterator<std::byte>(output));

  }
}// namespace darkstar::vol
