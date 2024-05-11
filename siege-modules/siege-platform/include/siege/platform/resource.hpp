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
#include <algorithm>
#include <functional>
#include <siege/platform/shared.hpp>

namespace siege::platform
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

  struct resource_reader
  {
    using folder_info = siege::platform::folder_info;
    using file_info = siege::platform::file_info;
    using content_info = std::variant<folder_info, siege::platform::file_info>;

    virtual bool stream_is_supported(std::istream&) const = 0;

    virtual std::vector<content_info> get_content_listing(std::istream&, const platform::listing_query& query) const = 0;

    virtual void set_stream_position(std::istream&, const file_info&) const = 0;

    virtual void extract_file_contents(std::istream&,
      const file_info&,
      std::ostream&,
      std::optional<std::reference_wrapper<platform::batch_storage>> = std::nullopt) const = 0;

    virtual ~resource_reader() = default;
    resource_reader() = default;
    resource_reader(const resource_reader&) = delete;
    resource_reader(resource_reader&&) = delete;
  };

  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  inline std::vector<resource_reader::content_info> get_all_content(const std::filesystem::path& src_path, std::istream& archive, const resource_reader& plugin)
  {
    auto content_listing = plugin.get_content_listing(archive, { src_path, src_path });

    auto all_content = content_listing;
    all_content.reserve(all_content.capacity() * 4);

    std::function<void(const decltype(content_listing)&)> visit_listing = [&](const auto& content_listing) {
      for (auto& entry : content_listing)
      {
        std::visit(overloaded {
                     [&](const siege::platform::folder_info& arg) {
                       auto child_listing = plugin.get_content_listing(archive, { src_path, arg.full_path });

                       if (all_content.size() + child_listing.size() + 1 > all_content.capacity())
                       {
                         all_content.reserve(all_content.capacity() + all_content.size() + child_listing.size() + 1);
                       }

                       all_content.emplace_back(arg);
                       std::copy(child_listing.begin(), child_listing.end(), std::back_inserter(all_content));
                       visit_listing(child_listing);
                     },
                     [](const siege::platform::file_info& arg) {
                     }
                   }, entry);
      }
    };

    visit_listing(content_listing);

    return all_content;
  }

  template<typename ContentType>
  inline std::vector<resource_reader::file_info> unwrap_content_of_type(const std::vector<resource_reader::content_info>& all_content)
  {
    std::vector<ContentType> files;
    files.reserve(std::count_if(all_content.begin(), all_content.end(),
      [&](auto& value) {
        return std::holds_alternative<ContentType>(value);
      }));

    for (auto& content : all_content)
    {
      if (std::holds_alternative<ContentType>(content))
      {
        files.emplace_back(std::get<ContentType>(content));
      }
    }

    return files;
  }

  template<typename ContentType>
  inline std::vector<resource_reader::file_info> get_all_content_of_type(const std::filesystem::path& src_path, std::istream& archive, const resource_reader& plugin)
  {
    return unwrap_content_of_type<ContentType>(get_all_content(src_path, archive, plugin));
  }

  inline auto make_auto_remove_path(std::filesystem::path some_path = "")
  {
    auto value = some_path.string().empty() ? nullptr : new std::filesystem::path(std::move(some_path));

    return std::unique_ptr<std::filesystem::path, void(*)(std::filesystem::path*)> {
      value,
      [](std::filesystem::path* value) {
        if (value) {
          std::error_code unused;
          std::filesystem::remove_all(*value, unused);
          delete value;
        }
      }
    };
  }
}

#endif