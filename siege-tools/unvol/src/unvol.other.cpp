#include <filesystem>
#include <utility>
#include <span>
#include <optional>
#include <string_view>

namespace fs = std::filesystem;

struct command_line_args
{
  fs::path app_path;
  std::optional<fs::path> vol_path;
};

command_line_args parse_command_line(std::span<std::string_view> args)
{
  fs::path app_path;
  std::optional<fs::path> vol_path;

  if (!args.empty())
  {
    app_path = args[0];
    
    if (args.size() >= 2)
    {
      vol_path = args[1];
    }
  }

  return { app_path, vol_path };
}