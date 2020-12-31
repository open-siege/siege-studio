#ifndef DARKSTARDTSCONVERTER_FILE_SYSTEM_ARCHIVE_HPP
#define DARKSTARDTSCONVERTER_FILE_SYSTEM_ARCHIVE_HPP

#include <filesystem>
#include <memory>
#include <map>
#include <algorithm>
#include <locale>
#include <fstream>
#include "archive.hpp"


namespace studio::fs
{
  struct null_buffer : public std::basic_streambuf<std::byte>
  {
    int overflow(int c) { return c; }
  };

  std::unique_ptr<std::basic_istream<std::byte>> open_stream(std::optional<std::filesystem::path> archive_path)
  {
    static studio::fs::null_buffer buffer;

    if (!archive_path.has_value())
    {
      return std::make_unique<std::basic_istream<std::byte>>(&buffer);
    }

    if (archive_path.has_value() && std::filesystem::is_directory(archive_path.value()))
    {
      return std::make_unique<std::basic_istream<std::byte>>(&buffer);
    }

    return std::make_unique<std::basic_ifstream<std::byte>>(archive_path.value(), std::ios::binary);
  }

  struct file_system_archive
  {
    std::multimap<std::string, std::unique_ptr<shared::archive::file_archive>> archive_types;

    std::locale default_locale;

    void add_archive_type(std::string extension, std::unique_ptr<shared::archive::file_archive> archive_type)
    {
      std::transform(extension.begin(), extension.end(), extension.begin(), [&](auto c) { return std::tolower(c, default_locale); });
      archive_types.insert(std::make_pair(std::move(extension), std::move(archive_type)));
    }

    std::filesystem::path get_archive_path(const std::filesystem::path& folder_path)
    {
      auto archive_path = folder_path;

      while (!std::filesystem::exists(archive_path) && !std::filesystem::is_directory(archive_path))
      {
        archive_path = archive_path.parent_path();
      }

      return archive_path;
    }

    bool is_regular_file(const std::filesystem::path& folder_path)
    {
      auto archive_path = get_archive_path(folder_path);

      if (archive_path == folder_path)
      {
        return !(std::filesystem::is_directory(folder_path) || get_archive_type(folder_path).has_value());
      }

      return folder_path.has_extension();
    }

    std::optional<std::reference_wrapper<shared::archive::file_archive>> get_archive_type(const std::filesystem::path& file_path)
    {
      auto ext = file_path.filename().extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), [&](char c) { return std::tolower(c, default_locale); });

      auto archive_type = archive_types.equal_range(ext);

      for (auto it = archive_type.first; it != archive_type.second; ++it)
      {
        auto file_stream = std::basic_ifstream<std::byte>{ file_path, std::ios::binary };

        if (it->second->stream_is_supported(file_stream))
        {
          return std::ref(*it->second);
        }
      }


      return std::nullopt;
    }

    void set_stream_position(const std::filesystem::path& archive_path,
      std::basic_istream<std::byte>& stream,
      const shared::archive::file_info& info)
    {
      auto archive = get_archive_type(archive_path);

      if (archive.has_value())
      {
        archive->get().set_stream_position(stream, info);
      }

    }

    std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> get_content_listing(const std::filesystem::path& folder_path)
    {
      std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> files;

      if (auto archive_type = get_archive_type(get_archive_path(folder_path)); archive_type.has_value())
      {
        auto file_stream = std::basic_ifstream<std::byte>{ get_archive_path(folder_path), std::ios::binary };

        return archive_type.value().get().get_content_listing(file_stream, folder_path);
      }

      for (auto& item : std::filesystem::directory_iterator(folder_path))
      {
        if (item.is_directory())
        {
          shared::archive::folder_info info{};
          info.name = item.path().filename().string();
          info.full_path = item.path();
          files.emplace_back(info);
        }
        else if (auto archive_type = get_archive_type(item.path()); archive_type.has_value())
        {
          shared::archive::folder_info info{};
          info.name = item.path().filename().string();
          info.full_path = item.path();
          files.emplace_back(info);
        }
        else
        {
          shared::archive::file_info info{};

          info.filename = item.path().filename().string();
          info.folder_path = item.path().parent_path();
          files.emplace_back(info);
        }
      }

      return files;
    }
  };
}// namespace studio::fs


#endif//DARKSTARDTSCONVERTER_FILE_SYSTEM_ARCHIVE_HPP
