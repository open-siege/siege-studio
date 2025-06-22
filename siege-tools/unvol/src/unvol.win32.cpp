#include <SDKDDKVer.h>
#include <filesystem>
#include <utility>
#include <span>
#include <optional>
#include <string_view>
#include <ranges>
#include <spanstream>
#include <siege/platform/win/module.hpp>

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
  auto app_arg = fs::path(win32::module_ref().GetModuleFileName<wchar_t>());
  std::optional<fs::path> file_arg;
  std::optional<fs::path> output_path;

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
  return { app_arg, file_arg, output_path };
}

bool has_embedded_file()
{
  return ::FindResourceW(nullptr, L"embedded", RT_RCDATA) != nullptr;
}

std::unique_ptr<std::istream> create_stream_for_embedded_file()
{
  auto resource = ::FindResourceW(nullptr, L"embedded", RT_RCDATA);

  if (!resource)
  {
    return nullptr;
  }

  auto handle = ::LoadResource(nullptr, resource);

  if (!handle)
  {
    return nullptr;
  }

  auto ptr = ::LockResource(handle);
  auto size = ::SizeofResource(nullptr, resource);

  auto span = std::span<char>((char*)ptr, size);

  return std::unique_ptr<std::istream>(new std::ispanstream(span));
}

bool has_embedded_output_path()
{
  return ::FindResourceW(nullptr, L"output_path", RT_RCDATA) != nullptr;
}

fs::path get_embedded_output_path()
{
  auto resource = ::FindResourceW(nullptr, L"output_path", RT_RCDATA);

  if (!resource)
  {
    return fs::path{};
  }

  auto handle = ::LoadResource(nullptr, resource);

  if (!handle)
  {
    return fs::path{};
  }

  auto ptr = ::LockResource(handle);
  auto size = ::SizeofResource(nullptr, resource);

  return std::string_view((char*)ptr, size);
}

bool has_embedded_post_extract_commands()
{
  return ::FindResourceW(nullptr, L"post_extract", RT_RCDATA) != nullptr;
}

std::vector<std::string> get_embedded_post_extract_commands()
{
  auto resource = ::FindResourceW(nullptr, L"post_extract", RT_RCDATA);

  if (!resource)
  {
    return {};
  }

  auto handle = ::LoadResource(nullptr, resource);

  if (!handle)
  {
    return {};
  }

  auto ptr = ::LockResource(handle);
  auto size = ::SizeofResource(nullptr, resource);

  std::string_view temp((char*)ptr, size);

  std::vector<std::string> results;

  std::size_t offset = 0;

  do
  {
    auto next = temp.find("\r\n");

    if (auto result = temp.substr(0, next); !result.empty())
    {
      results.emplace_back(result);
    }

    temp = next == std::string_view::npos ? std::string_view() : temp.substr(next + 2);
  } while (!temp.empty());

  return results;
}