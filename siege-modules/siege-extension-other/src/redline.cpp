#include <siege/platform/extension_module.hpp>
#include "shared.hpp"


extern "C" {
using namespace std::literals;

using game_command_line_caps = siege::platform::game_command_line_caps;

extern auto command_line_caps = game_command_line_caps{
  .ip_connect_setting = L"connect",
  .player_name_setting = L"name",
  .string_settings = { { L"connect", L"name" } }
};

constexpr static std::array<std::string_view, 13> verification_strings = { {
  "connect"sv,
  "name"sv,
  "team"sv,
  "skin"sv,
  "game"sv,
  "host"sv,
  "maxplayers"sv,
  "hostname"sv,
  "console"sv,
  "lobby"sv,
  "redline.cfg"sv,
  "RED6"sv,
  "Redline"sv,
} };


HRESULT executable_is_supported(const wchar_t* filename) noexcept
{
  return siege::executable_is_supported(filename, verification_strings);
}

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

    string_args.emplace_back(std::wstring(L"+") + setting.name);
    string_args.emplace_back(setting.value);
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