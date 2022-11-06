#include <string_view>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <deque>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cpr/cpr.h>
#include "games/games.hpp"
#include "resources/darkstar_volume.hpp"
#include "resources/three_space_volume.hpp"
#include "resources/trophy_bass_volume.hpp"
#include "resources/zip_volume.hpp"
#include "resources/cyclone_volume.hpp"
#include "resources/sword_volume.hpp"
#include "resources/resource_explorer.hpp"

namespace fs = std::filesystem;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


namespace dio
{
  namespace vol = studio::resources::vol;
  namespace zip = studio::resources::zip;
  namespace cln = studio::resources::cln;
  namespace atd = studio::resources::atd;
}

studio::resources::resource_explorer create_resource_explorer()
{
  studio::resources::resource_explorer archive;

  archive.add_archive_type(".tbv", std::make_unique<dio::vol::trophy_bass::tbv_file_archive>());
  archive.add_archive_type(".rbx", std::make_unique<dio::vol::trophy_bass::rbx_file_archive>());
  archive.add_archive_type(".rmf", std::make_unique<dio::vol::three_space::rmf_file_archive>());
  archive.add_archive_type(".map", std::make_unique<dio::vol::three_space::rmf_file_archive>());
  archive.add_archive_type(".vga", std::make_unique<dio::vol::three_space::rmf_file_archive>());
  archive.add_archive_type(".zip", std::make_unique<dio::zip::zip_file_archive>());
  archive.add_archive_type(".vl2", std::make_unique<dio::zip::zip_file_archive>());
  archive.add_archive_type(".pk3", std::make_unique<dio::zip::zip_file_archive>());
  archive.add_archive_type(".docx", std::make_unique<dio::zip::zip_file_archive>());
  archive.add_archive_type(".pptx", std::make_unique<dio::zip::zip_file_archive>());
  archive.add_archive_type(".xlxs", std::make_unique<dio::zip::zip_file_archive>());

  archive.add_archive_type(".dyn", std::make_unique<dio::vol::three_space::dyn_file_archive>());
  archive.add_archive_type(".vol", std::make_unique<dio::vol::three_space::vol_file_archive>());
  archive.add_archive_type(".vol", std::make_unique<dio::vol::darkstar::vol_file_archive>());

  archive.add_archive_type(".cln", std::make_unique<dio::cln::cln_file_archive>());
  archive.add_archive_type(".atd", std::make_unique<dio::atd::atd_file_archive>());

  return archive;
}

using input_arg = std::variant<std::string_view, fs::path, cpr::Url>;

struct parsed_args
{
  std::optional<fs::path> app_path;
  std::variant<std::monostate, fs::path, cpr::Url> src_path;
  std::optional<fs::path> dst_path;
  std::string_view game_name;
};

// argument examples

// <gameFolderPath>
// <gameFolderPath> <destination>
// <game>
// <gameFolderPath> <game>
// <gameFolderPath> <game> <destination>
// <gameFolderPath> <destination> <game>

parsed_args parse_args(int argc, char** argv)
{
  std::deque<input_arg> args;
  std::transform(argv, argv + argc, std::back_inserter(args), [](char* arg) -> input_arg {
    if (!arg)
    {
      return std::string_view{};
    }

    if (fs::exists(arg) || fs::exists(fs::path(arg).root_path()))
    {
      return fs::path(arg);
    }

    std::string_view result = arg;

    auto http_index = result.find("http://");

    if (http_index != std::string::npos)
    {
      return cpr::Url{arg};
    }

    http_index = result.find("https://");

    if (http_index != std::string::npos)
    {
      return cpr::Url{arg};
    }

    return result;
  });

  parsed_args result{};
  auto app_path = args.front();
  args.pop_front();

  if (std::holds_alternative<fs::path>(app_path))
  {
    result.app_path = std::get<fs::path>(app_path);
  }

  auto game_name = std::find_if(args.begin(), args.end(), std::holds_alternative<std::string_view, std::string_view, fs::path, cpr::Url>);

  if (game_name != args.end() && is_supported_game(std::get<std::string_view>(*game_name)))
  {
    result.game_name = std::get<std::string_view>(*game_name);
    args.erase(game_name);
  }

  auto src_url = std::find_if(args.begin(), args.end(), std::holds_alternative<cpr::Url, std::string_view, fs::path, cpr::Url>);

  bool dest_is_default = false;
  if (src_url != args.end())
  {
    result.src_path = std::get<cpr::Url>(*src_url);
    args.erase(src_url);
    dest_is_default = true;
  }

  auto src_or_dst_path = std::find_if(args.begin(), args.end(), std::holds_alternative<fs::path, std::string_view, fs::path, cpr::Url>);

  if (src_or_dst_path != args.end() && dest_is_default)
  {
    result.dst_path = std::get<fs::path>(*src_or_dst_path);
  }
  else if (src_or_dst_path != args.end() && args.size() >= 2)
  {
    result.src_path = std::get<fs::path>(*src_or_dst_path);
    std::advance(src_or_dst_path, 1);

    if (src_or_dst_path != args.end())
    {
      result.dst_path = std::get<fs::path>(*src_or_dst_path);
    }
  }
  else if (src_or_dst_path != args.end() && args.size() == 1)
  {
    auto temp = std::get<fs::path>(*src_or_dst_path);

    if (fs::exists(temp) && fs::is_directory(temp))
    {
      auto range = fs::directory_iterator(temp);
      auto count = std::distance(fs::begin(range), fs::end(range));

      if (count == 0)
      {
        result.dst_path = temp;
      }
      else
      {
        result.src_path = temp;
      }
    }
    else if (fs::exists(temp) && !fs::is_directory(temp))
    {
      result.src_path = temp;
    }
    else if (!fs::exists(temp))
    {
      result.dst_path = temp;
    }
  }

  return result;
}

