#include <optional>
#include <string_view>
#include <algorithm>
#include <siege/extension/shared.hpp>
#include "id-tech-shared.hpp"

using siege::platform::input_mapping_ex;
using siege::platform::controller_input_type;
namespace stl = std::ranges;
using namespace std::literals;

extern "C" {
const wchar_t** format_command_line(const siege::platform::game_command_line_args* args, std::uint32_t* new_size) noexcept
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


  auto map_iter = std::find_if(args->string_settings.begin(), args->string_settings.end(), [](auto& setting) { return setting.name && setting.name == std::wstring_view{ L"map" } && setting.value && setting.value[0]; });
  auto listen_iter = std::find_if(args->flags.begin(), args->flags.end(), [](auto& setting) { return setting && setting == std::wstring_view{ L"listen" }; });
  auto serve_iter = std::find_if(args->flags.begin(), args->flags.end(), [](auto& setting) { return setting && setting == std::wstring_view{ L"serve" }; });

  if (map_iter != args->string_settings.end())
  {
    if (listen_iter != args->flags.end())
    {
      string_args.emplace_back(L"+protocol");
      string_args.emplace_back(L"4");// tcp/ip
      string_args.emplace_back(L"+listen");
      string_args.emplace_back(map_iter->value);
    }
    else if (serve_iter != args->flags.end())
    {
      string_args.emplace_back(L"+protocol");
      string_args.emplace_back(L"4");// tcp/ip
      string_args.emplace_back(L"+serve");
      string_args.emplace_back(map_iter->value);
    }
    else
    {
      string_args.emplace_back(L"+map");
      string_args.emplace_back(map_iter->value);
    }
  }

  auto join_iter = std::find_if(args->flags.begin(), args->flags.end(), [](auto& setting) { return setting && setting == std::wstring_view{ L"join" }; });

  if (join_iter != args->flags.end())
  {
    string_args.emplace_back(L"+protocol");
    string_args.emplace_back(L"4");// tcp/ip
    string_args.emplace_back(L"+join");
  }

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

    if (std::wstring_view(setting.name) == L"exec")
    {
      string_args.emplace_back(L"+exec");
      string_args.emplace_back(setting.value);
    }
    else if (std::wstring_view(setting.name) != L"map")
    {
      string_args.emplace_back(std::wstring(L"+") + setting.name);
      string_args.emplace_back(setting.value);
    }
    else
    {
      continue;
    }
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

std::optional<std::string_view> mouse_key_for_vkey(const input_mapping_ex& mapping) noexcept
{
  if (mapping.vkey == VK_LBUTTON)
  {
    return "mouse1"sv;
  }

  if (mapping.vkey == VK_RBUTTON)
  {
    return "mouse2"sv;
  }

  if (mapping.vkey == VK_MBUTTON)
  {
    return "mouse3"sv;
  }

  return std::nullopt;
}

std::optional<input_mapping_ex> vkey_for_mouse_key(const std::string_view& mapping) noexcept
{
  input_mapping_ex result{};
  result.vkey = VK_LBUTTON;

  if (mouse_key_for_vkey(result) == mapping)
  {
    return result;
  }

  result.vkey = VK_RBUTTON;

  if (mouse_key_for_vkey(result) == mapping)
  {
    return result;
  }
  result.vkey = VK_MBUTTON;

  if (mouse_key_for_vkey(result) == mapping)
  {
    return result;
  }
  return std::nullopt;
}

std::optional<std::string_view> joy_key_for_vkey(const input_mapping_ex& mapping) noexcept
{
  constexpr static std::array<std::string_view, 15> buttons{ {
    "joy1"sv,
    "joy2"sv,
    "joy3"sv,
    "joy4"sv,
    "joy5"sv,
    "joy6"sv,
    "joy7"sv,
    "joy8"sv,
    "joy9"sv,
    "joy10"sv,
    "joy11"sv,
    "joy12"sv,
    "joy13"sv,
    "joy14"sv,
    "joy15"sv,
  } };

  if (siege::platform::is_for_controller(mapping.context) && mapping.hardware_input_type == controller_input_type::button && mapping.hardware_index < buttons.size())
  {
    return buttons[mapping.hardware_index];
  }

  return std::nullopt;
}

struct mapping
{
  WORD vkey;
  std::string_view name;
};

constexpr auto& get_ascii_keys()
{
  constexpr static auto uppercase_keys = [] {
    std::array<mapping, 36> results{};
    auto upper_keys = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"sv;

    for (auto i = 0; i < upper_keys.size(); ++i)
    {
      results[i].vkey = (WORD)upper_keys[i];
      results[i].name = upper_keys.substr(i, 1);
    }
    return results;
  }();

  static_assert(uppercase_keys[0].vkey == 0x41);
  static_assert(uppercase_keys[0].name == "A"sv);

  static_assert(uppercase_keys[uppercase_keys.size() - 1].vkey == 0x39);
  static_assert(uppercase_keys[uppercase_keys.size() - 1].name == "9"sv);
  return uppercase_keys;
}

constexpr auto& get_special_keys()
{
  constexpr static auto special_keys = std::array<mapping, 46>{ {
    { .vkey = VK_UP, .name = "UPARROW"sv },
    { VK_DOWN, "DOWNARROW"sv },
    { VK_LEFT, "LEFTARROW"sv },
    { VK_RIGHT, "UPARROW"sv },
    { VK_LCONTROL, "CTRL"sv },
    { VK_RETURN, "ENTER"sv },
    { VK_LSHIFT, "SHIFT"sv },
    { VK_LMENU, "ALT"sv },
    { VK_HOME, "HOME"sv },
    { VK_PRIOR, "PGUP"sv },
    { VK_ESCAPE, "ESCAPE"sv },
    { VK_NEXT, "PGDN"sv },
    { VK_END, "END"sv },
    { VK_DELETE, "DEL"sv },
    { VK_INSERT, "INS"sv },
    { VK_PRINT, "PRINTSCREEN"sv },
    { VK_CAPITAL, "CAPS"sv },
    { VK_PAUSE, "PAUSE"sv },
    { VK_TAB, "TAB"sv },
    { VK_SPACE, "SPACE"sv },
    { VK_OEM_1, "SEMICOLON"sv },
    { VK_OEM_7, "QUOTE"sv },
    { VK_SCROLL, "SCROLL"sv },
    { VK_F1, "F1"sv },
    { VK_F2, "F2"sv },
    { VK_F3, "F3"sv },
    { VK_F4, "F4"sv },
    { VK_F5, "F5"sv },
    { VK_F6, "F6"sv },
    { VK_F7, "F7"sv },
    { VK_F8, "F8"sv },
    { VK_F9, "F9"sv },
    { VK_F10, "F10"sv },
    { VK_F11, "F11"sv },
    { VK_F12, "F12"sv },
    { VK_OEM_COMMA, ","sv },
    { VK_OEM_PERIOD, "."sv },
    { VK_OEM_MINUS, "-"sv },
    { VK_OEM_PLUS, "="sv },
    { VK_OEM_1, ";"sv },
    { VK_OEM_2, "/"sv },
    { VK_OEM_3, "`"sv },
    { VK_OEM_4, "["sv },
    { VK_OEM_5, "\\"sv },
    { VK_OEM_6, "]"sv },
    { VK_OEM_7, "'"sv },
  } };
  return special_keys;
}

std::optional<std::string_view> bind_key_for_vkey(const input_mapping_ex& mapping) noexcept
{
  auto is_valid_key = [&](auto& key) {
    return key.vkey == mapping.vkey;
  };
  auto iter = stl::find_if(get_special_keys(), is_valid_key);

  if (iter != get_special_keys().end())
  {
    return iter->name;
  }

  auto letter_iter = stl::find_if(get_ascii_keys(), is_valid_key);

  if (letter_iter != get_ascii_keys().end())
  {
    return letter_iter->name;
  }

  return std::nullopt;
}

std::optional<input_mapping_ex> vkey_for_bind_key(const std::string_view& mapping) noexcept
{
  auto is_valid_key = [&](auto& key) {
    return key.name == mapping;
  };
  auto is_valid_key_lower = [&](auto& key) {
    return key.name == siege::platform::to_upper(mapping);
  };

  auto iter = stl::find_if(get_special_keys(), is_valid_key);

  if (iter == get_special_keys().end())
  {
    iter = stl::find_if(get_special_keys(), is_valid_key_lower);
  }

  input_mapping_ex result{
    .context = hardware_context::keyboard
  };

  if (iter != get_special_keys().end())
  {
    result.vkey = iter->vkey;
    return result;
  }

  auto letter_iter = stl::find_if(get_ascii_keys(), is_valid_key);

  if (letter_iter == get_ascii_keys().end())
  {
    letter_iter = stl::find_if(get_ascii_keys(), is_valid_key_lower);
  }

  if (letter_iter != get_ascii_keys().end())
  {
    result.vkey = letter_iter->vkey;
    return result;
  }

  return std::nullopt;
}

std::optional<std::string_view> pov_key_for_vkey(const input_mapping_ex& mapping) noexcept
{
  if (mapping.hardware_input_type == controller_input_type::hat && mapping.hardware_index == 0)
  {
    if (siege::platform::is_up_direction(mapping.vkey))
    {
      return "hat1";
    }

    if (siege::platform::is_down_direction(mapping.vkey))
    {
      return "hat2";
    }

    if (siege::platform::is_left_direction(mapping.vkey))
    {
      return "hat3";
    }

    if (siege::platform::is_right_direction(mapping.vkey))
    {
      return "hat4";
    }
  }

  return std::nullopt;
}