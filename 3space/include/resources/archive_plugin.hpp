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

  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

  inline std::vector<archive_plugin::content_info> get_all_content(const std::filesystem::path& src_path, std::istream& archive, const archive_plugin& plugin)
  {
    auto content_listing = plugin.get_content_listing(archive, { src_path, src_path });

    auto all_content = content_listing;
    all_content.reserve(all_content.capacity() * 4);

    std::function<void(const decltype(content_listing)&)> visit_listing = [&](const auto& content_listing) {
      for (auto& entry : content_listing)
      {
        std::visit(overloaded {
                     [&](const studio::resources::folder_info& arg) {
                       auto child_listing = plugin.get_content_listing(archive, { src_path, arg.full_path });

                       if (all_content.size() + child_listing.size() > all_content.capacity())
                       {
                         all_content.reserve(all_content.capacity() + all_content.size() + child_listing.size());
                       }

                       std::copy(child_listing.begin(), child_listing.end(), std::back_inserter(all_content));
                       visit_listing(child_listing);
                     },
                     [](const studio::resources::file_info& arg) {
                     }
                   }, entry);
      }
    };

    visit_listing(content_listing);

    return all_content;
  }

  template<typename ContentType>
  inline std::vector<archive_plugin::file_info> get_all_content_of_type(const std::filesystem::path& src_path, std::istream& archive, const archive_plugin& plugin)
  {
    auto all_content = get_all_content(src_path, archive, plugin);

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