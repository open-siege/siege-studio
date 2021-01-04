#ifndef DARKSTARDTSCONVERTER_FILE_SYSTEM_ARCHIVE_HPP
#define DARKSTARDTSCONVERTER_FILE_SYSTEM_ARCHIVE_HPP

#include <filesystem>
#include <memory>
#include <map>
#include <algorithm>
#include <locale>
#include <fstream>
#include <sstream>
#include <functional>
#include "archive.hpp"

namespace studio::fs
{
  using file_stream = std::pair<shared::archive::file_info, std::unique_ptr<std::basic_istream<std::byte>>>;

  struct null_buffer : public std::basic_streambuf<std::byte>
  {
    int overflow(int c) { return c; }
  };

  struct file_system_archive
  {
    const std::filesystem::path& search_path;

    std::locale default_locale;

    std::multimap<std::string, std::unique_ptr<shared::archive::file_archive>> archive_types;
    std::map<std::string, std::function<void(const shared::archive::file_info&)>> actions;

    mutable std::map<std::string, std::vector<shared::archive::file_info>> info_cache;

    explicit file_system_archive(const std::filesystem::path& search_path) : search_path(search_path) {}

    static std::filesystem::path get_archive_path(const std::filesystem::path& folder_path)
    {
      auto archive_path = folder_path;

      while (!std::filesystem::exists(archive_path) && !std::filesystem::is_directory(archive_path))
      {
        archive_path = archive_path.parent_path();
      }

      return archive_path;
    }

    void add_action(std::string name, std::function<void(const shared::archive::file_info&)> action)
    {
      actions.emplace(std::move(name), std::move(action));
    }

    void execute_action(const std::string& name, const shared::archive::file_info& info) const
    {
      auto result = actions.find(name);

      if (result != actions.end())
      {
        result->second(info);
      }
    }

    void add_archive_type(std::string extension, std::unique_ptr<shared::archive::file_archive> archive_type)
    {
      std::transform(extension.begin(), extension.end(), extension.begin(), [&](auto c) { return std::tolower(c, default_locale); });
      archive_types.insert(std::make_pair(std::move(extension), std::move(archive_type)));
    }

    std::vector<shared::archive::file_info> find_files(const std::filesystem::path& new_search_path, const std::vector<std::string_view>& extensions) const
    {
      std::stringstream key;
      key << new_search_path;
      std::for_each(extensions.begin(), extensions.end(), [&](auto& ext) { key << ext; });

      auto cache_result = info_cache.find(key.str());

      if (cache_result != info_cache.end())
      {
        return cache_result->second;
      }

      std::vector<shared::archive::file_info> results;

      auto files_folders = get_content_listing(new_search_path);

      std::function<void(decltype(files_folders)::const_reference)> get_files_folders = [&](const auto& file_folder) {
        std::visit([&](const auto& folder) {
          using T = std::decay_t<decltype(folder)>;

          if constexpr (std::is_same_v<T, shared::archive::folder_info>)
          {
            auto more_files = get_content_listing(folder.full_path);
            for (auto& item : more_files)
            {
              get_files_folders(item);
            }
          }

          if constexpr (std::is_same_v<T, shared::archive::file_info>)
          {
            if (extensions.size() == 1 && extensions.front() == "ALL")
            {
              results.emplace_back(folder);
            }
            else
            {
              for (auto& extension : extensions)
              {
                auto ext = folder.filename.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), [&](char c) { return std::tolower(c, default_locale); });
                if (ext == extension)
                {
                  results.emplace_back(folder);
                  break;
                }
              }
            }
          }
        },
          file_folder);
      };

      for (const auto& item : files_folders)
      {
        get_files_folders(item);
      }

      info_cache.emplace(key.str(), results);

      return results;
    }

    std::vector<shared::archive::file_info> find_files(const std::vector<std::string_view>& extensions) const
    {
      return find_files(search_path, extensions);
    }

    static void merge_results(std::vector<shared::archive::file_info>& group1,
      const std::vector<shared::archive::file_info>& group2)
    {
      group1.reserve(group1.capacity() + group2.size());

      for (auto& group2_item : group2)
      {
        auto result = std::find_if(group1.begin(), group1.end(), [&](const auto& group1_item) {
               return group1_item.folder_path == group2_item.folder_path;
        });

        if (result == group1.end())
        {
          group1.emplace_back(group2_item);
        }
      }
    }

    file_stream load_file(const std::filesystem::path& path)
    {
      shared::archive::file_info info{};

      info.folder_path = path.parent_path();
      info.filename = path.filename().string();

      return load_file(info);
    }

    file_stream load_file(const shared::archive::file_info& info) const
    {
      if (info.compression_type == shared::archive::compression_type::none)
      {
        if (std::filesystem::is_directory(info.folder_path))
        {
          return std::make_pair(info, std::make_unique<std::basic_ifstream<std::byte>>(info.folder_path / info.filename, std::ios::binary));
        }
        else
        {
          auto archive_path = get_archive_path(info.folder_path);
          auto file_stream = std::make_unique<std::basic_ifstream<std::byte>>(archive_path, std::ios::binary);

          auto archive = get_archive_type(archive_path);

          if (archive.has_value())
          {
            archive->get().set_stream_position(*file_stream, info);
          }

          return std::make_pair(info, std::move(file_stream));
        }
      }
      else
      {
        auto archive_path = get_archive_path(info.folder_path);

        auto file_stream = std::basic_ifstream<std::byte>(archive_path, std::ios::binary);
        auto archive = get_archive_type(archive_path);

        auto memory_stream = std::make_unique<std::basic_stringstream<std::byte>>();

        if (archive.has_value())
        {
          archive->get().extract_file_contents(file_stream, info, *memory_stream);
        }

        return std::make_pair(info, std::move(memory_stream));
      }
    }

    bool is_regular_file(const std::filesystem::path& folder_path) const
    {
      auto archive_path = get_archive_path(folder_path);

      if (archive_path == folder_path)
      {
        return !(std::filesystem::is_directory(folder_path) || get_archive_type(folder_path).has_value());
      }

      return folder_path.has_extension();
    }

    std::optional<std::reference_wrapper<shared::archive::file_archive>> get_archive_type(std::basic_istream<std::byte>& stream) const
    {
      for (auto& [key, value] : archive_types)
      {
        if (value->stream_is_supported(stream))
        {
          return std::ref(*value);
        }
      }

      return std::nullopt;
    }

    std::optional<std::reference_wrapper<shared::archive::file_archive>> get_archive_type(const std::filesystem::path& file_path) const
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

    std::vector<std::variant<shared::archive::folder_info, shared::archive::file_info>> get_content_listing(const std::filesystem::path& folder_path) const
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
