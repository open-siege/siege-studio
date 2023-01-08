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
#include "resources/archive_plugin.hpp"

namespace fs = std::filesystem;

namespace studio::resources
{
  using content_info = std::variant<folder_info, studio::resources::file_info>;

  std::string rtrim(std::string str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
  }

  template<std::size_t Size>
  inline std::optional<std::string> find_system_app(const std::array<std::string_view, Size>& commands)
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
  std::string find_system_app(std::string_view app_name, const std::array<std::string_view, Size>& commands)
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

  std::string find_cab_executable(std::string_view app_name, std::array<std::string_view, 2> search_names)
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


  std::string cab6_executable()
  {
    std::array<std::string_view, 2> names = {{
      "i6comp.exe",
      "i6cmp13b\\i6comp.exe"
    }};

    return find_cab_executable("i6comp", names);
  }

  std::string cab5_executable()
  {
    std::array<std::string_view, 2> names = {{
      "I5comp.exe",
      "i5comp21\\I5comp.exe"
    }};

    return find_cab_executable("I5comp", names);
  }

  std::string cab2_executable()
  {
    std::array<std::string_view, 2> names = {{
      "ICOMP.EXE",
      "icomp95\\ICOMP.EXE"
    }};

    return find_cab_executable("ICOMP", names);
  }

  std::string seven_zip_executable()
  {
    constexpr static std::array<std::string_view, 3> commands = {{
      "where 7z",
      "echo %PROGRAMFILES%\\7-Zip\\7z.exe",
      "echo %PROGRAMFILES(x86)%\\7-Zip\\7z.exe"
    }};

    return find_system_app("7z", commands);
  }

  std::string power_iso_executable()
  {
    constexpr static std::array<std::string_view, 3> commands = {{
      "where piso",
      "echo %PROGRAMFILES%\\PowerISO\\piso.exe",
      "echo %PROGRAMFILES(x86)%\\PowerISO\\piso.exe"
    }};

    return "piso";
  }

  std::vector<content_info> zip_get_content_listing(const listing_query& query)
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

      std::stringstream command;
      command << '\"' << seven_zip_executable() << " l " << query.archive_path << " > " << *listing_filename << '\"';
      std::cout << command.str() << '\n';
      std::system(command.str().c_str());

      std::vector<std::string> raw_contents;
      raw_contents.reserve(128);

      std::ifstream raw_listing(*listing_filename, std::ios::binary);

      for (std::string temp; std::getline(raw_listing, temp);)
      {
        raw_contents.push_back(rtrim(std::move(temp)));
      }

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

      cache_entry = stat_cache.emplace(cache_key, std::move(results)).first;
    }

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

  void add_folders(const file_info& file,
    const fs::path& archive_path,
    std::unordered_set<std::string>& folders,
    std::function<void(folder_info&&)> move_result)
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
    static std::list<std::string> name_cache;
    static std::unordered_map<std::string, std::vector<content_info>> stat_cache;

    auto cache_key = fs::exists(query.archive_path) ?
                                                    query.archive_path.string() + std::to_string(fs::file_size(query.archive_path)) :
                                                    query.archive_path.string();

    auto cache_entry = stat_cache.find(cache_key);

    if (cache_entry == stat_cache.end())
    {
      auto listing_filename = make_auto_remove_path(fs::temp_directory_path() / (query.archive_path.stem().string() + "-listing.txt"));

      std::stringstream command;
      command << '\"' << exe_path << " l " << query.archive_path << " > " << *listing_filename << '\"';
      std::cout << command.str() << '\n';
      std::system(command.str().c_str());

      std::vector<std::string> raw_contents;
      raw_contents.reserve(128);

      std::ifstream raw_listing(*listing_filename, std::ios::binary);

      for (std::string temp; std::getline(raw_listing, temp);)
      {
        raw_contents.push_back(rtrim(std::move(temp)));
      }

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

      cache_entry = stat_cache.emplace(cache_key, std::move(results)).first;
    }

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

  std::vector<content_info> cab2_get_content_listing(const listing_query& query)
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

      std::stringstream command;
      command << '\"' << cab2_executable() << " l " << query.archive_path << " > " << *listing_filename << '\"';
      std::cout << command.str() << '\n';
      std::system(command.str().c_str());

      std::vector<std::string> raw_contents;
      raw_contents.reserve(128);

      std::ifstream raw_listing(*listing_filename, std::ios::binary);

      for (std::string temp; std::getline(raw_listing, temp);)
      {
        raw_contents.push_back(rtrim(std::move(temp)));
      }

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

      cache_entry = stat_cache.emplace(cache_key, std::move(results)).first;
    }

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

  std::vector<content_info> iso_get_content_listing(const listing_query& query)
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

      std::stringstream command;
      command << '\"' << power_iso_executable() << " list " << query.archive_path << " / -r" << " > " << *listing_filename << '\"';
      std::cout << command.str() << '\n';
      std::cout.flush();
      std::system(command.str().c_str());

      std::vector<std::string> raw_contents;
      raw_contents.reserve(256);

      std::ifstream raw_listing(*listing_filename, std::ios::binary);

      for (std::string temp; std::getline(raw_listing, temp);)
      {
        raw_contents.push_back(rtrim(std::move(temp)));
      }

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

      cache_entry = stat_cache.emplace(cache_key, std::move(results)).first;
    }

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
}