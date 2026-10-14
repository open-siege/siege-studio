export module siege.platform.resource;

import std;

namespace siege::platform
{
  export enum class compression_type
  {
    none,
    size_rle,
    code_rle,
    lz77,
    lz78,
    lzss,
    lzw,
    lz77_huffman,
    lz78_huffman,
    lzss_huffman,
    cdxa,
    custom
  };

  export struct file_info
  {
    std::filesystem::path filename;
    std::size_t offset;
    std::size_t size;
    std::optional<std::size_t> compressed_size;
    siege::platform::compression_type compression_type;
    std::filesystem::path folder_path;
    std::filesystem::path archive_path;
    std::any metadata;

    std::filesystem::path relative_path() const
    {
      if (folder_path == archive_path)
      {
        return std::filesystem::path{};
      }
      std::error_code last_error;
      auto result = std::filesystem::relative(folder_path, archive_path, last_error);

      if (last_error)
      {
        auto new_path = folder_path.u8string();
        new_path.replace(0, archive_path.u8string().size(), u8"");

        if (!new_path.empty() && new_path.front() == '\\')
        {
          new_path.erase(0, 1);
        }

        return std::filesystem::path(new_path);
      }

      return result;
    }
  };

  export struct folder_info
  {
    std::string name;
    std::optional<std::size_t> file_count;
    std::filesystem::path full_path;
    std::filesystem::path archive_path;
  };

  export using content_info = std::variant<folder_info, file_info>;

  export struct listing_query
  {
    std::filesystem::path archive_path;
    std::filesystem::path folder_path;
  };

  export struct batch_storage
  {
    std::unordered_map<std::string_view, std::variant<void*, std::shared_ptr<void>>> temp;
  };

  export struct resource_writer
  {
    using folder_info = siege::platform::folder_info;
    using file_info = siege::platform::file_info;
    using content_info = std::variant<folder_info, siege::platform::file_info>;

    virtual void write_content(std::ostream&,
      std::vector<content_info>,
      std::optional<std::reference_wrapper<platform::batch_storage>> = std::nullopt) const = 0;

    virtual ~resource_writer() = default;
    resource_writer() = default;
    resource_writer(const resource_writer&) = delete;
    resource_writer(resource_writer&&) = delete;
  };

  export struct resource_write_context;

  export struct resource_reader
  {
    using folder_info = siege::platform::folder_info;
    using file_info = siege::platform::file_info;
    using content_info = std::variant<folder_info, siege::platform::file_info>;

    bool (&is_stream_supported)(std::istream&);

    std::vector<content_info> (&get_content_listing)(std::any&, std::istream&, const platform::listing_query& query);

    void (*set_stream_position)(std::istream&, const file_info&) = nullptr;

    void (&extract_file_contents)(std::any&, std::istream&, const file_info&, std::ostream&);

    std::list<platform::file_info> get_all_files_for_query(std::any& cache, std::istream&, const platform::listing_query& query) const;
  };

  export struct resource_reader_context;

  export template<class... Ts>
  struct overloaded : Ts...
  {
    using Ts::operator()...;
  };

  export auto make_auto_remove_path(std::filesystem::path some_path = "")
  {
    auto value = some_path.string().empty() ? nullptr : new std::filesystem::path(std::move(some_path));

    return std::shared_ptr<std::filesystem::path>{
      value,
      [](std::filesystem::path* value) {
        if (value)
        {
          std::error_code unused;
          std::filesystem::remove_all(*value, unused);
          delete value;
        }
      }
    };
  }

std::list<platform::file_info> resource_reader::get_all_files_for_query(std::any& cache, std::istream& stream, const platform::listing_query& query) const
  {
    std::list<platform::file_info> results;
    auto top_level_items = this->get_content_listing(cache, stream, query);

    std::function<void(decltype(top_level_items)&)> get_full_listing = [&](std::vector<resource_reader::content_info>& items) mutable {
      for (resource_reader::content_info& info : items)
      {
        if (auto parent_info = std::get_if<folder_info>(&info); parent_info)
        {
          std::vector<resource_reader::content_info> children;
          children = this->get_content_listing(cache, stream, listing_query{ .archive_path = query.archive_path, .folder_path = parent_info->full_path });
          get_full_listing(children);
        }

        if (auto leaf_info = std::get_if<file_info>(&info); leaf_info)
        {
          results.emplace_back(*leaf_info);
        }
      }
    };

    get_full_listing(top_level_items);
    
    return results;
  }

}// namespace siege::platform
