#include <array>
#include <string>
#include <string_view>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <iostream>
#include <functional>
#include <variant>
#include <list>
#include <map>
#include <unordered_set>
#include <siege/resource/external_utils.hpp>

#ifdef WIN32
#include <siege/platform/win/module.hpp>
#endif

namespace fs = std::filesystem;
namespace platform = siege::platform;

namespace siege::resource
{
  [[nodiscard]] std::string find_cab_executable(std::string_view app_name, std::array<std::string_view, 2> search_names)
  {
#ifdef WIN32
    try
    {

      for (auto& search_name : search_names)
      {
        auto path = win32::find_binary_module({
          .module_name = search_name,
        });

        if (path)
        {
          auto result = path->string();

          if (result.contains(" "))
          {
            result.insert(0, 1, '\"');
            result.push_back('\"');
          }
          return result;
        }
      }
    }
    catch (...)
    {
    }
#endif

    return std::string(app_name);
  }

  [[nodiscard]] std::string cab6_executable()
  {
    std::array<std::string_view, 2> names = { { "i6comp.exe",
      "i6cmp13b\\i6comp.exe" } };

    return find_cab_executable("i6comp", names);
  }

  [[nodiscard]] std::string cab5_executable()
  {
    std::array<std::string_view, 2> names = { { "I5comp.exe",
      "i5comp21\\I5comp.exe" } };

    return find_cab_executable("I5comp", names);
  }

  [[nodiscard]] std::string cab2_executable()
  {
    std::array<std::string_view, 2> names = { { "ICOMP.EXE",
      "icomp95\\ICOMP.EXE" } };

    return find_cab_executable("ICOMP", names);
  }

  [[nodiscard]] std::string seven_zip_executable()
  {
#ifdef WIN32
    for (auto name : { L"7zr", L"7z" })
    {
      auto path = win32::find_binary_module({ .module_name = name,
        .search_env_paths = true });

      if (path)
      {
        return path->string();
      }
    }
#endif
    std::error_code last_error;
    for (auto var : { "PROGRAMFILES", "PROGRAMFILES(x86)" })
    {
      auto env = ::getenv(var);

      if (!env)
      {
        continue;
      }

      auto path = fs::path(env) / "7-Zip" / "7z.exe";

      if (fs::exists(path, last_error))
      {
        return path.string();
      }
    }

    return "7z";
  }

  [[nodiscard]] std::string wincdemu_executable()
  {
    std::error_code last_error;
    for (auto var : { "PROGRAMFILES", "PROGRAMFILES(x86)" })
    {
      auto env = ::getenv(var);

      if (!env)
      {
        continue;
      }

      auto path = fs::path(env) / "WinCDEmu" / "batchmnt.exe";

      if (fs::exists(path, last_error))
      {
        return path.string();
      }
    }

    return "batchmnt";
  }

  [[nodiscard]] std::string power_iso_executable()
  {
    std::error_code last_error;

    for (auto var : { "PROGRAMFILES", "PROGRAMFILES(x86)" })
    {
      auto env = ::getenv(var);

      if (!env)
      {
        continue;
      }

      auto path = fs::path(env) / "PowerISO" / "piso.exe";

      if (fs::exists(path, last_error))
      {
        return path.string();
      }
    }

    return "piso";
  }

  using content_map = std::unordered_map<std::string, std::vector<content_info>>;

  [[nodiscard]] std::vector<content_info> filter_results_for_query(const platform::listing_query& query, const content_map::const_iterator& cache_entry)
  {
    std::vector<content_info> results;

    auto is_valid = platform::overloaded{
      [&](const platform::file_info& item) {
        return item.folder_path == query.folder_path;
      },
      [&](const platform::folder_info& item) {
        return item.full_path.parent_path() == query.folder_path;
      }
    };

    results.reserve(std::count_if(cache_entry->second.begin(), cache_entry->second.end(), [&](const auto& info) {
      return std::visit(is_valid, info);
    }));


    for (const auto& item : cache_entry->second)
    {
      if (std::visit(is_valid, item))
      {
        results.emplace_back(item);
      }
    }

    return results;
  }

