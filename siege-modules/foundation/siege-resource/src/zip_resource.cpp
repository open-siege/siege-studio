#include <fstream>
#include <filesystem>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <zip.h>

#include "siege/resource/zip_resource.hpp"
#include <siege/platform/stream.hpp>

namespace fs = std::filesystem;

namespace siege::resource::zip
{
  using folder_info = siege::platform::folder_info;

  constexpr auto file_record_tag = platform::to_tag<4>({ 'P', 'K', 0x03, 0x04 });
  constexpr auto folder_record_tag = platform::to_tag<4>({ 'P', 'K', 0x01, 0x02 });
  constexpr auto end_record_tag = platform::to_tag<4>({ 'P', 'K', 0x05, 0x06 });

  bool stream_is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char*>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == file_record_tag || tag == folder_record_tag || tag == end_record_tag;
  }


  struct stream_info
  {
    std::istream& stream;
    std::size_t start_offset;
    std::size_t stream_size;
  };

  static auto process_zip_stream = [](void* userdata, void* data, zip_uint64_t len, zip_source_cmd_t cmd) -> zip_int64_t {
    auto* og_stream = reinterpret_cast<stream_info*>(userdata);
    switch (cmd)
    {
    case ZIP_SOURCE_OPEN:
      return 0;
    case ZIP_SOURCE_READ: {
      if (og_stream->stream.eof())
      {
        return 0;
      }

      og_stream->stream.read(reinterpret_cast<char*>(data), len);

      return len;
    }
    case ZIP_SOURCE_CLOSE:
      return 0;
    case ZIP_SOURCE_STAT: {
      zip_stat_t* st = reinterpret_cast<zip_stat_t*>(data);
      zip_stat_init(st);

      st->comp_size = og_stream->stream_size;
      st->size = st->comp_size;
      st->comp_method = ZIP_CM_DEFAULT;
      st->encryption_method = ZIP_EM_NONE;
      st->valid = ZIP_STAT_MTIME | ZIP_STAT_SIZE | ZIP_STAT_COMP_SIZE | ZIP_STAT_COMP_METHOD | ZIP_STAT_ENCRYPTION_METHOD;

      return sizeof(zip_stat_t);
    }
    case ZIP_SOURCE_ERROR:
      return 0;
    case ZIP_SOURCE_SEEK: {
      zip_error_t error;
      zip_source_args_seek* seek_data = ZIP_SOURCE_GET_ARGS(zip_source_args_seek, data, len, &error);

      if (seek_data->whence == SEEK_SET)
      {
        og_stream->stream.seekg(og_stream->start_offset + seek_data->offset, std::ios_base::beg);
      }
      else if (seek_data->whence == SEEK_CUR)
      {
        og_stream->stream.seekg(seek_data->offset, std::ios_base::cur);
      }
      else if (seek_data->whence == SEEK_END)
      {
        og_stream->stream.seekg(seek_data->offset, std::ios_base::end);
      }
      return 0;
    }
    case ZIP_SOURCE_TELL: {
      auto curr = std::size_t(og_stream->stream.tellg());

      if (curr < og_stream->start_offset)
      {
        curr = og_stream->start_offset;
        og_stream->stream.seekg(og_stream->start_offset);
      }

      return curr - og_stream->start_offset;
    }
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


  struct stream_cache
  {
    std::size_t start_offset;
    std::size_t stream_size;
    std::vector<cached_zip_stat> stat_cache;
  };

  struct zip_cache
  {
    std::unordered_map<std::string, stream_cache> stream_cache;
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

  std::vector<zip_resource_reader::content_info> get_content_listing(std::any& cache, std::istream& stream, const platform::listing_query& query)
  {
    platform::istream_pos_resetter resetter(stream);
    auto& zip_cache = cache_as_zip_cache(cache);

    auto cache_key = fs::exists(query.archive_path) ? fs::path(query.archive_path).make_preferred().string() + std::to_string(fs::file_size(query.archive_path)) : fs::path(query.archive_path).make_preferred().string();

    if (cache_key.empty())
    {
      auto path = siege::platform::get_stream_path(stream);
      if (path)
      {
        cache_key = path->string();
      }
    }

    auto cache_entry = zip_cache.stream_cache.find(cache_key);

    if (cache_entry == zip_cache.stream_cache.end())
    {
      zip_error_t src_error;

      stream_info info{
        .stream = stream,
        .start_offset = (std::size_t)stream.tellg(),
        .stream_size = siege::platform::get_stream_size(stream) - (std::size_t)stream.tellg()
      };

      auto* source = zip_source_function_create(process_zip_stream, &info, &src_error);

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

      cache_entry = zip_cache.stream_cache.emplace(cache_key,
                                            stream_cache{ .start_offset = info.start_offset, .stream_size = info.stream_size, .stat_cache = std::move(entries) })
                      .first;

      zip_close(zip_file);
    }

    std::vector<zip_resource_reader::content_info> results;
    for (auto& entry : cache_entry->second.stat_cache)
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

      platform::file_info temp{};

      temp.size = entry.size;
      temp.filename = name.filename();
      temp.folder_path = query.folder_path;
      temp.archive_path = query.archive_path;
      temp.compression_type = platform::compression_type::lz77_huffman;

      results.emplace_back(std::move(temp));
    }

    return results;
  }

  void extract_file_contents(std::any& cache, std::istream& stream, const siege::platform::file_info& info, std::ostream& output)
  {
    auto& zip_cache = cache_as_zip_cache(cache);

    auto create_archive = [&]() {
      zip_error_t src_error;

      auto cache_key = fs::exists(info.archive_path) ? fs::path(info.archive_path).make_preferred().string() + std::to_string(fs::file_size(info.archive_path)) : fs::path(info.archive_path).make_preferred().string();

      if (cache_key.empty())
      {
        auto path = siege::platform::get_stream_path(stream);
        if (path)
        {
          cache_key = path->string();
        }
      }

      auto* info = new stream_info{
        .stream = stream,
        .start_offset = (std::size_t)stream.tellg(),
        .stream_size = siege::platform::get_stream_size(stream) - (std::size_t)stream.tellg()
      };

      auto cached_info = zip_cache.stream_cache.find(cache_key);

      if (cached_info != zip_cache.stream_cache.end())
      {
        info->start_offset = cached_info->second.start_offset;
        info->stream_size = cached_info->second.stream_size;
      }

      auto* source = zip_source_function_create(process_zip_stream, info, &src_error);

      zip_error_t err;
      return std::shared_ptr<zip_t>(zip_open_from_source(source, 0, &err), [info](auto* handle) {
        zip_close(handle);
        delete info;
      });
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
