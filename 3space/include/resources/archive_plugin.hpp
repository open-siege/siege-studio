#ifndef DARKSTARDTSCONVERTER_ARCHIVE_SHARED_HPP
#define DARKSTARDTSCONVERTER_ARCHIVE_SHARED_HPP

#include <istream>
#include <ostream>
#include <string>
#include <optional>
#include <variant>
#include <filesystem>
#include <unordered_map>
#include <memory>
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
    resources::compression_type compression_type;
    std::filesystem::path folder_path;
    std::filesystem::path archive_path;
  };

  struct folder_info
  {
    std::string name;
    std::optional<std::size_t> file_count;
    std::filesystem::path full_path;
    std::filesystem::path archive_path;
  };

  struct listing_query
  {
    std::filesystem::path archive_path;
    std::filesystem::path folder_path;
  };

  struct batch_storage
  {
    std::unordered_map<std::string_view, std::variant<void*, std::shared_ptr<void>>> temp;
  };

  struct archive_plugin
  {
    using folder_info = studio::resources::folder_info;
    using file_info = studio::resources::file_info;
    using content_info = std::variant<folder_info, studio::resources::file_info>;

    virtual bool stream_is_supported(std::istream&) const = 0;

    virtual std::vector<content_info> get_content_listing(std::istream&, const listing_query& query) const = 0;

    virtual void set_stream_position(std::istream&, const file_info&) const = 0;

    virtual void extract_file_contents(std::istream&,
      const file_info&,
      std::ostream&,
      std::optional<std::reference_wrapper<batch_storage>> = std::nullopt) const = 0;

    virtual ~archive_plugin() = default;
    archive_plugin() = default;
    archive_plugin(const archive_plugin&) = delete;
    archive_plugin(archive_plugin&&) = delete;
  };
}

#endif