  [[nodiscard]] std::vector<std::string> read_lines(const fs::path& listing_filename)
  {
    std::ifstream raw_listing(listing_filename, std::ios::binary);
    return read_lines(raw_listing);
  }

  [[nodiscard]] std::vector<std::string> execute_command(const fs::path& listing_filename, const std::function<void(std::stringstream&)>& generate_command)
  {
    std::stringstream command;
    generate_command(command);
    std::cout << command.str() << '\n';
    std::system(command.str().c_str());

    return read_lines(listing_filename);
  }

  void execute_command(const std::function<void(std::stringstream&)>& generate_command)
  {
    std::stringstream command;
    generate_command(command);
    std::cout << command.str() << '\n';
    std::system(command.str().c_str());
  }

  struct content_cache
  {
    std::unordered_map<std::string, std::vector<content_info>> stat_cache;
    std::map<std::string, std::shared_ptr<fs::path>> auto_delete_files;
    std::unordered_set<std::string> already_ran_commands;
    std::map<std::string_view, std::set<fs::path>> additional_flags;
  };

  content_cache& cache_as_content_cache(std::any& cache)
  {
    if (cache.has_value() && cache.type() == typeid(content_cache))
    {
      return std::any_cast<content_cache&>(cache);
    }
    else
    {
      content_cache temp{};
      cache = temp;
      return std::any_cast<content_cache&>(cache);
    }
  }

  std::vector<content_info> cached_get_content_listing(std::any& cache, const platform::listing_query& query, const std::function<std::vector<content_info>(const fs::path& listing_filename)>& get_listing)
  {
    content_cache& temp = cache_as_content_cache(cache);

    auto cache_key = fs::exists(query.archive_path) ? query.archive_path.string() + std::to_string(fs::file_size(query.archive_path)) : query.archive_path.string();

    auto cache_entry = temp.stat_cache.find(cache_key);

    if (cache_entry == temp.stat_cache.end())
    {
      auto listing_filename = platform::make_auto_remove_path(fs::temp_directory_path() / (query.archive_path.stem().string() + "-listing.txt"));

      auto results = get_listing(*listing_filename);

      if (results.empty())
      {
        return {};
      }

      cache_entry = temp.stat_cache.emplace(cache_key, std::move(results)).first;
    }

    return filter_results_for_query(query, cache_entry);
  }

  std::unordered_map<std::string, std::string>& get_cached_drive_paths()
  {
    static std::unordered_map<std::string, std::string> drive_paths;
    return drive_paths;
  }

  void wincdemu_unmount_iso(const fs::path& archive_path)
  {
    auto& drive_paths = get_cached_drive_paths();

    if (drive_paths.find(archive_path.string()) == drive_paths.end())
    {
      return;
    }

    execute_command([&](auto& command) {
      command << '\"' << wincdemu_executable() << " /unmount " << archive_path << '\"';
      drive_paths.erase(archive_path.string());
    });
  }

  std::optional<std::string> wincdemu_mount_iso(const fs::path& archive_path, const fs::path& listing_filename)
  {
    auto& drive_paths = get_cached_drive_paths();
    auto existing_path = drive_paths.find(archive_path.string());

    if (existing_path != drive_paths.end())
    {
      return existing_path->second;
    }

    auto lines = execute_command(listing_filename, [&](auto& command) {
      command << '\"' << wincdemu_executable() << ' ' << archive_path << " > " << listing_filename << '\"';
    });

    if (lines.empty())
    {
      return std::nullopt;
    }

    bool already_mounted = lines[0].find("already mounted") != std::string::npos;

    if (lines[0].find("successfully") == std::string::npos && lines[0].find("already mounted") == std::string::npos)
    {
      return std::nullopt;
    }

    lines = execute_command(listing_filename, [&](auto& command) {
      command << '\"' << wincdemu_executable() << " /list > " << listing_filename << '\"';
    });

    auto drive_letter_iter = std::find_if(lines.begin(), lines.end(), [&](auto& line) {
      return line.find(archive_path.string()) != std::string::npos;
    });

    if (drive_letter_iter == lines.end())
    {
      return std::nullopt;
    }

    auto drive_letter = drive_letter_iter->substr(0, drive_letter_iter->find(archive_path.string()));

    if (!fs::exists(drive_letter))
    {
      return std::nullopt;
    }

    // wincdemu only maps one image to one drive,
    // so if it is already mounted externally,
    // we don't want to unmount it later (because it was most likely mounted by the user already).
    if (!already_mounted)
    {
      drive_paths.emplace(archive_path.string(), drive_letter);
    }

    return drive_letter;
  }

