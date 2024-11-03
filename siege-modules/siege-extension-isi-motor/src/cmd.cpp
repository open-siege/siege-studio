#include <siege/platform/win/desktop/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {

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
  static std::vector<const wchar_t*> raw_args;
  string_args.clear();

  std::string ip_address = args->connect_ip_address.ip_address.data();

  if (!ip_address.empty())
  {
    string_args.emplace_back(L"+connect");

    std::wstring temp;
    temp.reserve(ip_address.size() + 8);
    temp.resize(ip_address.size());

    std::transform(ip_address.begin(), ip_address.end(), temp.begin(), [](char t) {
      return (wchar_t)t;
    });

    temp.append(1, ':');

    if (args->connect_ip_address.port_number)
    {
      temp.append(std::to_wstring(args->connect_ip_address.port_number));
    }
    else
    {
      temp.append(L"0");
    }

    string_args.emplace_back(std::move(temp));
  }

  if (string_args.empty())
  {
    return nullptr;
  }

  raw_args.resize(string_args.size());
  *new_size = (std::uint32_t)string_args.size();

  std::transform(string_args.begin(), string_args.end(), raw_args.begin(), [](const std::wstring& value) {
    return value.c_str();
  });

  return raw_args.data();
}
}