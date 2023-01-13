#include <array>
#include <string>
#include <string_view>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <iostream>
#include <functional>
#include <variant>
#include <unordered_set>
#include "resources/external_utils.hpp"

namespace fs = std::filesystem;

namespace studio::resources
{
  [[nodiscard]] std::string rtrim(std::string str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
  }

  template<std::size_t Size>
  [[nodiscard]] inline std::optional<std::string> find_system_app(const std::array<std::string_view, Size>& commands)
  {
    auto output_path = make_auto_remove_path(fs::temp_directory_path() / "output.txt");

    for (auto command : commands)
    {
      std::stringstream command_str;
      command_str << command << " > " << *output_path;
      std::system(command_str.str().c_str());
      std::string temp;
      if (std::ifstream output(*output_path, std::ios::binary);
          output && std::getline(output, temp) && fs::exists(rtrim(temp)))
      {
        return "\"" + rtrim(temp) + "\"";
      }
    }

    return std::nullopt;
  }

  template<std::size_t Size>
  [[nodiscard]] std::string find_system_app(std::string_view app_name, const std::array<std::string_view, Size>& commands)
  {
#ifdef WIN32
      try
      {
        auto name_with_extension = fs::path(app_name).replace_extension(".exe");

        if (fs::exists(name_with_extension))
        {
          return name_with_extension.string();
        }

        return find_system_app(commands).value_or(std::string(app_name));
      }
      catch (...)
      {

      }
#endif

      return std::string(app_name);
  }

