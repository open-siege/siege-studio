#ifndef DARKSTARDTSCONVERTER_ARCHIVE_SHARED_HPP
#define DARKSTARDTSCONVERTER_ARCHIVE_SHARED_HPP

#include <istream>
#include <ostream>
#include <string>
#include <optional>
#include <variant>
#include <filesystem>
#include "../shared.hpp"

namespace studio::resources
{
  enum class compression_type
  {
    none,
    rle,
    lz,
    lzh
  };

  struct file_info
  {
    std::filesystem::path filename;
    std::size_t offset;
    std::size_t size;
    std::optional<std::size_t> compressed_size;
    compression_type compression_type;
    std::filesystem::path folder_path;
  };

  struct folder_info
  {
    std::string name;
    std::optional<std::size_t> file_count;
    std::filesystem::path full_path;
  };

  struct listing_query
  {
    std::filesystem::path archive_path;
    std::filesystem::path folder_path;
  };

  struct archive_plugin
  {
    using folder_info = studio::resources::folder_info;
    using file_info = studio::resources::file_info;
    using content_info = std::variant<folder_info, studio::resources::file_info>;

    virtual bool stream_is_supported(std::basic_istream<std::byte>&) const = 0;

    virtual std::vector<content_info> get_content_listing(std::basic_istream<std::byte>&, const listing_query& query) const = 0;

    virtual void set_stream_position(std::basic_istream<std::byte>&, const file_info&) const = 0;

    virtual void extract_file_contents(std::basic_istream<std::byte>&, const file_info&, std::basic_ostream<std::byte>&) const = 0;

    virtual ~archive_plugin() = default;
    archive_plugin() = default;
    archive_plugin(const archive_plugin&) = delete;
    archive_plugin(archive_plugin&&) = delete;
  };
}

#endif