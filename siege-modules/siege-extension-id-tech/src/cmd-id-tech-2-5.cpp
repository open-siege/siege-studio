#include <siege/platform/win/desktop/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {

using game_command_line_caps = siege::platform::game_command_line_caps;
extern game_command_line_caps command_line_caps;

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

  auto ip_address_iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [&](auto& arg) {
    return arg.value != nullptr && std::wstring_view(arg.name) == command_line_caps.ip_connect_setting;
  });

  if (ip_address_iter != args->string_settings.end())
  {
    string_args.emplace_back(L"+connect");
    string_args.emplace_back(ip_address_iter->value);
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