  std::vector<content_info> wincdemu_get_content_listing(std::any& cache, const platform::listing_query& query)
  {
    return cached_get_content_listing(cache, query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto drive_letter = wincdemu_mount_iso(query.archive_path, listing_filename);

      if (!drive_letter.has_value())
      {
        return {};
      }

      std::vector<content_info> results;
      results.reserve(128);

      for (const fs::directory_entry& dir_entry :
        fs::recursive_directory_iterator(drive_letter.value()))
      {
        if (dir_entry.is_directory())
        {
          platform::folder_info folder{};
          folder.full_path = dir_entry.path().relative_path();
          folder.archive_path = query.archive_path;
          folder.name = folder.full_path.filename().string();
          results.emplace_back(folder);
        }
        else
        {
          platform::file_info file{};
          file.archive_path = query.archive_path;
          file.compression_type = platform::compression_type::none;
          file.size = dir_entry.file_size();
          file.folder_path = dir_entry.path().parent_path().relative_path();
          file.filename = dir_entry.path().filename();
          results.emplace_back(file);
        }
      }

      wincdemu_unmount_iso(query.archive_path);

      return results;
    });
  }

  std::optional<std::string> winiso_mount_iso(const fs::path& archive_path, const fs::path& listing_filename)
  {
    auto lines = execute_command(listing_filename, [&](auto& command) {
      command << '\"' << "powershell -c \"(Mount-DiskImage -ImagePath " << archive_path << " | Get-Volume).DriveLetter + ':'\" > " << listing_filename << '\"';
    });

    if (lines.empty())
    {
      return std::nullopt;
    }

    if (lines[0].find(":") == std::string::npos)
    {
      return std::nullopt;
    }

    if (!fs::exists(lines[0]))
    {
      return std::nullopt;
    }

    return lines[0];
  }

  void winiso_unmount_iso(const fs::path& archive_path)
  {
    auto& drive_paths = get_cached_drive_paths();

    if (drive_paths.find(archive_path.string()) == drive_paths.end())
    {
      return;
    }

    execute_command([&](auto& command) {
      command << '\"' << "powershell -c \"Dismount-DiskImage -ImagePath " << archive_path << '\"' << '\"';
      drive_paths.erase(archive_path.string());
    });
  }

  std::vector<content_info> winiso_get_content_listing(std::any& cache, const platform::listing_query& query)
  {
    return cached_get_content_listing(cache, query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto drive_letter = winiso_mount_iso(query.archive_path, listing_filename);
      if (!drive_letter.has_value())
      {
        return {};
      }

      std::vector<content_info> results;
      results.reserve(128);

      for (const fs::directory_entry& dir_entry :
        fs::recursive_directory_iterator(drive_letter.value()))
      {
        if (dir_entry.is_directory())
        {
          platform::folder_info folder{};
          folder.full_path = dir_entry.path().relative_path();
          folder.archive_path = query.archive_path;
          folder.name = folder.full_path.filename().string();
          results.emplace_back(folder);
        }
        else
        {
          platform::file_info file{};
          file.archive_path = query.archive_path;
          file.compression_type = platform::compression_type::none;
          file.size = dir_entry.file_size();
          file.folder_path = dir_entry.path().parent_path().relative_path();
          file.filename = dir_entry.path().filename();
          results.emplace_back(file);
        }
      }

      winiso_unmount_iso(query.archive_path);

      return results;
    });
  }

  std::vector<content_info> zip_get_content_listing(std::any& cache, const platform::listing_query& query)
  {
    return cached_get_content_listing(cache, query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto raw_contents = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << seven_zip_executable() << " l " << query.archive_path << " > " << listing_filename << '\"';
      });

      auto header_iter = std::find_if(raw_contents.begin(), raw_contents.end(), [](auto& row) {
        return row.find("Date") != std::string::npos && row.find("Time") != std::string::npos && row.find("Attr") != std::string::npos && row.find("Size") != std::string::npos && row.find("Compressed") != std::string::npos && row.find("Name") != std::string::npos;
      });

      if (header_iter == raw_contents.end())
      {
        return {};
      }

      auto line_iter = header_iter;
      std::advance(line_iter, 1);

      if (line_iter == raw_contents.end())
      {
        return {};
      }

      auto date_time_indices = std::make_pair(line_iter->find('-'), line_iter->find(' '));
      auto attr_indices = std::make_pair(line_iter->find('-', date_time_indices.second), line_iter->find(' ', date_time_indices.second + 1));
      auto size_indices = std::make_pair(line_iter->find('-', attr_indices.second), line_iter->find(' ', attr_indices.second + 1));
      auto compressed_indices = std::make_pair(line_iter->find('-', size_indices.second), line_iter->find(' ', size_indices.second + 1));
      auto name_indices = std::make_pair(line_iter->find('-', compressed_indices.second), line_iter->rfind('-'));

      auto first = line_iter;
      std::advance(first, 1);

      if (first == raw_contents.end())
      {
        return {};
      }

      std::vector<content_info> results;
      results.reserve(std::distance(first, raw_contents.end()));

      for (auto current = first; current != raw_contents.end(); ++current)
      {
        auto attr = current->substr(attr_indices.first, attr_indices.second - attr_indices.first);

        if (attr == "-----")
        {
          break;
        }

        if (attr.find('D') != std::string::npos)
        {
          platform::folder_info folder{};
          folder.archive_path = query.archive_path;
          folder.full_path = (query.archive_path / current->substr(name_indices.first)).make_preferred();
          folder.name = folder.full_path.filename().string();
          results.emplace_back(folder);
        }
        else
        {
          platform::file_info file{};

          auto name = fs::path(current->substr(name_indices.first));

          file.filename = name.filename();
          file.archive_path = query.archive_path;
          file.folder_path = (query.archive_path / name.make_preferred()).parent_path();
          file.compression_type = platform::compression_type::lz77_huffman;

          try
          {
            auto size = current->substr(size_indices.first, size_indices.second - size_indices.first);

            if (auto iter = std::find_if(size.begin(), size.end(), [](auto val) { return std::isdigit(val); });
              iter != size.end())
            {
              file.size = std::stoi(size);
            }
          }
          catch (...)
          {
          }

          results.emplace_back(file);
        }
      }

      return results;
    });
  }

  void add_folders(const platform::file_info& file,
    const fs::path& archive_path,
    std::unordered_set<std::string>& folders,
    const std::function<void(platform::folder_info&&)>& move_result)
  {
    auto [folder_iter, folder_added] = folders.emplace(file.folder_path.string());

    if (folder_added && file.folder_path != archive_path)
    {
      auto relative_path = fs::relative(file.folder_path, archive_path);

      auto new_path = archive_path;

      for (auto& segment : relative_path)
      {
        platform::folder_info folder{};
        new_path = new_path / segment;

        if (new_path != file.folder_path && folders.contains(new_path.string()))
        {
          continue;
        }

        folders.emplace(new_path.string());
        folder.full_path = new_path;
        folder.archive_path = archive_path;
        folder.name = folder.full_path.filename().string();
        move_result(std::move(folder));
      }
    }
  }

  std::vector<content_info> cab6_get_content_listing(std::any& cache, const std::string_view exe_path, const platform::listing_query& query)
  {
    return cached_get_content_listing(cache, query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto raw_contents = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << exe_path << " l -o " << query.archive_path << " > " << listing_filename << '\"';
      });
      std::vector<content_info> results;
      results.reserve(raw_contents.size());

      std::unordered_set<std::string> folders;

      constexpr static auto size_index = 17;
      constexpr static auto size_count = 9;
      constexpr static auto name_index = 50;

      for (auto& current : raw_contents)
      {
        if (std::any_of(current.begin(), current.end(), [](char item) { return (int)item < 0 || (int)item > 127; }))
        {
          continue;
        }

        if (std::all_of(current.begin(), current.end(), [](char item) { return item == '=' || item == ' '; }))
        {
          continue;
        }

        if (current.empty())
        {
          continue;
        }

        platform::file_info file{};

        auto name = fs::path(current.substr(name_index));

        file.filename = name.filename();
        file.archive_path = query.archive_path;
        file.folder_path = (query.archive_path / name.make_preferred()).parent_path();

        add_folders(file, query.archive_path, folders, [&](platform::folder_info&& folder) {
          results.emplace_back(folder);
        });
        file.compression_type = platform::compression_type::lz77_huffman;

        try
        {
          auto size = current.substr(size_index, size_count);

          if (auto iter = std::find_if(size.begin(), size.end(), [](auto val) { return std::isdigit(val); });
            iter != size.end())
          {
            file.size = std::stoi(size);
          }
        }
        catch (...)
        {
        }

        results.emplace_back(std::move(file));
      }

      return results;
    });
  }


  std::vector<content_info> cab5_get_content_listing(std::any& cache, const std::string_view exe_path, const platform::listing_query& query)
  {
    return cached_get_content_listing(cache, query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto raw_contents = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << exe_path << " l -o " << query.archive_path << " > " << listing_filename << '\"';
      });
      std::vector<content_info> results;
      results.reserve(raw_contents.size());

      std::unordered_set<std::string> folders;

      constexpr static auto size_index = 17;
      constexpr static auto size_count = 9;
      constexpr static auto name_index = 47;

      for (auto& current : raw_contents)
      {
        if (std::any_of(current.begin(), current.end(), [](char item) { return (int)item < 0 || (int)item > 127; }))
        {
          continue;
        }

        if (std::all_of(current.begin(), current.end(), [](char item) { return item == '=' || item == ' '; }))
        {
          continue;
        }

        if (current.empty())
        {
          continue;
        }

        platform::file_info file{};

        auto name = fs::path(current.substr(name_index));

        file.filename = name.filename();
        file.archive_path = query.archive_path;
        file.folder_path = (query.archive_path / name.make_preferred()).parent_path();

        add_folders(file, query.archive_path, folders, [&](platform::folder_info&& folder) {
          results.emplace_back(folder);
        });
        file.compression_type = platform::compression_type::lz77_huffman;

        try
        {
          auto size = current.substr(size_index, size_count);

          if (auto iter = std::find_if(size.begin(), size.end(), [](auto val) { return std::isdigit(val); });
            iter != size.end())
          {
            file.size = std::stoi(size);
          }
        }
        catch (...)
        {
        }

        results.emplace_back(std::move(file));
      }

      return results;
    });
  }

  std::vector<content_info> cab2_get_content_listing(std::any& cache, const platform::listing_query& query)
  {
    return cached_get_content_listing(cache, query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto raw_contents = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << cab2_executable() << " l " << query.archive_path << " > " << listing_filename << '\"';
      });

      auto header_iter = std::find_if(raw_contents.begin(), raw_contents.end(), [](auto& row) {
        return row.find("Date") != std::string::npos && row.find("Time") != std::string::npos && row.find("OrigSize") != std::string::npos && row.find("Attr") != std::string::npos && row.find("CompSize") != std::string::npos && row.find("Name") != std::string::npos;
      });

      if (header_iter == raw_contents.end())
      {
        return {};
      }

      auto line_iter = header_iter;
      std::advance(line_iter, 1);

      if (line_iter == raw_contents.end())
      {
        return {};
      }

      auto date_time_indices = std::make_pair(line_iter->find('='), line_iter->find(' '));
      auto attr_indices = std::make_pair(line_iter->find('=', date_time_indices.second), line_iter->find(' ', date_time_indices.second + 1));
      auto size_indices = std::make_pair(line_iter->find('=', attr_indices.second), line_iter->find(' ', attr_indices.second + 1));
      auto compressed_indices = std::make_pair(line_iter->find('=', size_indices.second), line_iter->find(' ', size_indices.second + 1));
      auto name_indices = std::make_pair(line_iter->find('=', compressed_indices.second), line_iter->rfind('='));

      auto first = line_iter;
      std::advance(first, 1);

      if (first == raw_contents.end())
      {
        return {};
      }

      std::vector<content_info> results;
      results.reserve(std::distance(first, raw_contents.end()));

      std::unordered_set<std::string> folders;

      for (auto current = first; current != raw_contents.end(); ++current)
      {
        auto attr = current->substr(attr_indices.first, attr_indices.second - attr_indices.first);

        if (attr == "====")
        {
          break;
        }

        if (attr.find('A') == std::string::npos)
        {
          platform::folder_info folder{};
          folder.archive_path = query.archive_path;
          folder.full_path = (query.archive_path / current->substr(name_indices.first)).make_preferred();
          folder.name = folder.full_path.filename().string();
          results.emplace_back(folder);
        }
        else
        {
          platform::file_info file{};

          auto name = fs::path(current->substr(name_indices.first));

          file.filename = name.filename();
          file.archive_path = query.archive_path;
          file.folder_path = (query.archive_path / name.make_preferred()).parent_path();
          file.compression_type = platform::compression_type::lz77_huffman;

          add_folders(file, query.archive_path, folders, [&](platform::folder_info&& folder) {
            results.emplace_back(folder);
          });

          try
          {
            auto size = current->substr(size_indices.first, size_indices.second - size_indices.first);

            if (auto iter = std::find_if(size.begin(), size.end(), [](auto val) { return std::isdigit(val); });
              iter != size.end())
            {
              file.size = std::stoi(size);
            }
          }
          catch (...)
          {
          }

          results.emplace_back(file);
        }
      }

      return results;
    });
  }


  std::vector<content_info> cab_get_content_listing(std::any& cache, const platform::listing_query& query)
  {
    content_cache& temp = cache_as_content_cache(cache);

    bool is_cab2 = temp.additional_flags["cab2"].contains(query.archive_path);
    bool is_cab5 = temp.additional_flags["cab5"].contains(query.archive_path);
    bool is_cab6 = temp.additional_flags["cab6"].contains(query.archive_path);

    auto cab2_listing = cab2_get_content_listing(cache, query);

    if (!cab2_listing.empty())
    {
      if (!is_cab5 && !is_cab6)
      {
        temp.additional_flags["cab2"].emplace(query.archive_path);
      }
      return cab2_listing;
    }

    auto cab5_listing = cab5_get_content_listing(cache, cab5_executable(), query);

    if (!cab5_listing.empty())
    {
      if (!is_cab2 && !is_cab6)
      {
        temp.additional_flags["cab5"].emplace(query.archive_path);
      }
      return cab5_listing;
    }

    auto cab6_listing = cab6_get_content_listing(cache, cab6_executable(), query);

    if (!cab6_listing.empty())
    {
      if (!is_cab2 && !is_cab5)
      {
        temp.additional_flags["cab6"].emplace(query.archive_path);
      }

      return cab6_listing;
    }

    return zip_get_content_listing(cache, query);
  }

  std::vector<content_info> power_iso_get_content_listing(std::any& cache, const platform::listing_query& query)
  {
    return cached_get_content_listing(cache, query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto raw_contents = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << power_iso_executable() << " list " << query.archive_path << " / -r"
                << " > " << listing_filename << '\"';
      });
      std::vector<content_info> results;

      auto first_folder = std::find_if(raw_contents.begin(), raw_contents.end(), [](auto& raw) {
        return raw.find("Objects in ") != std::string::npos;
      });

      auto current_path = query.archive_path;

      for (auto row = first_folder; row != raw_contents.end(); ++row)
      {
        if (row->empty())
        {
          continue;
        }

        if (row->find("Files, ") != std::string::npos && row->find("Folders") != std::string::npos && row->find("bytes") != std::string::npos)
        {
          continue;
        }

        if (row->find("Total:") == 0)
        {
          continue;
        }

        if (row->find("Objects in ") != std::string::npos)
        {
          auto start = row->find("\\") + 1;
          auto count = row->rfind(":") - start;
          auto path = row->substr(start, count);

          if (path.empty())
          {
            current_path = query.archive_path;
          }
          else
          {
            current_path = query.archive_path / fs::path(path).make_preferred();
          }

          continue;
        }

        if (row->find("<DIR>") != std::string::npos)
        {
          if (row->back() == '.')
          {
            continue;
          }

          auto name = row->substr(row->rfind("  ") + 2);

          if (name.empty())
          {
            continue;
          }

          platform::folder_info folder{};
          folder.archive_path = query.archive_path;
          folder.name = name;
          folder.full_path = current_path / name;

          results.emplace_back(folder);
        }
        else
        {
          platform::file_info file{};

          auto name = row->substr(row->rfind("  ") + 2);

          if (name.empty())
          {
            continue;
          }

          file.filename = name;
          file.archive_path = query.archive_path;
          file.folder_path = current_path;
          file.compression_type = platform::compression_type::cdxa;

          results.emplace_back(file);
        }
      }

      return results;
    });
  }

  std::vector<content_info> iso_get_content_listing(std::any& cache, const platform::listing_query& query)
  {
    auto listing = power_iso_get_content_listing(cache, query);

    if (!listing.empty())
    {
      return listing;
    }

    listing = zip_get_content_listing(cache, query);

    if (!listing.empty())
    {
      return listing;
    }

    listing = wincdemu_get_content_listing(cache, query);

    if (!listing.empty())
    {
      return listing;
    }

    return winiso_get_content_listing(cache, query);
  }

  [[maybe_unused]] bool extract_file_contents_using_external_app(std::any& cache, const siege::platform::file_info& info, std::ostream& output, std::string (*extract_one_command)(const siege::platform::file_info&, const fs::path&, const fs::path&), std::string (*extract_all_command)(const siege::platform::file_info&, const fs::path&, const fs::path&))
  {
    content_cache& temp_cache = cache_as_content_cache(cache);

    auto delete_path = platform::make_auto_remove_path();

    auto temp_path = fs::temp_directory_path() / (info.archive_path.stem().string() + "temp");
    auto internal_file_path = info.folder_path == info.archive_path ? info.filename : fs::relative(info.folder_path, info.archive_path) / info.filename;

    auto current_working_path = fs::current_path();
    fs::create_directories(temp_path);

    if (temp_cache.stat_cache.empty())
    {
      delete_path.reset(new fs::path(temp_path / internal_file_path));

      std::cout << extract_one_command(info, temp_path, internal_file_path) << '\n';
      std::cout.flush();
      std::system(extract_one_command(info, temp_path, internal_file_path).c_str());
    }
    else if (!temp_cache.already_ran_commands.contains(extract_all_command(info, temp_path, info.archive_path)))
    {
      delete_path.reset(new fs::path(temp_path));

      std::cout << extract_all_command(info, temp_path, info.archive_path) << '\n';
      std::cout.flush();
      std::system(extract_all_command(info, temp_path, info.archive_path).c_str());
      auto [command_iter, added] = temp_cache.already_ran_commands.emplace(extract_all_command(info, temp_path, info.archive_path));

      temp_cache.auto_delete_files.emplace(*command_iter, std::move(delete_path));
    }

    fs::current_path(current_working_path);

    if (!fs::exists(temp_path / internal_file_path))
    {
      return false;
    }

    std::ifstream temp(temp_path / internal_file_path, std::ios::binary);

    std::copy_n(std::istreambuf_iterator(temp),
      fs::file_size(temp_path / internal_file_path),
      std::ostreambuf_iterator(output));

    return true;
  }

  bool seven_extract_file_contents(std::any& storage, const siege::platform::file_info& info, std::ostream& output)
  {
    return extract_file_contents_using_external_app(
      storage,
      info,
      output,
      [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
        std::stringstream command;
        command << '\"' << seven_zip_executable() << " x -y -o" << temp_path
                << ' ' <<  info.archive_path << " \"" << internal_file_path.string() << "\""
                << '\"';
        return command.str(); },
      [](const auto& info, const auto& temp_path, const auto&) {
        std::stringstream command;
        command << '\"' << seven_zip_executable() << " x -y -o" << temp_path << ' ' <<  info.archive_path << '\"';
        return command.str(); });
  }

  void iso_extract_file_contents(std::any& storage, const siege::platform::file_info& info, std::ostream& output)
  {
    auto extracted = extract_file_contents_using_external_app(
      storage, info, output, [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
        std::stringstream command;
        command << '\"' << power_iso_executable() << " extract " << info.archive_path << " / -y -od " << temp_path << '\"';
        return command.str(); }, [](const auto& info, const auto& temp_path, const auto&) {
        std::stringstream command;
        command << '\"' << power_iso_executable() << " extract " << info.archive_path << " / -y -od " << temp_path << '\"';
        return command.str(); });

    if (extracted)
    {
      return;
    }

    extracted = seven_extract_file_contents(storage, info, output);

    if (extracted)
    {
      return;
    }

    const auto temp_file = fs::temp_directory_path() / "temp-output.txt";
    auto drive_letter = wincdemu_mount_iso(info.archive_path, temp_file);

    if (!drive_letter.has_value())
    {
      drive_letter = winiso_mount_iso(info.archive_path, temp_file);
    }

    if (!drive_letter.has_value())
    {
      return;
    }

    std::ifstream temp(drive_letter.value() / info.folder_path / info.filename, std::ios::binary);

    std::copy_n(std::istreambuf_iterator(temp),
      info.size,
      std::ostreambuf_iterator(output));

    if (!storage.has_value())
    {
      std::shared_ptr<platform::file_info> unmounter(const_cast<platform::file_info*>(&info), [archive_path = info.archive_path](platform::file_info*) {
        wincdemu_unmount_iso(archive_path);
        winiso_unmount_iso(archive_path);
      });
      storage = unmounter;
    }
  }

  void cab_extract_file_contents(std::any& storage, const siege::platform::file_info& info, std::ostream& output)
  {
    content_cache& temp = cache_as_content_cache(storage);

    auto extract_cab_2 = [&] {
      return extract_file_contents_using_external_app(
        storage, info, output, [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
        fs::current_path(temp_path);
        std::stringstream command;
        command << '\"' << cab2_executable() << " x "
                <<  info.archive_path << " \"" << internal_file_path.filename().string() << "\""
                << '\"';
        return command.str(); }, [](const auto& info, const auto& temp_path, const auto&) {
        fs::current_path(temp_path);
        std::stringstream command;
        command << '\"' << cab2_executable() << " x " <<  info.archive_path << '\"';
        return command.str(); });
    };


    auto extract_cab_5 = [&] {
      return extract_file_contents_using_external_app(
        storage, info, output, [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
          fs::current_path(temp_path);
          std::stringstream command;
          command << '\"' << cab5_executable() << " x "
                  <<  info.archive_path << " \"" << internal_file_path.filename().string() << "\""
                  << '\"';
          return command.str(); }, [](const auto& info, const auto& temp_path, const auto&) {
          fs::current_path(temp_path);
          std::stringstream command;
          command << '\"' << cab5_executable() << " x " <<  info.archive_path << '\"';
          return command.str(); });
    };

    auto extract_cab_6 = [&] {
      return extract_file_contents_using_external_app(
        storage, info, output, [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
        fs::current_path(temp_path);
        std::stringstream command;
        command << '\"' << cab6_executable() << " x "
                <<  info.archive_path << " \"" << internal_file_path.filename().string() << "\""
                << '\"';
        return command.str(); }, [](const auto& info, const auto& temp_path, const auto&) {
        fs::current_path(temp_path);
        std::stringstream command;
        command << '\"' << cab6_executable() << " x " <<  info.archive_path << '\"';
        return command.str(); });
    };

    auto type = temp.additional_flags.find("cab_type");

    if (temp.additional_flags["cab2"].contains(info.archive_path))
    {
      extract_cab_2();
      return;
    }

    if (temp.additional_flags["cab5"].contains(info.archive_path))
    {
      extract_cab_5();
      return;
    }

    if (temp.additional_flags["cab6"].contains(info.archive_path))
    {
      extract_cab_6();
      return;
    }

    auto extracted = extract_cab_2();

    if (extracted)
    {
      return;
    }

    extracted = extract_cab_5();

    if (extracted)
    {
      return;
    }

    extracted = extract_cab_6();

    if (extracted)
    {
      return;
    }

    seven_extract_file_contents(storage, info, output);
  }
}// namespace siege::resource