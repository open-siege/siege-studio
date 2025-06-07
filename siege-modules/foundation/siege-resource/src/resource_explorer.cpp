#include <sstream>
#include <functional>
#include <filesystem>
#include <climits>
#include <siege/platform/shared.hpp>
#include <siege/resource/resource_explorer.hpp>

// Check to make sure our chars are 8 bits wide and additional sanity checks.
static_assert(CHAR_BIT == 8);
static_assert(sizeof(char) == sizeof(std::byte));

namespace siege::resource
{
  std::filesystem::path resource_explorer::get_search_path() const
  {
    return std::filesystem::current_path();
  }

  void resource_explorer::add_archive_type(std::string extension, std::unique_ptr<siege::platform::resource_reader> archive_type, std::optional<std::span<std::string_view>> explicit_extensions)
  {
    auto result = archive_types.insert(std::make_pair(platform::to_lower(extension), std::move(archive_type)));

    if (explicit_extensions.has_value())
    {
      archive_explicit_extensions.emplace(std::make_pair(result->first, explicit_extensions.value()));
    }
  }

  std::vector<std::string_view> resource_explorer::get_archive_extensions() const
  {
    std::vector<std::string_view> extensions;
    extensions.reserve(archive_types.size());

    for (auto &[key, value] : archive_types)
    {
      if (extensions.empty() || extensions.back() != key)
      {
        extensions.emplace_back(key);
      }
    }

    return extensions;
  }

