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

#include "resources/iso_volume.hpp"

namespace fs = std::filesystem;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace studio::resources::iso
{
  using folder_info = studio::resources::folder_info;

  constexpr auto file_record_tag = shared::to_tag<4>({ '7', 'z', 0xbc, 0xaf });

  std::string rtrim(std::string str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
  }

  std::string power_iso_executable()
  {
#ifdef WIN32
    try
    {
      if (fs::exists("piso.exe"))
      {
        return "piso.exe";
      }

      constexpr static std::array<std::string_view, 3> commands = {{
        "where piso",
        "echo %PROGRAMFILES%\\PowerISO\\piso.exe",
        "echo %PROGRAMFILES(x86)%\\PowerISO\\piso.exe"
      }};

      for (auto command : commands)
      {
        std::stringstream command_str;
        auto output_path = fs::temp_directory_path()/ "output.txt";
        command_str << command << " > " << output_path;
        std::system(command_str.str().c_str());
        std::string temp;
        if (std::ifstream output(output_path, std::ios::binary);
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

    return "piso";
  }

  bool iso_file_archive::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag != file_record_tag;
  }

  bool iso_file_archive::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<iso_file_archive::content_info> iso_file_archive::get_content_listing(std::istream& stream, const listing_query& query) const
  {
    static std::list<std::string> name_cache;
    static std::unordered_map<std::string, std::vector<iso_file_archive::content_info>> stat_cache;

    auto cache_key = fs::exists(query.archive_path) ?
                                                    query.archive_path.string() + std::to_string(fs::file_size(query.archive_path)) :
                                                    query.archive_path.string();

    auto cache_entry = stat_cache.find(cache_key);

    if (cache_entry == stat_cache.end())
    {
      auto listing_filename = fs::temp_directory_path() / (query.archive_path.stem().string() + "-listing.txt");

      std::stringstream command;
      command << '\"' << power_iso_executable() << " list " << query.archive_path << " / -r" << " > " << listing_filename << '\"';
      std::cout << command.str() << '\n';
      std::cout.flush();
      std::system(command.str().c_str());

      std::vector<std::string> raw_contents;
      raw_contents.reserve(256);

      std::ifstream raw_listing(listing_filename, std::ios::binary);

      for (std::string temp; std::getline(raw_listing, temp);)
      {
        raw_contents.push_back(rtrim(std::move(temp)));
      }

      std::vector<iso_file_archive::content_info> results;

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

          iso_file_archive::folder_info folder{};
          folder.archive_path = query.archive_path;
          folder.name = name;
          folder.full_path = current_path / name;

          results.emplace_back(folder);
        }
        else
        {
          iso_file_archive::file_info file{};

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

    std::vector<iso_file_archive::content_info> results;

    auto is_valid = overloaded {
      [&](const iso_file_archive::file_info& item) {
        return item.folder_path == query.folder_path;
      },
        [&](const iso_file_archive::folder_info& item) {
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

  void iso_file_archive::set_stream_position(std::istream& stream, const studio::resources::file_info& info) const
  {

  }

  void iso_file_archive::extract_file_contents(std::istream& stream,
    const studio::resources::file_info& info,
    std::ostream& output,
    std::optional<std::reference_wrapper<batch_storage>> storage) const
  {
    auto temp_path = fs::temp_directory_path() / (info.archive_path.stem().string() + "temp");
    auto internal_file_path = info.folder_path == info.archive_path ?
                                                                    info.filename :
                                                                    fs::relative(info.folder_path, info.archive_path) / info.filename;

    fs::create_directories(temp_path);


    static std::unordered_set<std::string> already_ran_commands;

    std::stringstream command;

    if (already_ran_commands.count(info.archive_path.string()) == 0)
    {
      command << '\"' << power_iso_executable() << " extract " << info.archive_path << " / -y -od " << temp_path << '\"';
      std::cout << command.str() << '\n';
      std::cout.flush();
      std::system(command.str().c_str());
      already_ran_commands.emplace(info.archive_path.string());
    }

    temp_path = temp_path / "Data";

    std::ifstream temp(temp_path / internal_file_path, std::ios::binary);

    std::copy_n(std::istreambuf_iterator(temp),
      fs::file_size(temp_path / internal_file_path),
      std::ostreambuf_iterator(output));
  }
}// namespace darkstar::vol
