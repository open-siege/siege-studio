#include <fstream>
#include <filesystem>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <zip.h>

#include "siege/resource/zip_resource.hpp"

namespace fs = std::filesystem;

namespace siege::resource::zip
{
  using folder_info = siege::platform::folder_info;

  constexpr auto file_record_tag = platform::to_tag<4>({ 'P', 'K', 0x03, 0x04 });
  constexpr auto folder_record_tag = platform::to_tag<4>({ 'P', 'K', 0x01, 0x02 });
  constexpr auto end_record_tag = platform::to_tag<4>({ 'P', 'K', 0x05, 0x06 });

  bool zip_resource_reader::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == file_record_tag || tag == folder_record_tag || tag == end_record_tag;
  }

  bool zip_resource_reader::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  static auto process_zip_stream = [](void* userdata, void* data, zip_uint64_t len, zip_source_cmd_t cmd) -> zip_int64_t {
    auto* og_stream = reinterpret_cast<std::istream*>(userdata);
    switch (cmd)
    {
    case ZIP_SOURCE_OPEN:
      return 0;
    case ZIP_SOURCE_READ: {
      if (og_stream->eof())
      {
        return 0;
      }

      og_stream->read(reinterpret_cast<char*>(data), len);

      return len;
    }
    case ZIP_SOURCE_CLOSE:
      return 0;
    case ZIP_SOURCE_STAT: {
      zip_stat_t* st = reinterpret_cast<zip_stat_t*>(data);
      zip_stat_init(st);
      auto current_pos = og_stream->tellg();

      og_stream->seekg(0, std::ios::end);

      st->comp_size = std::size_t(og_stream->tellg());
      st->size = st->comp_size;
      st->comp_method = ZIP_CM_DEFAULT;
      st->encryption_method = ZIP_EM_NONE;
      st->valid = ZIP_STAT_MTIME | ZIP_STAT_SIZE | ZIP_STAT_COMP_SIZE | ZIP_STAT_COMP_METHOD | ZIP_STAT_ENCRYPTION_METHOD;

      og_stream->seekg(current_pos, std::ios::beg);

      return sizeof(zip_stat_t);
    }
    case ZIP_SOURCE_ERROR:
      return 0;
    case ZIP_SOURCE_SEEK: {
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
        ZIP_SOURCE_FREE,
        ZIP_SOURCE_TELL,
        ZIP_SOURCE_SUPPORTS);
    default:
      return -1;
    }
  };

  struct cached_zip_stat : zip_stat
  {
    std::shared_ptr<std::string> name_storage;
  };

  struct zip_cache
  {
    std::unordered_map<std::string, std::vector<cached_zip_stat>> stat_cache;
    std::unordered_map<fs::path, std::shared_ptr<zip_t>> archives;
  };

  zip_cache& cache_as_zip_cache(std::any& cache)
  {
    if (cache.type() != typeid(zip_cache))
    {
      zip_cache temp{};
      cache = temp;
    }

    return std::any_cast<zip_cache&>(cache);
  }

  std::vector<zip_resource_reader::content_info> zip_resource_reader::get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query) const
  {
    platform::istream_pos_resetter resetter(stream);
    auto& zip_cache = cache_as_zip_cache(cache);

    auto cache_key = fs::exists(query.archive_path) ? fs::path(query.archive_path).make_preferred().string() + std::to_string(fs::file_size(query.archive_path)) : fs::path(query.archive_path).make_preferred().string();

    auto cache_entry = zip_cache.stat_cache.find(cache_key);

    if (cache_entry == zip_cache.stat_cache.end())
    {
      zip_error_t src_error;
      auto* source = zip_source_function_create(process_zip_stream, &stream, &src_error);

      std::vector<cached_zip_stat> entries;
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
        cached_zip_stat st;
        zip_stat_init(&st);
        zip_stat_index(zip_file, i, 0, &st);

        if (st.name)
        {
          st.name_storage = std::make_shared<std::string>(st.name);

          if (st.size == 0 && st.name_storage->ends_with('/'))
          {
            st.name_storage->pop_back();
          }

          st.name = st.name_storage->c_str();
          entries.emplace_back(std::move(st));
        }
      }

      cache_entry = zip_cache.stat_cache.emplace(cache_key, std::move(entries)).first;

      zip_close(zip_file);
    }

    std::vector<zip_resource_reader::content_info> results;
    for (auto& entry : cache_entry->second)
    {
      fs::path name(entry.name);
      name.make_preferred();

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
      temp.compression_type = platform::compression_type::lz77_huffman;

      results.emplace_back(std::move(temp));
    }

    return results;
  }

  void zip_resource_reader::set_stream_position(std::istream& stream, const siege::platform::file_info& info) const
  {
  }

  void zip_resource_reader::extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output) const
  {
    auto& zip_cache = cache_as_zip_cache(cache);

    auto create_archive = [&]() {
      zip_error_t src_error;
      auto* source = zip_source_function_create(process_zip_stream, &stream, &src_error);

      zip_error_t err;
      return std::shared_ptr<zip_t>(zip_open_from_source(source, 0, &err), zip_close);
    };

    if (!zip_cache.archives.contains(info.archive_path))
    {
      zip_cache.archives.emplace(info.archive_path, create_archive());
    }

    using file_ptr = std::unique_ptr<zip_file, void (*)(zip_file*)>;

    auto full_path = info.folder_path != info.archive_path ? std::filesystem::relative(info.folder_path, info.archive_path) / info.filename : info.filename;

    auto full_path_str = full_path.string();

    for (char& i : full_path_str)
    {
      if (i == '\\')
      {
        i = '/';
      }
    }

    file_ptr entry = file_ptr(zip_fopen(zip_cache.archives.at(info.archive_path).get(), full_path_str.c_str(), 0), [](zip_file* file) { zip_fclose(file); });

    std::vector<std::byte> contents(info.size);

    zip_fread(entry.get(), contents.data(), info.size);

    std::copy_n(reinterpret_cast<char*>(contents.data()),
      info.size,
      std::ostreambuf_iterator(output));
  }
}// namespace siege::resource::zip
