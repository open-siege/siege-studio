#include <string_view>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include "games/games.hpp"

namespace fs = std::filesystem;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int main()
{
  std::string_view game = "earthsiege2";
  fs::path destination_path = "C:\\test\\Earthshiege 2";


  auto maybe_info = get_info_for_game(game);

  if (!maybe_info.has_value())
  {
    return -1;
  }

  auto& info = maybe_info.value();

  std::unordered_map<std::string, std::string_view> variable_values;

  std::transform(info.variables.begin(), info.variables.end(), std::inserter(variable_values, variable_values.begin()), [](auto& item){
    return std::make_pair("<" + std::string(item.first) + ">", item.second.front());
  });

  std::vector<fs::path> search_paths;
  search_paths.reserve(27);

  search_paths.emplace_back(fs::current_path());

  std::vector<char> temp;

  const static auto separator = []{
    auto value = (fs::path("a") / "b").string();
    value.pop_back();
    return value.back();
  }();

  for (auto i = 0; i < 26; ++i)
  {
    temp.emplace_back('A' + i);
    temp.emplace_back(':');
    temp.emplace_back(separator);
    search_paths.emplace_back(std::string_view(temp.data(), temp.size()));
    temp.clear();
  }

  std::unordered_map<std::string, std::string> expected_files;
  std::unordered_map<std::string, std::string> wildcard_files;

  auto last_variable_used = variable_values.begin();

  for (const auto& mapping : info.directory_mappings)
  {
    const auto& src_path = mapping.first;
    const auto& dst_path = mapping.second;
    auto star_index = src_path.find('*');
    auto bracket_index = src_path.find('<');

    if (bracket_index != std::string_view::npos)
    {
      std::decay_t<decltype(std::string_view::npos)> variable_index;

      auto do_replace = [&](auto current_iter){
        if (current_iter == variable_values.end())
        {
          return false;
        }

        variable_index = src_path.find(current_iter->first);

        if (variable_index != std::string_view::npos)
        {
          if (star_index == std::string_view::npos)
          {
            expected_files.emplace(std::string(src_path).replace(variable_index,
                                     current_iter->first.size(),
                                     current_iter->second), dst_path);
          }
          else
          {
            wildcard_files.emplace(std::string(src_path).replace(variable_index,
                                     current_iter->first.size(),
                                     current_iter->second), dst_path);
          }


          return true;
        }

        return false;
      };

      auto replaced = do_replace(last_variable_used);

      if (!replaced)
      {
        for (auto iter = variable_values.begin(); iter != variable_values.end(); std::advance(iter, 1))
        {
          replaced = do_replace(iter);

          if (replaced)
          {
            last_variable_used = iter;
            break;
          }
        }
      }
    }
    else if (star_index != std::string_view::npos)
    {
      wildcard_files.emplace(src_path, dst_path);
    }
    else
    {
      expected_files.emplace(src_path, dst_path);
    }
  }

  std::optional<fs::path> content_path;
  std::unordered_map<std::string, std::string> found_files;

  for (const auto& path : search_paths)
  {
    if (fs::exists(path))
    {
      for (const auto& [source_file, destination_rule] : expected_files)
      {
        auto path_of_interest = path / source_file;
        if (fs::exists(path_of_interest) &&
            (fs::is_directory(path_of_interest) ||
                std::ifstream(path_of_interest, std::ios::binary))
          )
        {
          if (destination_rule == "=")
          {
            found_files.emplace(path_of_interest.string(), std::string_view(source_file));
          }
          else
          {
            found_files.emplace(path_of_interest.string(), destination_rule);
          }
        }
      }

      if (found_files.size() == expected_files.size())
      {
        content_path.emplace(path);
        break;
      }
    }
    found_files.clear();
  }

  if (content_path.has_value())
  {
    std::cout << "Found " << game << " content at " << content_path.value() << '\n';

    for (const auto& [source_wildcard, destination_rule] : wildcard_files)
    {
      auto working_path = content_path.value() / fs::path(source_wildcard).parent_path();
      auto working_value = fs::path(source_wildcard).filename();

      if (fs::exists(working_path))
      {
        for (const auto& dir_entry : std::filesystem::directory_iterator{working_path})
        {
          if (working_value.stem() == "*" && dir_entry.path().extension() == working_value.extension())
          {
            if (destination_rule == "=")
            {
              found_files.emplace(dir_entry.path().string(), dir_entry.path().string().erase(0, working_path.string().size()));
            }
            else
            {
              found_files.emplace(dir_entry.path().string(), destination_rule);
            }
          }
        }
      }

      std::cout << "Wildcard file at " << working_path << " " << working_value << '\n';
    }

    fs::create_directories(destination_path);
    for (const auto& [src, dst] : found_files)
    {
      auto new_path = destination_path / dst;

      bool is_file = false;

      if (!fs::is_directory(src))
      {
        is_file = true;
        fs::create_directories(new_path.parent_path());
      }

      try
      {
        if (is_file && fs::exists(new_path) && fs::file_size(new_path) == 0)
        {
          std::cout << "Removing empty file " << new_path << '\n';
          fs::remove(new_path);
        }

        std::cout << "Copying from " << src << " to " << new_path << '\n';
        fs::copy(src, new_path, fs::copy_options::recursive | fs::copy_options::skip_existing);

        fs::permissions(new_path,  fs::perms::owner_read |
                                    fs::perms::owner_write |
                                    fs::perms::group_read |
                                    fs::perms::group_write,
          fs::perm_options::add);
      }
      catch(const std::exception& ex)
      {
        std::cerr << "Error: " << ex.what() << '\n';
      }
    }

    for (const auto& file : info.generated_files)
    {
      const auto& dst = file.first;
      const auto& rule = file.second;
      auto dst_path = destination_path / dst;

      fs::create_directories(dst_path.parent_path());

      std::visit(overloaded {
                   [&](const literal_template& arg) {
                     std::ofstream output(dst_path, std::ios::binary);
                     output << arg.value;
                   },
                   [&](const internal_generated_template& arg) {

                     template_args args{};

                     auto dst_path_str = dst_path.string();
                     args.file_path = dst_path_str.c_str(); // it allows functions to also take the form of char**
                     args.file_path_size = dst_path_str.size();

                     args.original_path = dst.data();
                     args.original_path_size = dst.size();

                     auto result = arg.generate_template(&args);

                     switch (result)
                     {
                       case std::errc::no_such_file_or_directory:
                       {
                         std::cerr << "Function for " << dst_path_str << " should have created the file automatically.";
                         break;
                       }
                       default:
                       {
                         break;
                       }
                     }
                   },
                   [](const auto& arg) { }
                 }, rule);
    }
  }
  else
  {
    std::cout << "Did not find " << game << " content at \n";
  }
}