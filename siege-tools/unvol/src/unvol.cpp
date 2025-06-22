#include <fstream>
#include <array>
#include <memory>
#include <filesystem>
#include <functional>
#include <utility>
#include <iostream>
#include <span>
#include <siege/resource/resource_maker.hpp>

namespace fs = std::filesystem;

struct command_line_args
{
  fs::path app_path;
  std::optional<fs::path> vol_path;
  std::optional<fs::path> output_path;
};

command_line_args parse_command_line(std::span<std::string_view> args);

bool has_embedded_file();
std::unique_ptr<std::istream> create_stream_for_embedded_file();

bool has_embedded_output_path();
fs::path get_embedded_output_path();

bool has_embedded_post_extract_commands();
std::vector<std::string> get_embedded_post_extract_commands();

int main(int argc, const char** argv)
{
  std::vector<std::string_view> raw_args(argv, argv + argc);
  command_line_args args = parse_command_line(raw_args);

  if (!args.vol_path && !has_embedded_file())
  {
    std::cerr << "No file specified. Please specify a file to extract as the first argument." << '\n';
    return EXIT_FAILURE;
  }


  if (has_embedded_file() && !(has_embedded_output_path() || args.output_path.has_value()))
  {
    std::cerr << "No output directory specified. Please specify a path with --output then the path to the directory to extract to." << '\n';
    return EXIT_FAILURE;
  }

  std::unique_ptr<std::istream> volume_stream;

  if (has_embedded_file())
  {
    volume_stream = create_stream_for_embedded_file();
    args.vol_path = fs::path();

    if (!args.output_path)
    {
      args.output_path = get_embedded_output_path();
    }
  }
  else
  {
    volume_stream = std::unique_ptr<std::istream>(new std::ifstream{ *args.vol_path, std::ios::binary });
  }

  if (!siege::resource::is_resource_readable(*volume_stream))
  {
    std::cerr << "Could not extract " << *args.vol_path << '\n';
    return EXIT_FAILURE;
  }

  siege::platform::resource_reader reader = siege::resource::make_resource_reader(*volume_stream);

  std::any cache;
  auto files = reader.get_content_listing(cache, *volume_stream, { *args.vol_path, *args.vol_path });

  fs::path output_folder = [&] {
    if (args.output_path)
    {
      return *args.output_path;
    }

    auto temp = *args.vol_path;

    if (temp.has_extension())
    {
      temp.replace_extension("");
    }

    return temp;
  }();


  std::function<void(decltype(files)&)> extract_files = [&](const auto& files) {
    for (const auto& some_file : files)
    {
      std::visit([&](const auto& info) {
        using info_type = std::decay_t<decltype(info)>;

        if constexpr (std::is_same_v<info_type, siege::platform::file_info>)
        {
          auto final_folder = output_folder / info.relative_path();

          std::filesystem::create_directories(final_folder);
          auto filename = final_folder / info.filename;
          auto new_stream = std::ofstream{ filename, std::ios::binary };
          reader.extract_file_contents(cache, *volume_stream, info, new_stream);
        }

        if constexpr (std::is_same_v<info_type, siege::platform::folder_info>)
        {
          auto files = reader.get_content_listing(cache, *volume_stream, { *args.vol_path, info.full_path });
          extract_files(files);
        }
      },
        some_file);
    }
  };

  extract_files(files);

  if (args.output_path && has_embedded_post_extract_commands())
  {
    auto commands = get_embedded_post_extract_commands();
    fs::current_path(*args.output_path);

    for (auto& command : commands)
    {
      std::system(command.c_str());
    }
  }
}
