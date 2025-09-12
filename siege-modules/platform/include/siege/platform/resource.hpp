#ifndef SIEGE_PLATFORM_RESOURCE_HPP
#define SIEGE_PLATFORM_RESOURCE_HPP

#include <istream>
#include <ostream>
#include <string>
#include <optional>
#include <variant>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <any>
#include <functional>
#include <siege/platform/shared.hpp>

namespace siege::platform
{
  enum class compression_type
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

  struct file_info
  {
    std::filesystem::path filename;
    std::size_t offset;
    std::size_t size;
    std::optional<std::size_t> compressed_size;
    siege::platform::compression_type compression_type;
    std::filesystem::path folder_path;
    std::filesystem::path archive_path;
    std::any metadata;

    inline std::filesystem::path relative_path() const
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

  struct folder_info
  {
    std::string name;
    std::optional<std::size_t> file_count;
    std::filesystem::path full_path;
    std::filesystem::path archive_path;
  };

  using content_info = std::variant<folder_info, file_info>;

  struct listing_query
  {
    std::filesystem::path archive_path;
    std::filesystem::path folder_path;
  };

  struct batch_storage
  {
    std::unordered_map<std::string_view, std::variant<void*, std::shared_ptr<void>>> temp;
  };

  struct resource_writer
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

  struct resource_write_context;

  struct resource_reader
  {
    using folder_info = siege::platform::folder_info;
    using file_info = siege::platform::file_info;
    using content_info = std::variant<folder_info, siege::platform::file_info>;

    bool (&stream_is_supported)(std::istream&);

    std::vector<content_info> (&get_content_listing)(std::any&, std::istream&, const platform::listing_query& query);

    void (*set_stream_position)(std::istream&, const file_info&) = nullptr;

    void (&extract_file_contents)(std::any&, std::istream&, const file_info&, std::ostream&);

    std::list<platform::file_info> get_all_files_for_query(std::any& cache, std::istream&, const platform::listing_query& query) const;
  };

  struct resource_reader_context;

  template<class... Ts>
  struct overloaded : Ts...
  {
    using Ts::operator()...;
  };
  template<class... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;

  inline auto make_auto_remove_path(std::filesystem::path some_path = "")
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
}// namespace siege::platform

#endif