  std::vector<siege::platform::file_info> resource_explorer::find_files(const std::filesystem::path& new_search_path, const std::vector<std::string_view>& extensions) const
  {
    std::stringstream key;
    key << new_search_path;
    std::for_each(extensions.begin(), extensions.end(), [&](auto& ext) { key << ext; });

    auto cache_result = info_cache.find(key.str());

    if (cache_result != info_cache.end())
    {
      return cache_result->second;
    }

    std::vector<siege::platform::file_info> results;

    auto files_folders = get_content_listing(new_search_path);

    std::function<void(decltype(files_folders)::const_reference)> get_files_folders = [&](const auto& file_folder) {
      std::visit([&](const auto& folder) {
        using T = std::decay_t<decltype(folder)>;

        if constexpr (std::is_same_v<T, siege::platform::folder_info>)
        {
          const auto& real_folder = static_cast<const siege::platform::folder_info&>(folder);

          const auto ext = platform::to_lower(real_folder.full_path.extension().string());

          // There are specific archives that must not be queried, unless
          // they or their supported formats are explicitly queried.
          if (auto must_be_explicit = archive_explicit_extensions.find(ext);
              std::filesystem::exists(folder.full_path) &&
              !std::filesystem::is_directory(real_folder.full_path) && must_be_explicit != archive_explicit_extensions.end())
          {
            auto count = std::count(extensions.begin(), extensions.end(), "ALL");

            if (count == 0)
            {
              count = std::count(extensions.begin(), extensions.end(), ext);
            }

            if (count == 0)
            {
              for (auto value : must_be_explicit->second)
              {
                count += std::count(extensions.begin(), extensions.end(), value);
              }

              if (count == 0)
              {
                return;
              }
            }
          }

          auto more_files = get_content_listing(folder.full_path);

          if (std::filesystem::exists(folder.full_path) && !std::filesystem::is_directory(folder.full_path))
          {
            for (auto& extension : extensions)
            {
              if (platform::to_lower(folder.full_path.filename().extension().string()) == extension)
              {
                siege::platform::file_info info{};
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

        if constexpr (std::is_same_v<T, siege::platform::file_info>)
        {
          if (extensions.size() == 1 && extensions.front() == "ALL")
          {
            results.emplace_back(folder);
          }
          else
          {
            for (auto& extension : extensions)
            {
              auto ext = platform::to_lower(folder.filename.extension().string());
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

  std::vector<siege::platform::file_info> resource_explorer::find_files(const std::vector<std::string_view>& extensions) const
  {
    return find_files(get_search_path(), extensions);
  }

  file_stream resource_explorer::load_file(const std::filesystem::path& path) const
  {
    siege::platform::file_info info{};

    info.folder_path = path.parent_path();
    info.filename = path.filename().string();

    return load_file(info);
  }

  file_stream resource_explorer::load_file(const siege::platform::file_info& info) const
  {
    std::any cache;
    if (info.compression_type == siege::platform::compression_type::none)
    {
      if (std::filesystem::is_directory(info.folder_path))
      {
        return std::make_pair(info, std::make_unique<std::ifstream>(info.folder_path / info.filename, std::ios::binary));
      }
      else
      {
        auto archive_path = get_archive_path(info.folder_path);
        auto file_stream = std::make_unique<std::ifstream>(archive_path, std::ios::binary);

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

      auto file_stream = std::ifstream(archive_path, std::ios::binary);
      auto archive = get_archive_type(archive_path);

      auto memory_stream = std::make_unique<std::stringstream>(std::ios::binary | std::ios::in | std::ios::out);

      if (archive.has_value())
      {
        archive->get().extract_file_contents(cache, file_stream, info, *memory_stream);
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

  std::filesystem::path resource_explorer::get_archive_path(const std::filesystem::path& folder_path) const
  {
    auto root = folder_path.root_path();
    auto temp = folder_path;

    // first pass, based on supported extension
    while (temp != root)
    {
      auto ext = platform::to_lower(temp.filename().extension().string());
      auto archive_type = archive_types.equal_range(ext);

      if (archive_type.first != archive_type.second)
      {
        return temp;
      }

      temp = temp.parent_path();
    }

    // second pass, looking for a real file and not a folder
    auto archive_path = folder_path;

    constexpr static auto max_depth = 16;
    auto depth = 0;

    while (!std::filesystem::exists(archive_path) && !std::filesystem::is_directory(archive_path))
    try
    {
      archive_path = archive_path.parent_path();
      depth++;

      if (depth >= max_depth)
      {
        break;
      }
    }
    catch(...)
    {
      break;
    }

    return archive_path;
  }

  std::optional<std::reference_wrapper<siege::platform::resource_reader>> resource_explorer::get_archive_type(const std::filesystem::path& file_path) const
  {
    auto ext = platform::to_lower(file_path.filename().extension().string());
    auto archive_type = archive_types.equal_range(ext);

    for (auto it = archive_type.first; it != archive_type.second; ++it)
    {
      auto file_stream = std::ifstream{ file_path, std::ios::binary };

      if (it->second->stream_is_supported(file_stream))
      {
        return std::ref(*it->second);
      }
    }

    return std::nullopt;
  }

  void resource_explorer::extract_file_contents(std::istream& archive_file,
    std::filesystem::path destination,
    const siege::platform::file_info& info,
    std::optional<std::reference_wrapper<platform::batch_storage>> storage) const
  {
    std::any cache;
    auto archive_path = get_archive_path(info.folder_path);

    if (destination.filename() != info.filename)
    {
      destination = destination / std::filesystem::relative(archive_path, get_search_path()).parent_path() / archive_path.stem() / std::filesystem::relative(info.folder_path, archive_path).replace_extension("");

      if (archive_path.stem() == destination.stem())
      {
        destination = destination.parent_path();
      }
      std::filesystem::create_directories(destination);
      destination = destination / info.filename;
    }

    std::ofstream new_file(destination, std::ios::binary);

    auto type = get_archive_type(archive_path);

    if (type.has_value())
    {
      type->get().extract_file_contents(cache, archive_file, info, new_file);
    }
  }

  std::vector<std::variant<siege::platform::folder_info, siege::platform::file_info>> resource_explorer::get_content_listing(const std::filesystem::path& folder_path) const
  {
    std::any cache;
    std::vector<std::variant<siege::platform::folder_info, siege::platform::file_info>> files;

    if (auto archive_type = get_archive_type(get_archive_path(folder_path)); archive_type.has_value())
    {
      auto file_stream = std::ifstream{ get_archive_path(folder_path), std::ios::binary };

      return archive_type.value().get().get_content_listing(cache, file_stream, { get_archive_path(folder_path), folder_path });
    }

    if (!std::filesystem::is_directory(folder_path))
    {
      return files;
    }

    for (auto& item : std::filesystem::directory_iterator(folder_path))
    try
    {
      if (item.is_directory())
      {
        siege::platform::folder_info info{};
        info.name = item.path().filename().string();
        info.full_path = item.path();
        files.emplace_back(info);
      }
      else if (auto archive_type = get_archive_type(item.path()); archive_type.has_value())
      {
        siege::platform::folder_info info{};
        info.name = item.path().filename().string();
        info.full_path = item.path();
        files.emplace_back(info);
      }
      else
      {
        siege::platform::file_info info{};

        info.filename = item.path().filename().string();
        info.folder_path = item.path().parent_path();
        info.size = std::size_t(std::filesystem::file_size(item.path()));
        files.emplace_back(info);
      }
    }
    catch(...)
    {
      continue;
    }

    return files;
  }
}// namespace siege::resource