// TODO list:
// * Add support for extracting contexts from archive files
// * Add support for downloading archive files
// * Add support for creating registry keys
// * Add support for Starsiege
// * Fix CI builds

int main(int argc, char** argv)
{
  auto args = parse_args(argc, argv);

  std::vector<fs::path> search_paths;
  search_paths.reserve(32);

  if (std::holds_alternative<fs::path>(args.src_path))
  {
    search_paths.emplace_back(std::get<fs::path>(args.src_path));
  }

  search_paths.emplace_back(fs::current_path());

  auto app_path = args.app_path.value_or(fs::current_path() / "installer");

  if (fs::current_path() != app_path.parent_path())
  {
    search_paths.emplace_back(app_path.parent_path());
  }

  std::vector<std::string_view> supported_games;

  if (!args.game_name.empty())
  {
    supported_games.emplace_back(args.game_name);
  }
  else
  {
    supported_games = get_supported_games();
  }

  for (const auto& game : supported_games)
  {
    auto maybe_info = get_info_for_game(game);

    if (!maybe_info.has_value())
    {
      continue;
    }
    auto& info = maybe_info.value();

    std::unordered_map<std::string, std::string> variable_values;

    std::transform(info.variables.begin(), info.variables.end(), std::inserter(variable_values, variable_values.begin()), [](auto& item){
      return std::make_pair("<" + std::string(item.first) + ">", std::string(item.second.front()));
    });

    if (auto path_with_root = args.dst_path.value_or( fs::current_path()); path_with_root.has_root_path())
    {
      variable_values.emplace("<systemDrive>", path_with_root.root_path().string());
    }
    else
    {
      variable_values.emplace("<systemDrive>", "C:\\");
    }

    auto replace_variables = [&](const fs::path& input) -> fs::path {
      fs::path result;
      for (auto& item : input)
      {
        auto variable = variable_values.find(item.string());

        if (variable != variable_values.end())
        {
          result /= variable->second;
        }
        else
        {
          result /= item;
        }
      }

      return result;
    };

    fs::path destination_path = replace_variables(args.dst_path.value_or(info.storage_properties.default_install_path));

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

    using found_file_info = std::pair<std::variant<studio::resources::folder_info, studio::resources::file_info>, std::string>;

    std::unordered_map<std::string, found_file_info> found_files;

    auto explorer = create_resource_explorer();

    std::unordered_map<std::string,
      std::vector<std::variant<studio::resources::folder_info, studio::resources::file_info>>> path_listings;

    for (const auto& path : search_paths)
    {
      path_listings.emplace((path / "").string(), explorer.get_content_listing(path));

      for (const auto& [source_file, destination_rule] : expected_files)
      {
        std::vector<fs::path> path_parts;
        path_parts.reserve(4);

        auto source_file_path = fs::path(source_file);

        std::copy(source_file_path.begin(), source_file_path.end(), std::back_inserter(path_parts));
        path_parts.pop_back();

        for (const auto& part : path_parts)
        {
          auto new_path = (path / part).string();
          auto existing_cache = path_listings.find(new_path);

          if (existing_cache == path_listings.end())
          {
            path_listings.emplace(std::move(new_path), explorer.get_content_listing(path / part));
          }
        }

        auto listing = path_listings.find((path / source_file_path.parent_path()).string());

        if (listing != path_listings.end())
        {
          auto file_info = listing->second.end();
          file_info = std::find_if(listing->second.begin(), listing->second.end(), [&](const auto& entry) {
            return std::visit(overloaded {
                                [&](const studio::resources::folder_info& arg) {
                                  return arg.name == source_file_path.filename();
                                },
                                [&](const studio::resources::file_info& arg) {
                                  return arg.filename == source_file_path.filename();
                                }
                              }, entry);
          });

          if (file_info != listing->second.end())
          {
            auto path_of_interest = std::visit(overloaded {
                                                 [&](const studio::resources::folder_info& arg) {
                                                   return arg.full_path;
                                                 },
                                                 [&](const studio::resources::file_info& arg) {
                                                   return arg.folder_path / arg.filename;
                                                 }
                                               }, *file_info);

            if (destination_rule == "=")
            {
              found_files.emplace(path_of_interest.string(), std::make_pair(*file_info, source_file));
            }
            else
            {
              found_files.emplace(path_of_interest.string(), std::make_pair(*file_info, destination_rule));
            }
          }
        }
      }

      if (found_files.size() == expected_files.size())
      {
        content_path.emplace(path);
        break;
      }

      found_files.clear();
    }

    if (content_path.has_value())
    {
      std::cout << "Found " << game << " content at " << content_path.value() << '\n';

      for (const auto& [source_wildcard, destination_rule] : wildcard_files)
      {
        auto working_path = (content_path.value() / fs::path(source_wildcard).parent_path()).make_preferred();
        auto working_value = fs::path(source_wildcard).filename();

        auto [iterator, added] = path_listings.emplace(working_path.string(), explorer.get_content_listing(working_path));

        for (const auto& file_info : iterator->second)
        {
          auto path_of_interest = std::visit(overloaded {
                                               [&](const studio::resources::folder_info& arg) {
                                                 return arg.full_path;
                                               },
                                               [&](const studio::resources::file_info& arg) {
                                                 return arg.folder_path / arg.filename;
                                               }
                                             }, file_info);

          if (working_value.stem() == "*" && path_of_interest.extension() == working_value.extension())
          {
            if (destination_rule == "=")
            {
              found_files.emplace(path_of_interest.string(), std::make_pair(file_info, path_of_interest.string().erase(0, working_path.string().size())));
            }
            else
            {
              found_files.emplace(path_of_interest.string(), std::make_pair(file_info, destination_rule));
            }
          }
        }

        std::cout << "Wildcard file at " << working_path << " " << working_value << '\n';
      }

      fs::create_directories(destination_path);
      for (const auto& found_file : found_files)
      {
        const auto& src = found_file.first;
        const auto& dst = found_file.second.second;
        const auto& file_info = found_file.second.first;

        try
        {
          auto new_path = (destination_path / dst).make_preferred();
          fs::create_directories(new_path.parent_path());

          auto process_file_info = [&](const studio::resources::file_info& arg, const auto& new_path) {
            if (fs::exists(new_path) && fs::file_size(new_path) == 0)
            {
              std::cout << "Removing empty file " << new_path << '\n';
              fs::remove(new_path);
            }

            if (fs::exists(arg.folder_path / arg.filename))
            {
              std::cout << "Copying from " << arg.folder_path / arg.filename << " to " << new_path << '\n';
              fs::copy(arg.folder_path / arg.filename, new_path, fs::copy_options::recursive | fs::copy_options::skip_existing);
            }
            else
            {
              std::cout << "Extracting " << arg.folder_path / arg.filename << " to " << new_path << '\n';
              std::ifstream archive{ explorer.get_archive_path(arg.folder_path), std::ios::binary };
              explorer.extract_file_contents(archive, new_path, arg);
            }
          };

          std::function<void(const studio::resources::folder_info&)> process_folder_info = [&](const studio::resources::folder_info& arg) {
            if (!fs::is_directory(arg.full_path) && fs::exists(arg.full_path))
            {
              fs::copy(arg.full_path, new_path, fs::copy_options::recursive | fs::copy_options::skip_existing);
              return;
            }

            auto child_files = path_listings.find(arg.full_path.string());

            if (child_files == path_listings.end())
            {
              child_files = path_listings.emplace(arg.full_path.string(), explorer.get_content_listing(arg.full_path.string())).first;
            }

            fs::create_directories(new_path);

            for (const auto& child_info : child_files->second)
            {
              std::visit(overloaded {
                           [&](const studio::resources::folder_info& arg) {
                             process_folder_info(arg);
                           },
                           [&](const studio::resources::file_info& arg) {
                             process_file_info(arg, new_path / arg.filename);
                           },
                         }, child_info);
            }
          };

          std::visit(overloaded {
                       [&](const studio::resources::folder_info& arg) {
                         process_folder_info(arg);
                       },
                       [&](const studio::resources::file_info& arg) {
                         process_file_info(arg, new_path);
                       }
                     }, file_info);


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
        auto dst_path = (destination_path / dst).make_preferred();

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
      return 0;
    }
    else
    {
      std::cout << "Did not find " << game << " content at \n";
    }
  }
}