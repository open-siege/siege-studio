#include <fstream>
#include <filesystem>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <iostream>

#include "resources/seven_zip_volume.hpp"

namespace fs = std::filesystem;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace studio::resources::seven_zip
{
  using folder_info = studio::resources::folder_info;

  constexpr auto file_record_tag = shared::to_tag<4>({ '7', 'z', 0xbc, 0xaf });

  std::string rtrim(std::string str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
  }

  std::string seven_zip_executable()
  {
#ifdef WIN32
    try
    {
      if (fs::exists("7z.exe"))
      {
        return "7z.exe";
      }

      constexpr static std::array<std::string_view, 3> commands = {{
        "where 7z",
        "echo %PROGRAMFILES%\\7-Zip\\7z.exe",
        "echo %PROGRAMFILES(x86)%\\7-Zip\\7z.exe"
      }};

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
    }
    catch (...)
    {

    }
#endif

    return "7z";
  }

  bool seven_zip_file_archive::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == file_record_tag;
  }

  bool seven_zip_file_archive::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<seven_zip_file_archive::content_info> seven_zip_file_archive::get_content_listing(std::istream& stream, const listing_query& query) const
  {
    static std::list<std::string> name_cache;
    static std::unordered_map<std::string, std::vector<seven_zip_file_archive::content_info>> stat_cache;

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

      std::vector<seven_zip_file_archive::content_info> results;
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
          seven_zip_file_archive::folder_info folder{};
          folder.archive_path = query.archive_path;
          folder.full_path = (query.archive_path / current->substr(name_indices.first)).make_preferred();
          folder.name = folder.full_path.filename().string();
          results.emplace_back(folder);
        }
        else
        {
          seven_zip_file_archive::file_info file{};

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

    std::vector<seven_zip_file_archive::content_info> results;

    auto is_valid = overloaded {
      [&](const seven_zip_file_archive::file_info& item) {
        return item.folder_path == query.folder_path;
      },
        [&](const seven_zip_file_archive::folder_info& item) {
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

  void seven_zip_file_archive::set_stream_position(std::istream& stream, const studio::resources::file_info& info) const
  {

  }

  void seven_zip_file_archive::extract_file_contents(std::istream& stream,
    const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage) const
  {
    auto delete_path = make_auto_remove_path();

    auto temp_path = fs::temp_directory_path() / (info.archive_path.stem().string() + "temp");
    auto internal_file_path = info.folder_path == info.archive_path ?
                                                                    info.filename :
                                                                    fs::relative(info.folder_path, info.archive_path) / info.filename;

    fs::create_directories(temp_path);


    static std::unordered_set<std::string> already_ran_commands;

    std::stringstream command;
    if (!storage.has_value())
    {
      command << '\"' << seven_zip_executable() << " x -y -o" << temp_path
              << ' ' <<  info.archive_path << " \"" << internal_file_path.string() << "\""
              << '\"';

      delete_path.reset(new fs::path(temp_path / internal_file_path));

      std::cout << command.str() << '\n';
      std::cout.flush();
      std::system(command.str().c_str());
    }
    else if (already_ran_commands.count(info.archive_path.string()) == 0)
    {
      command << '\"' << seven_zip_executable() << " x -y -o" << temp_path << ' ' <<  info.archive_path << '\"';

      delete_path.reset(new fs::path(temp_path));

      std::cout << command.str() << '\n';
      std::cout.flush();
      std::system(command.str().c_str());
      auto [command_iter, added] = already_ran_commands.emplace(info.archive_path.string());
      storage.value().get().temp.emplace(*command_iter, std::move(delete_path));
    }

    std::ifstream temp(temp_path / internal_file_path, std::ios::binary);

    std::copy_n(std::istreambuf_iterator(temp),
      fs::file_size(temp_path / internal_file_path),
      std::ostreambuf_iterator(output));
  }
}// namespace darkstar::vol
