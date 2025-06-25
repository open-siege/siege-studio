#include <filesystem>
#include <utility>
#include <span>
#include <optional>
#include <string_view>
#include <ranges>

namespace fs = std::filesystem;
namespace stl = std::ranges;

struct command_line_args
{
  fs::path app_path;
  std::optional<fs::path> vol_path;
  std::optional<fs::path> output_path;
};

command_line_args parse_command_line(std::span<std::string_view> args)
{
  fs::path app_path;
  std::optional<fs::path> vol_path;
  std::optional<fs::path> output_path;

  if (!args.empty())
  {
    app_path = args[0];
    
    if (args.size() >= 2)
    {
      vol_path = args[1];
    }

    auto output = stl::find_if(args, [](auto& arg) {
      return arg.starts_with("--output") || arg.starts_with("-o");
    });

    if (output != args.end())
    {
      if (output->contains("="))
      {
        output_path = output->substr(output->find("=") + 1);
      }
      else
      {
        auto next = output;
        std::advance(next, 1);

        if (next != args.end())
        {
          output_path = *next;
        }
      }
    }
  }

  return { app_path, vol_path, output_path };
}

bool has_embedded_file()
{
  return false;
}

std::unique_ptr<std::istream> create_stream_for_embedded_file()
{
  return nullptr;
}

bool has_embedded_output_path()
{
  return false;
}

fs::path get_embedded_output_path()
{
  return {};
}

bool has_embedded_post_extract_commands()
{
  return false;
}

std::vector<std::string> get_embedded_post_extract_commands()
{
  return {};
}

int start_ui_modal(command_line_args args, std::function<void(command_line_args)> action)
{   
    return 0;
}