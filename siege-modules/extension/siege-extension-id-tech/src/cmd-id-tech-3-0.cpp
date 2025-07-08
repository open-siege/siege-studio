#include <siege/platform/win/window_module.hpp>
#include <siege/resource/zip_resource.hpp>
#include <detours.h>
#include <siege/extension/shared.hpp>


extern "C" {
using game_command_line_caps = siege::platform::game_command_line_caps;
namespace fs = std::filesystem;
using namespace std::literals;
extern game_command_line_caps command_line_caps;

// TODO
// Specify controller cfg in the command line
const wchar_t** format_command_line(const siege::platform::game_command_line_args* args, std::uint32_t* new_size)
{
  if (!args)
  {
    return nullptr;
  }

  if (!new_size)
  {
    return nullptr;
  }

  static std::vector<std::wstring> string_args;
  string_args.clear();

  for (auto& setting : args->string_settings)
  {
    if (!setting.name)
    {
      continue;
    }

    if (!setting.value)
    {
      continue;
    }

    if (!setting.value[0])
    {
      continue;
    }

    if (std::wstring_view(setting.name) == command_line_caps.ip_connect_setting)
    {
      string_args.emplace_back(L"+connect");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) == L"map")
    {
      string_args.emplace_back(L"+map");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) == L"exec")
    {
      string_args.emplace_back(L"+exec");
      string_args.emplace_back(setting.value);
    }
    else
    {
      string_args.emplace_back(L"+set");
      string_args.emplace_back(setting.name);
      string_args.emplace_back(setting.value);
    }
  }

  for (auto& flag : args->flags)
  {
    if (!flag)
    {
      break;
    }

    string_args.emplace_back(flag);
  }

  if (string_args.empty())
  {
    return nullptr;
  }

  static std::vector<const wchar_t*> raw_args;
  raw_args.resize(string_args.size());
  *new_size = (std::uint32_t)string_args.size();

  std::transform(string_args.begin(), string_args.end(), raw_args.begin(), [](const std::wstring& value) {
    return value.c_str();
  });

  return raw_args.data();
}
}