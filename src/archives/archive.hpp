#ifndef DARKSTARDTSCONVERTER_ARCHIVE_SHARED_HPP
#define DARKSTARDTSCONVERTER_ARCHIVE_SHARED_HPP

#include <istream>
#include <ostream>
#include <string>
#include <optional>
#include <variant>
#include <filesystem>
#include "../shared.hpp"

namespace shared::archive
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
    std::string filename;
    std::size_t offset;
    std::size_t size;
    compression_type compression_type;
    std::filesystem::path folder_path;
  };

  struct folder_info
  {
    std::string name;
    std::optional<std::size_t> file_count;
    std::filesystem::path full_path;
  };

  using is_supported_func = bool(std::basic_istream<std::byte>&);

  struct file_archive
  {
    virtual bool stream_is_supported(std::basic_istream<std::byte>&) = 0;

    virtual std::vector<std::variant<folder_info, file_info>> get_content_info(std::basic_istream<std::byte>&, std::filesystem::path) = 0;

    virtual void set_stream_position(std::basic_istream<std::byte>&, const file_info&) = 0;

    virtual void extract_file_contents(std::basic_istream<std::byte>&, const file_info&, std::basic_ostream<std::byte>&) = 0;

    virtual ~file_archive() = default;
    file_archive() = default;
    file_archive(const file_archive&) = delete;
    file_archive(file_archive&&) = delete;
  };
}

#endif