#include <SDKDDKVer.h>
#include <filesystem>
#include <utility>
#include <span>
#include <optional>
#include <string_view>
#include <siege/platform/win/module.hpp>

namespace fs = std::filesystem;

struct command_line_args
{
  fs::path app_path;
  std::optional<fs::path> vol_path;
};

command_line_args parse_command_line(std::span<std::string_view> args)
{
  auto app_arg = fs::path(win32::module_ref().GetModuleFileName<wchar_t>());
  std::optional<fs::path> file_arg;

  if (!args.empty())
  {
    auto temp = fs::path(args[0]);

    auto next_arg = 0;

    if (temp.has_extension() && (temp.extension() == ".exe" || temp.extension() == ".EXE"))
    {
      app_arg = temp;
      next_arg = 1;
    }

    // in the very rare case you launch a process with the first argument not being the exe path
    // next_arg will be 0
    if (next_arg + 1 <= args.size() && fs::is_regular_file(args[next_arg]))
    {
      file_arg = args[next_arg];
    }
  }
  return { app_arg, file_arg };
}
