#include <siege/platform/win/desktop/window_module.hpp>
#include <detours.h>
#include "shared.hpp"


extern "C" {

using game_command_line_caps = siege::platform::game_command_line_caps;
extern game_command_line_caps command_line_caps;
using namespace std::literals;

const wchar_t** format_command_line(const siege::platform::game_command_line_args* args, std::uint32_t* new_size)
{
  OutputDebugStringW(L"format_command_line");

  if (!args)
  {
    OutputDebugStringW(L"!args true");
    return nullptr;
  }

  if (!new_size)
  {
    OutputDebugStringW(L"!new_size");
    return nullptr;
  }

  static std::vector<std::wstring> string_args;
  string_args.clear();

  for (auto& setting : args->string_settings)
  {
    if (!setting.name)
    {
      OutputDebugStringW(L"!setting.name");
      continue;
    }

    if (!setting.value)
    {
      OutputDebugStringW(L"!setting.value");
      continue;
    }

    if (!setting.value[0])
    {
      OutputDebugStringW(L"!setting.value[0]");
      continue;
    }

    if (std::wstring_view(setting.name) == command_line_caps.ip_connect_setting)
    {
      if (setting.value == L"0.0.0.0"sv)
      {
        continue;
      }

      string_args.emplace_back(L"+connect");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) == L"map")
    {
      string_args.emplace_back(L"+map");
      string_args.emplace_back(setting.value);
    }
    else
    {
      string_args.emplace_back(L"+set");
      string_args.emplace_back(setting.name);
      string_args.emplace_back(setting.value);
    }
  }

  for (auto& value : string_args)
  {
    OutputDebugStringW(value.c_str());
    OutputDebugStringW(L"\n");
  }

  if (string_args.empty())
  {
    OutputDebugStringW(L"No string args found");
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