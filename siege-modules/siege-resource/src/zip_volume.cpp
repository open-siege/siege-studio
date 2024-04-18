#include <fstream>
#include <filesystem>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <zip.h>

#include "siege/resource/zip_volume.hpp"

namespace fs = std::filesystem;

namespace siege::resource::zip
{
  using folder_info = siege::platform::folder_info;

  constexpr auto file_record_tag = platform::to_tag<4>({ 'P', 'K', 0x03, 0x04 });
  constexpr auto folder_record_tag = platform::to_tag<4>({ 'P', 'K', 0x01, 0x02 });
  constexpr auto end_record_tag = platform::to_tag<4>({ 'P', 'K', 0x05, 0x06 });

  bool zip_file_archive::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == file_record_tag || tag == folder_record_tag || tag == end_record_tag;
  }

  bool zip_file_archive::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  static auto process_zip_stream = [](void *userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd) -> zip_int64_t
  {
    auto* og_stream = reinterpret_cast<std::istream*>(userdata);
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

      og_stream->read(reinterpret_cast<char*>(data), len);

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

  std::vector<zip_file_archive::content_info> zip_file_archive::get_content_listing(std::istream& stream, const platform::listing_query& query) const
  {
    static std::list<std::string> name_cache;
    static std::unordered_map<std::string, std::vector<struct zip_stat>> stat_cache;

    auto cache_key = fs::exists(query.archive_path) ?
                                                    query.archive_path.string() + std::to_string(fs::file_size(query.archive_path)) :
                                                    query.archive_path.string();

    auto cache_entry = stat_cache.find(cache_key);

    if (cache_entry == stat_cache.end())
    {
      zip_error_t src_error;
      auto* source = zip_source_function_create(process_zip_stream, &stream, &src_error);

      std::vector<struct zip_stat> entries;
      zip_t* zip_file;

      zip_error_t err;
      zip_file = zip_open_from_source(source, 0, &err);

      auto entry_count = zip_get_num_entries(zip_file, 0);

      if (entry_count == -1)
      {
        zip_close(zip_file);
        return {};
      }

      entries.reserve(entry_count);

      for (decltype(entry_count) i = 0; i < entry_count; ++i)
      {
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat_index(zip_file, i, 0, &st);

        if (st.name)
        {
          st.name = name_cache.emplace_back(st.name).c_str();
          entries.emplace_back(st);
        }
      }

      cache_entry = stat_cache.emplace(cache_key, std::move(entries)).first;

      zip_close(zip_file);
    }

    std::vector<zip_file_archive::content_info> results;
    for (auto& entry : cache_entry->second)
    {
      std::string_view name_str(entry.name);
      std::filesystem::path name = !name_str.empty() && name_str[name_str.size() - 1] == '/' ? name_str.substr(0, name_str.size() - 1) : name_str;

      auto relative_path = fs::relative(query.folder_path, query.archive_path);

      if (relative_path.string() == ".")
      {
        relative_path = "";
      }

      auto parent_path = name.parent_path();

      if (parent_path != relative_path)
      {
        continue;
      }

      if (entry.size == 0)
      {
        folder_info temp{};

        temp.name = name.filename().string();
        temp.full_path = query.archive_path / name;
        temp.archive_path = query.archive_path;
        results.emplace_back(std::move(temp));
        continue;
      }

      file_info temp{};

      temp.size = entry.size;
      temp.filename = name.filename();
      temp.folder_path = query.folder_path;
      temp.archive_path = query.archive_path;
      temp.compression_type = platform::compression_type::lz;

      results.emplace_back(std::move(temp));
    }

    return results;
  }

  void zip_file_archive::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {

  }

  void zip_file_archive::extract_file_contents(std::istream& stream,
    const siege::platform::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage) const
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

    auto full_path = info.folder_path != info.archive_path ? std::filesystem::relative(info.folder_path, info.archive_path) / info.filename : info.filename;

    auto full_path_str = full_path.string();

    for (char & i : full_path_str)
    {
      if (i == '\\')
      {
        i = '/';
      }
    }

    file_ptr entry = file_ptr(zip_fopen(archive.get(), full_path_str.c_str(), 0), [](zip_file* file){ zip_fclose(file); });

    std::vector<std::byte> contents(info.size);

    zip_fread(entry.get(), contents.data(), info.size);

    std::copy_n(reinterpret_cast<char *>(contents.data()),
      info.size,
      std::ostreambuf_iterator(output));
  }
}// namespace darkstar::vol