  [[nodiscard]] std::string find_cab_executable(std::string_view app_name, std::array<std::string_view, 2> search_names)
  {
#ifdef WIN32
    try
    {
      for (auto& search_name : search_names)
      {
        if (fs::exists(search_name))
        {
          return "\"" + (fs::current_path() / search_name).string() + "\"";
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
    std::array<std::string_view, 2> names = {{
      "i6comp.exe",
      "i6cmp13b\\i6comp.exe"
    }};

    return find_cab_executable("i6comp", names);
  }

  [[nodiscard]] std::string cab5_executable()
  {
    std::array<std::string_view, 2> names = {{
      "I5comp.exe",
      "i5comp21\\I5comp.exe"
    }};

    return find_cab_executable("I5comp", names);
  }

  [[nodiscard]] std::string cab2_executable()
  {
    std::array<std::string_view, 2> names = {{
      "ICOMP.EXE",
      "icomp95\\ICOMP.EXE"
    }};

    return find_cab_executable("ICOMP", names);
  }

  [[nodiscard]] std::string seven_zip_executable()
  {
    constexpr static std::array<std::string_view, 3> commands = {{
      "where 7z",
      "echo %PROGRAMFILES%\\7-Zip\\7z.exe",
      "echo %PROGRAMFILES(x86)%\\7-Zip\\7z.exe"
    }};

    return find_system_app("7z", commands);
  }

  [[nodiscard]] std::string wincdemu_executable()
  {
    constexpr static std::array<std::string_view, 3> commands = {{
      "where batchmnt",
      "echo %PROGRAMFILES%\\WinCDEmu\\batchmnt.exe",
      "echo %PROGRAMFILES(x86)%\\WinCDEmu\\batchmnt.exe"
    }};

    return find_system_app("batchmnt", commands);
  }

  [[nodiscard]] std::string power_iso_executable()
  {
    constexpr static std::array<std::string_view, 3> commands = {{
      "where piso",
      "echo %PROGRAMFILES%\\PowerISO\\piso.exe",
      "echo %PROGRAMFILES(x86)%\\PowerISO\\piso.exe"
    }};

    return find_system_app("piso", commands);
  }

  using content_map = std::unordered_map<std::string, std::vector<content_info>>;

  [[nodiscard]] std::vector<content_info> filter_results_for_query(const listing_query& query, const content_map::const_iterator& cache_entry)
  {
    std::vector<content_info> results;

    auto is_valid = overloaded {
      [&](const file_info& item) {
        return item.folder_path == query.folder_path;
      },
      [&](const folder_info& item) {
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
    std::vector<std::string> raw_contents;
    raw_contents.reserve(128);
    std::ifstream raw_listing(listing_filename, std::ios::binary);

    for (std::string temp; std::getline(raw_listing, temp);)
    {
      raw_contents.push_back(rtrim(std::move(temp)));
    }

    return raw_contents;
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

  std::vector<content_info> cached_get_content_listing(const listing_query& query, const std::function<std::vector<content_info>(const fs::path& listing_filename)>& get_listing)
  {
    static std::list<std::string> name_cache;
    static std::unordered_map<std::string, std::vector<content_info>> stat_cache;

    auto cache_key = fs::exists(query.archive_path) ?
                                                    query.archive_path.string() + std::to_string(fs::file_size(query.archive_path)) :
                                                    query.archive_path.string();

    auto cache_entry = stat_cache.find(cache_key);

    if (cache_entry == stat_cache.end())
    {
      auto listing_filename = make_auto_remove_path(fs::temp_directory_path() / (query.archive_path.stem().string() + "-listing.txt"));
      cache_entry = stat_cache.emplace(cache_key, std::move(get_listing(*listing_filename))).first;
    }

    return filter_results_for_query(query, cache_entry);
  }

  std::vector<content_info> wincdemu_get_content_listing(const listing_query& query)
  {
    return cached_get_content_listing(query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto lines = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << wincdemu_executable() << ' ' << query.archive_path << " > " << listing_filename << '\"';
      });

      if (lines.empty())
      {
        return {};
      }

      if (lines[0].find("successfully") == std::string::npos)
      {
        return {};
      }

      lines = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << wincdemu_executable() << " /list > " << listing_filename << '\"';
      });

      auto drive_letter_iter = std::find_if(lines.begin(), lines.end(), [&](auto& line) {
        return line.find(query.archive_path.string()) != std::string::npos;
      });

      if (drive_letter_iter == lines.end())
      {
        return {};
      }

      auto drive_letter = drive_letter_iter->substr(0, drive_letter_iter->find(query.archive_path.string()));
      if (!fs::exists(drive_letter))
      {
        return {};
      }

      std::vector<content_info> results;
      results.reserve(128);

      for (const fs::directory_entry& dir_entry :
        fs::recursive_directory_iterator(drive_letter))
      {
        if (dir_entry.is_directory())
        {
          folder_info folder{};
          folder.full_path = dir_entry.path();
          folder.archive_path = drive_letter;
          folder.name = folder.full_path.filename().string();
          results.emplace_back(folder);
        }
        else
        {
          file_info file{};
          file.archive_path = drive_letter;
          file.compression_type = compression_type::none;
          file.size = dir_entry.file_size();
          file.folder_path = dir_entry.path().parent_path();
          file.filename = dir_entry.path().filename();
          results.emplace_back(file);
        }
      }

      execute_command([&](auto& command) {
        command << '\"' << wincdemu_executable() << " /unmount " << query.archive_path << '\"';
      });

      return results;
    });
  }

  std::vector<content_info> winiso_get_content_listing(const listing_query& query)
  {
    return cached_get_content_listing(query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto lines = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << "powershell -c \"(Mount-DiskImage -ImagePath " << query.archive_path << " | Get-Volume).DriveLetter + ':'\" > " << listing_filename << '\"';
      });

      if (lines.empty())
      {
        return {};
      }

      if (lines[0].find(":") == std::string::npos)
      {
        return {};
      }


      auto drive_letter = lines[0];
      if (!fs::exists(drive_letter))
      {
        return {};
      }

      std::vector<content_info> results;
      results.reserve(128);

      for (const fs::directory_entry& dir_entry :
        fs::recursive_directory_iterator(drive_letter))
      {
        if (dir_entry.is_directory())
        {
          folder_info folder{};
          folder.full_path = dir_entry.path();
          folder.archive_path = drive_letter;
          folder.name = folder.full_path.filename().string();
          results.emplace_back(folder);
        }
        else
        {
          file_info file{};
          file.archive_path = drive_letter;
          file.compression_type = compression_type::none;
          file.size = dir_entry.file_size();
          file.folder_path = dir_entry.path().parent_path();
          file.filename = dir_entry.path().filename();
          results.emplace_back(file);
        }
      }

      execute_command([&](auto& command) {
        command << '\"' << "powershell -c \"Dismount-DiskImage -ImagePath " << query.archive_path << '\"' << '\"';
      });

      return results;
    });
  }

  std::vector<content_info> zip_get_content_listing(const listing_query& query)
  {
    return cached_get_content_listing(query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto raw_contents = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << seven_zip_executable() << " l " << query.archive_path << " > " << listing_filename << '\"';
      });

      auto header_iter = std::find_if(raw_contents.begin(), raw_contents.end(), [](auto& row) {
        return row.find("Date") != std::string::npos &&
               row.find("Time") != std::string::npos &&
               row.find("Attr") != std::string::npos &&
               row.find("Size") != std::string::npos &&
               row.find("Compressed") != std::string::npos &&
               row.find("Name") != std::string::npos;
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
          folder_info folder{};
          folder.archive_path = query.archive_path;
          folder.full_path = (query.archive_path / current->substr(name_indices.first)).make_preferred();
          folder.name = folder.full_path.filename().string();
          results.emplace_back(folder);
        }
        else
        {
          file_info file{};

          auto name = fs::path(current->substr(name_indices.first));

          file.filename = name.filename();
          file.archive_path = query.archive_path;
          file.folder_path = (query.archive_path / name.make_preferred()).parent_path();
          file.compression_type = compression_type::lzh;

          try
          {
            auto size = current->substr(size_indices.first, size_indices.second - size_indices.first);

            if (auto iter = std::find_if(size.begin(), size.end(), [](auto val) { return std::isdigit(val);});
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

  void add_folders(const file_info& file,
    const fs::path& archive_path,
    std::unordered_set<std::string>& folders,
    const std::function<void(folder_info&&)>& move_result)
  {
    auto [folder_iter, folder_added] = folders.emplace(file.folder_path.string());

    if (folder_added && file.folder_path != archive_path)
    {
      auto relative_path = fs::relative(file.folder_path, archive_path);

      auto new_path = archive_path;

      for (auto& segment : relative_path)
      {
        folder_info folder{};
        new_path = new_path / segment;
        folder.full_path = new_path;
        folder.archive_path = archive_path;
        folder.name = folder.full_path.filename().string();
        move_result(std::move(folder));
      }
    }
  }

  std::vector<content_info> cab5_get_content_listing(const std::string_view exe_path, const listing_query& query)
  {
    return cached_get_content_listing(query, [&](const auto& listing_filename) -> std::vector<content_info> {
      auto raw_contents = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << exe_path << " l " << query.archive_path << " > " << listing_filename << '\"';
      });
      std::vector<content_info> results;
      results.reserve(raw_contents.size());

      std::unordered_set<std::string> folders;

      constexpr static auto size_index = 17;
      constexpr static auto size_count = 9;
      constexpr static auto name_index = 47;

      for (auto& current : raw_contents)
      {
        file_info file{};

        auto name = fs::path(current.substr(name_index));

        file.filename = name.filename();
        file.archive_path = query.archive_path;
        file.folder_path = (query.archive_path / name.make_preferred()).parent_path();

        add_folders(file, query.archive_path, folders, [&](folder_info&& folder) {
          results.emplace_back(folder);
        });
        file.compression_type = compression_type::lzh;

        try
        {
          auto size = current.substr(size_index, size_count);

          if (auto iter = std::find_if(size.begin(), size.end(), [](auto val) { return std::isdigit(val);});
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

  std::vector<content_info> cab2_get_content_listing(const listing_query& query)
  {
    return cached_get_content_listing(query, [&](const auto& listing_filename) -> std::vector<content_info>
    {
      auto raw_contents = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << cab2_executable() << " l " << query.archive_path << " > " << listing_filename << '\"';
      });

      auto header_iter = std::find_if(raw_contents.begin(), raw_contents.end(), [](auto& row) {
        return row.find("Date") != std::string::npos &&
               row.find("Time") != std::string::npos &&
               row.find("OrigSize") != std::string::npos &&
               row.find("Attr") != std::string::npos &&
               row.find("CompSize") != std::string::npos &&
               row.find("Name") != std::string::npos;
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
          folder_info folder{};
          folder.archive_path = query.archive_path;
          folder.full_path = (query.archive_path / current->substr(name_indices.first)).make_preferred();
          folder.name = folder.full_path.filename().string();
          results.emplace_back(folder);
        }
        else
        {
          file_info file{};

          auto name = fs::path(current->substr(name_indices.first));

          file.filename = name.filename();
          file.archive_path = query.archive_path;
          file.folder_path = (query.archive_path / name.make_preferred()).parent_path();
          file.compression_type = compression_type::lzh;

          add_folders(file, query.archive_path, folders, [&](folder_info&& folder) {
            results.emplace_back(folder);
          });

          try
          {
            auto size = current->substr(size_indices.first, size_indices.second - size_indices.first);

            if (auto iter = std::find_if(size.begin(), size.end(), [](auto val) { return std::isdigit(val);});
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


  std::vector<content_info> cab_get_content_listing(const listing_query& query)
  {
    auto cab2_listing = cab2_get_content_listing(query);

    if (!cab2_listing.empty())
    {
      return cab2_listing;
    }

    auto cab5_listing = cab5_get_content_listing(cab5_executable(), query);

    if (!cab5_listing.empty())
    {
      return cab5_listing;
    }

    auto cab6_listing = cab5_get_content_listing(cab6_executable(), query);

    if (!cab6_listing.empty())
    {
      return cab6_listing;
    }

    return zip_get_content_listing(query);
  }

  std::vector<content_info> power_iso_get_content_listing(const listing_query& query)
  {
    return cached_get_content_listing(query, [&](const auto& listing_filename) -> std::vector<content_info>
    {
      auto raw_contents = execute_command(listing_filename, [&](auto& command) {
        command << '\"' << power_iso_executable() << " list " << query.archive_path << " / -r" << " > " << listing_filename << '\"';
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

        if (row->find("Files, ") != std::string::npos &&
            row->find("Folders") != std::string::npos &&
            row->find("bytes") != std::string::npos)
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

          folder_info folder{};
          folder.archive_path = query.archive_path;
          folder.name = name;
          folder.full_path = current_path / name;

          results.emplace_back(folder);
        }
        else
        {
          file_info file{};

          auto name = row->substr(row->rfind("  ") + 2);

          if (name.empty())
          {
            continue;
          }

          file.filename = name;
          file.archive_path = query.archive_path;
          file.folder_path = current_path;
          file.compression_type = compression_type::lzh;

          results.emplace_back(file);
        }
      }

      return results;
    });
  }

  std::vector<content_info> iso_get_content_listing(const listing_query& query)
  {
    auto listing = power_iso_get_content_listing(query);

    if (!listing.empty())
    {
      return listing;
    }

    listing = zip_get_content_listing(query);

    if (!listing.empty())
    {
      return listing;
    }

    listing = wincdemu_get_content_listing(query);

    if (!listing.empty())
    {
      return listing;
    }

    return winiso_get_content_listing(query);
  }

  [[maybe_unused]] bool extract_file_contents_using_external_app(const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage,
    std::string (*extract_one_command)(const studio::resources::file_info&, const fs::path&, const fs::path&),
    std::string (*extract_all_command)(const studio::resources::file_info&, const fs::path&, const fs::path&))
  {
    auto delete_path = make_auto_remove_path();

    auto temp_path = fs::temp_directory_path() / (info.archive_path.stem().string() + "temp");
    auto internal_file_path = info.folder_path == info.archive_path ?
                                                                    info.filename :
                                                                    fs::relative(info.folder_path, info.archive_path) / info.filename;

    auto current_working_path = fs::current_path();
    fs::create_directories(temp_path);
    static std::unordered_set<std::string> already_ran_commands;

    if (!storage.has_value())
    {
      delete_path.reset(new fs::path(temp_path / internal_file_path));

      std::cout << extract_one_command(info, temp_path, internal_file_path) << '\n';
      std::cout.flush();
      std::system(extract_one_command(info, temp_path, internal_file_path).c_str());
    }
    else if (already_ran_commands.count(info.archive_path.string()) == 0)
    {
      delete_path.reset(new fs::path(temp_path));

      std::cout << extract_all_command(info, temp_path, internal_file_path) << '\n';
      std::cout.flush();
      std::system(extract_all_command(info, temp_path, internal_file_path).c_str());
      auto [command_iter, added] = already_ran_commands.emplace(info.archive_path.string());
      storage.value().get().temp.emplace(*command_iter, std::move(delete_path));
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

  void iso_extract_file_contents(const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage)
  {
    extract_file_contents_using_external_app(info, output, storage,
      [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
        std::stringstream command;
        command << '\"' << power_iso_executable() << " extract " << info.archive_path << " / -y -od " << temp_path << '\"';
        return command.str();
      }, [] (const auto& info, const auto& temp_path, const auto&) {
        std::stringstream command;
        command << '\"' << power_iso_executable() << " extract " << info.archive_path << " / -y -od " << temp_path << '\"';
        return command.str();
      });
  }

  void seven_extract_file_contents(const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage)
  {
    extract_file_contents_using_external_app(info, output, storage,
      [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
        std::stringstream command;
        command << '\"' << seven_zip_executable() << " x -y -o" << temp_path
                << ' ' <<  info.archive_path << " \"" << internal_file_path.string() << "\""
                << '\"';
        return command.str();
      }, [] (const auto& info, const auto& temp_path, const auto&) {
        std::stringstream command;
        command << '\"' << seven_zip_executable() << " x -y -o" << temp_path << ' ' <<  info.archive_path << '\"';
        return command.str();
      });
  }

  void cab_extract_file_contents(const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage)
  {
    auto extracted = extract_file_contents_using_external_app(info, output, storage,
      [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
        fs::current_path(temp_path);
        std::stringstream command;
        command << '\"' << cab2_executable() << " x "
                <<  info.archive_path << " \"" << internal_file_path.filename().string() << "\""
                << '\"';
        return command.str();
    }, [] (const auto& info, const auto& temp_path, const auto&) {
        fs::current_path(temp_path);
        std::stringstream command;
        command << '\"' << cab2_executable() << " x " <<  info.archive_path << '\"';
        return command.str();
      });

    if (!extracted)
    {
      extracted = extract_file_contents_using_external_app(info, output, storage,
        [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
          fs::current_path(temp_path);
          std::stringstream command;
          command << '\"' << cab5_executable() << " x "
                  <<  info.archive_path << " \"" << internal_file_path.filename().string() << "\""
                  << '\"';
          return command.str();
        }, [] (const auto& info, const auto& temp_path, const auto&) {
          fs::current_path(temp_path);
          std::stringstream command;
          command << '\"' << cab5_executable() << " x " <<  info.archive_path << '\"';
          return command.str();
        });
    }

    if (!extracted)
    {
      extracted = extract_file_contents_using_external_app(info, output, storage,
        [](const auto& info, const auto& temp_path, const auto& internal_file_path) {
          fs::current_path(temp_path);
          std::stringstream command;
          command << '\"' << cab6_executable() << " x "
                  <<  info.archive_path << " \"" << internal_file_path.filename().string() << "\""
                  << '\"';
          return command.str();
        }, [] (const auto& info, const auto& temp_path, const auto&) {
          fs::current_path(temp_path);
          std::stringstream command;
          command << '\"' << cab6_executable() << " x " <<  info.archive_path << '\"';
          return command.str();
        });
    }

    if (!extracted)
    {
      return seven_extract_file_contents(info, output, storage);
    }
  }
}