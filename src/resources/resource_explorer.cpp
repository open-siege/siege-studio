#include <sstream>
#include <iostream>
#include "resource_explorer.hpp"

namespace studio::resources
{
  std::filesystem::path resource_explorer::get_archive_path(const std::filesystem::path& folder_path)
  {
    auto archive_path = folder_path;

    while (!std::filesystem::exists(archive_path) && !std::filesystem::is_directory(archive_path))
    {
      archive_path = archive_path.parent_path();
    }

    return archive_path;
  }

  void resource_explorer::add_action(std::string name, std::function<void(const studio::resources::file_info&)> action)
  {
    actions.emplace(std::move(name), std::move(action));
  }

  void resource_explorer::execute_action(const std::string& name, const studio::resources::file_info& info) const
  {
    auto result = actions.find(name);

    if (result != actions.end())
    {
      result->second(info);
    }
  }

  std::filesystem::path resource_explorer::get_search_path() const
  {
    return search_path;
  }

  void resource_explorer::add_archive_type(std::string extension, std::unique_ptr<studio::resources::archive_plugin> archive_type)
  {
    std::transform(extension.begin(), extension.end(), extension.begin(), [&](auto c) { return std::tolower(c, default_locale); });
    archive_types.insert(std::make_pair(std::move(extension), std::move(archive_type)));
  }

  std::vector<studio::resources::file_info> resource_explorer::find_files(const std::filesystem::path& new_search_path, const std::vector<std::string_view>& extensions) const
  {
    std::stringstream key;
    key << new_search_path;
    std::for_each(extensions.begin(), extensions.end(), [&](auto& ext) { key << ext; });

    auto cache_result = info_cache.find(key.str());

    if (cache_result != info_cache.end())
    {
      return cache_result->second;
    }

    std::vector<studio::resources::file_info> results;

    auto files_folders = get_content_listing(new_search_path);

    std::function<void(decltype(files_folders)::const_reference)> get_files_folders = [&](const auto& file_folder) {
      std::visit([&](const auto& folder) {
        using T = std::decay_t<decltype(folder)>;

        if constexpr (std::is_same_v<T, studio::resources::folder_info>)
        {
          auto more_files = get_content_listing(folder.full_path);

          if (std::filesystem::exists(folder.full_path) && !std::filesystem::is_directory(folder.full_path))
          {
            for (auto& extension : extensions)
            {
              auto ext = folder.full_path.filename().extension().string();
              std::transform(ext.begin(), ext.end(), ext.begin(), [&](char c) { return std::tolower(c, default_locale); });
              if (ext == extension)
              {
                studio::resources::file_info info{};
                info.filename = folder.full_path.filename();
                info.folder_path = folder.full_path.parent_path();
                results.emplace_back(info);
                break;
              }
            }
          }

          for (auto& item : more_files)
          {
            get_files_folders(item);
          }
        }

        if constexpr (std::is_same_v<T, studio::resources::file_info>)
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

  std::vector<studio::resources::file_info> resource_explorer::find_files(const std::vector<std::string_view>& extensions) const
  {
    return find_files(search_path, extensions);
  }

  void resource_explorer::merge_results(std::vector<studio::resources::file_info>& group1,
    const std::vector<studio::resources::file_info>& group2)
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

  file_stream resource_explorer::load_file(const std::filesystem::path& path) const
  {
    studio::resources::file_info info{};

    info.folder_path = path.parent_path();
    info.filename = path.filename().string();

    return load_file(info);
  }

  file_stream resource_explorer::load_file(const studio::resources::file_info& info) const
  {
    if (info.compression_type == studio::resources::compression_type::none)
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

  bool resource_explorer::is_regular_file(const std::filesystem::path& folder_path) const
  {
    auto archive_path = get_archive_path(folder_path);

    if (archive_path == folder_path)
    {
      return !(std::filesystem::is_directory(folder_path) || get_archive_type(folder_path).has_value());
    }

    return folder_path.has_extension();
  }

  std::optional<std::reference_wrapper<studio::resources::archive_plugin>> resource_explorer::get_archive_type(const std::filesystem::path& file_path) const
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

  void resource_explorer::extract_file_contents(std::basic_istream<std::byte>& archive_file, std::filesystem::path destination, const studio::resources::file_info& info) const
  {
    auto archive_path = get_archive_path(info.folder_path);

    destination = destination / std::filesystem::relative(archive_path, search_path).parent_path() / archive_path.stem() / std::filesystem::relative(info.folder_path, archive_path).replace_extension("");

    if (archive_path.stem() == destination.stem())
    {
      destination = destination.parent_path();
    }

    std::filesystem::create_directories(destination);

    std::basic_ofstream<std::byte> new_file(destination / info.filename, std::ios::binary);

    auto type = get_archive_type(archive_path);

    if (type.has_value())
    {
      type->get().extract_file_contents(archive_file, info, new_file);
    }
  }

  std::vector<std::variant<studio::resources::folder_info, studio::resources::file_info>> resource_explorer::get_content_listing(const std::filesystem::path& folder_path) const
  {
    std::vector<std::variant<studio::resources::folder_info, studio::resources::file_info>> files;

    if (auto archive_type = get_archive_type(get_archive_path(folder_path)); archive_type.has_value())
    {
      auto file_stream = std::basic_ifstream<std::byte>{ get_archive_path(folder_path), std::ios::binary };

      return archive_type.value().get().get_content_listing(file_stream, folder_path);
    }

    for (auto& item : std::filesystem::directory_iterator(folder_path))
    {
      if (item.is_directory())
      {
        studio::resources::folder_info info{};
        info.name = item.path().filename().string();
        info.full_path = item.path();
        files.emplace_back(info);
      }
      else if (auto archive_type = get_archive_type(item.path()); archive_type.has_value())
      {
        studio::resources::folder_info info{};
        info.name = item.path().filename().string();
        info.full_path = item.path();
        files.emplace_back(info);
      }
      else
      {
        studio::resources::file_info info{};

        info.filename = item.path().filename().string();
        info.folder_path = item.path().parent_path();
        info.size = std::filesystem::file_size(item.path());
        files.emplace_back(info);
      }
    }

    return files;
  }
}// namespace studio::resource