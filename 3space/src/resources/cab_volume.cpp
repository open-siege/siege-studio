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

#include "resources/cab_volume.hpp"

namespace fs = std::filesystem;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace studio::resources::cab
{
  using folder_info = studio::resources::folder_info;

  constexpr auto file_record_tag = shared::to_tag<4>({ 'I', 'S', 'c', 0x28 });

  std::string rtrim(std::string str)
  {
    auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
    str.erase( it1.base() , str.end() );
    return str;
  }

  std::string cab_executable()
  {
#ifdef WIN32
    try
    {
      if (fs::exists("I5comp.exe"))
      {
        return "\"" + (fs::current_path() / "I5comp.exe").string() + "\"";
      }

      if (fs::exists("i5comp21\\I5comp.exe"))
      {
        return "\"" + (fs::current_path() / "i5comp21\\I5comp.exe").string() + "\"";
      }
    }
    catch (...)
    {

    }
#endif

    return "I5comp";
  }

  bool cab_file_archive::is_supported(std::istream& stream)
  {
    std::array<std::byte, 4> tag{};
    stream.read(reinterpret_cast<char *>(tag.data()), sizeof(tag));

    stream.seekg(-int(sizeof(tag)), std::ios::cur);

    return tag == file_record_tag;
  }

  bool cab_file_archive::stream_is_supported(std::istream& stream) const
  {
    return is_supported(stream);
  }

  std::vector<cab_file_archive::content_info> cab_file_archive::get_content_listing(std::istream& stream, const listing_query& query) const
  {
    static std::list<std::string> name_cache;
    static std::unordered_map<std::string, std::vector<cab_file_archive::content_info>> stat_cache;

    auto cache_key = fs::exists(query.archive_path) ?
                                                    query.archive_path.string() + std::to_string(fs::file_size(query.archive_path)) :
                                                    query.archive_path.string();

    auto cache_entry = stat_cache.find(cache_key);

    if (cache_entry == stat_cache.end())
    {
      auto listing_filename = make_auto_remove_path(fs::temp_directory_path() / (query.archive_path.stem().string() + "-listing.txt"));

      std::stringstream command;
      command << '\"' << cab_executable() << " l " << query.archive_path << " > " << *listing_filename << '\"';
      std::cout << command.str() << '\n';
      std::system(command.str().c_str());

      std::vector<std::string> raw_contents;
      raw_contents.reserve(128);

      std::ifstream raw_listing(*listing_filename, std::ios::binary);

      for (std::string temp; std::getline(raw_listing, temp);)
      {
        raw_contents.push_back(rtrim(std::move(temp)));
      }

      std::vector<cab_file_archive::content_info> results;
      results.reserve(raw_contents.size());

      std::unordered_set<std::string> folders;

      constexpr static auto size_index = 17;
      constexpr static auto size_count = 9;
      constexpr static auto name_index = 47;

      for (auto& current : raw_contents)
      {
        cab_file_archive::file_info file{};

        auto name = fs::path(current.substr(name_index));

        file.filename = name.filename();
        file.archive_path = query.archive_path;
        file.folder_path = (query.archive_path / name.make_preferred()).parent_path();
        auto [folder_iter, folder_added] = folders.emplace(file.folder_path.string());

        if (folder_added && file.folder_path != query.archive_path)
        {
          auto relative_path = fs::relative(file.folder_path, query.archive_path);

          auto new_path = query.archive_path;

          for (auto& segment : relative_path)
          {
            cab_file_archive::folder_info folder{};
            new_path = new_path / segment;
            folder.full_path = new_path;
            folder.archive_path = query.archive_path;
            folder.name = folder.full_path.filename().string();
            results.emplace_back(std::move(folder));
          }
        }

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

    std::vector<cab_file_archive::content_info> results;

    auto is_valid = overloaded {
      [&](const cab_file_archive::file_info& item) {
        return item.folder_path == query.folder_path;
      },
        [&](const cab_file_archive::folder_info& item) {
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

  void cab_file_archive::set_stream_position(std::istream& stream, const studio::resources::file_info& info) const
  {

  }

  void cab_file_archive::extract_file_contents(std::istream& stream,
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

    auto current_working_path = fs::current_path();
    auto cab_exe = cab_executable();

    fs::current_path(temp_path);
    if (!storage.has_value())
    {
      command << '\"' << cab_exe << " x "
              <<  info.archive_path << " \"" << internal_file_path.filename().string() << "\""
              << '\"';

      delete_path.reset(new fs::path(temp_path / internal_file_path));

      std::cout << command.str() << '\n';
      std::cout.flush();
      std::system(command.str().c_str());

    }
    else if (already_ran_commands.count(info.archive_path.string()) == 0)
    {
      command << '\"' << cab_exe << " x " <<  info.archive_path << '\"';

      delete_path.reset(new fs::path(temp_path));
      std::cout << command.str() << '\n';
      std::cout.flush();
      std::system(command.str().c_str());
      auto [command_iter, added] = already_ran_commands.emplace(info.archive_path.string());
      storage.value().get().temp.emplace(*command_iter, std::move(delete_path));
    }

    fs::current_path(current_working_path);

    std::ifstream temp(temp_path / internal_file_path, std::ios::binary);

    std::copy_n(std::istreambuf_iterator(temp),
      fs::file_size(temp_path / internal_file_path),
      std::ostreambuf_iterator(output));
  }
}// namespace darkstar::vol
