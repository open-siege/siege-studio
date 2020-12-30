#ifndef DARKSTARDTSCONVERTER_FILE_SYSTEM_ARCHIVE_HPP
#define DARKSTARDTSCONVERTER_FILE_SYSTEM_ARCHIVE_HPP

#include <filesystem>
#include <memory>
#include <map>
#include <algorithm>
#include <locale>
#include "archive.hpp"


namespace studio::fs
{
  struct null_buffer : public std::basic_streambuf<std::byte>
  {
    int overflow(int c) { return c; }
  };

  struct file_system_archive : shared::archive::file_archive
  {
    std::map<std::string, std::unique_ptr<shared::archive::file_archive>> archive_types;

    std::locale default_locale;


    void add_archive_type(std::string extension, std::unique_ptr<shared::archive::file_archive> archive_type)
    {
      std::transform(extension.begin(), extension.end(), extension.begin(), [&](auto c) { return std::tolower(c, default_locale); });
      archive_types.insert(std::make_pair(std::move(extension), std::move(archive_type)));
    }

    bool stream_is_supported(std::basic_istream<std::byte>&) override
    {
      return false;
    }

    std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> get_content_info(std::basic_istream<std::byte>&, std::filesystem::path folder_path) override
    {
      std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> files;

      auto archive_path = folder_path;

      while (!std::filesystem::exists(archive_path))
      {
        archive_path = archive_path.parent_path();
      }

      auto ext = archive_path.filename().extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), [&](char c) { return std::tolower(c, default_locale); });

      if (auto archive_type = archive_types.find(ext); archive_type != archive_types.end())
      {
        auto file_stream = std::basic_ifstream<std::byte>{ archive_path, std::ios::binary };

        if (archive_type->second->stream_is_supported(file_stream))
        {
          return archive_type->second->get_content_info(file_stream, folder_path);
        }
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
        else
        {
          ext = item.path().filename().extension().string();
          std::transform(ext.begin(), ext.end(), ext.begin(), [&](auto c) { return std::tolower(c, default_locale); });

          if (auto archive_type = archive_types.find(ext); archive_type != archive_types.end())
          {
            auto file_stream = std::basic_ifstream<std::byte>{ item, std::ios::binary };

            if (archive_type->second->stream_is_supported(file_stream))
            {
              shared::archive::folder_info info{};
              info.name = item.path().filename().string();
              info.full_path = item.path();
              files.emplace_back(info);
            }
          }
          else
          {
            shared::archive::file_info info{};

            info.filename = item.path().filename().string();
            info.folder_path = item.path().parent_path();
            files.emplace_back(info);
          }
        }
      }

      return files;
    }

    void set_stream_position(std::basic_istream<std::byte>&, const shared::archive::file_info&) override
    {
    }

    void extract_file_contents(std::basic_istream<std::byte>&, const shared::archive::file_info&, std::basic_ostream<std::byte>&) override
    {
    }
  };
}// namespace studio::fs


#endif//DARKSTARDTSCONVERTER_FILE_SYSTEM_ARCHIVE_HPP
