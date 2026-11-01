module;

export module siege.extension.vampire;

import std;

import siege.configuration.id_tech;
import siege.configuration.shared;
import siege.extension.id_tech;
import siege.extension.shared;
import siege.platform;
import siege.platform.shared;

using vkey = siege::platform::vkey;

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

export std::optional<std::string_view> mouse_key_for_vkey(const input_mapping_ex& mapping) noexcept
{
  if (mapping.vkey == vkey::lbutton)
  {
    return "mouse1"sv;
  }

  if (mapping.vkey == vkey::rbutton)
  {
    return "mouse2"sv;
  }

  if (mapping.vkey == vkey::mbutton)
  {
    return "mouse3"sv;
  }

  return std::nullopt;
}

export std::optional<input_mapping_ex> vkey_for_mouse_key(const std::string_view& mapping) noexcept
{
  input_mapping_ex result{};
  result.vkey = vkey::lbutton;

  if (mouse_key_for_vkey(result) == mapping)
  {
    return result;
  }

  result.vkey = vkey::rbutton;

  if (mouse_key_for_vkey(result) == mapping)
  {
    return result;
  }
  result.vkey = vkey::mbutton;

  if (mouse_key_for_vkey(result) == mapping)
  {
    return result;
  }
  return std::nullopt;
}

export std::optional<std::string_view> joy_key_for_vkey(const input_mapping_ex& mapping) noexcept
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
  vkey vkey;
  std::string_view name;
};

constexpr auto& get_ascii_keys()
{
  constexpr static auto uppercase_keys = [] {
    std::array<mapping, 36> results{};
    auto upper_keys = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"sv;

    for (auto i = 0; i < upper_keys.size(); ++i)
    {
      results[i].vkey = static_cast<vkey>(upper_keys[i]);
      results[i].name = upper_keys.substr(i, 1);
    }
    return results;
  }();

  static_assert(std::to_underlying(uppercase_keys[0].vkey) == 0x41);
  static_assert(uppercase_keys[0].name == "A"sv);

  static_assert(std::to_underlying(uppercase_keys[uppercase_keys.size() - 1].vkey) == 0x39);
  static_assert(uppercase_keys[uppercase_keys.size() - 1].name == "9"sv);
  return uppercase_keys;
}

constexpr auto& get_special_keys()
{
  constexpr static auto special_keys = std::array<mapping, 46>{ {
    { .vkey = vkey::up, .name = "UPARROW"sv },
    { vkey::down, "DOWNARROW"sv },
    { vkey::left, "LEFTARROW"sv },
    { vkey::right, "UPARROW"sv },
    { vkey::control, "CTRL"sv },
    { vkey::enter, "ENTER"sv },
    { vkey::shift, "SHIFT"sv },
    { vkey::alt, "ALT"sv },
    { vkey::home, "HOME"sv },
    { vkey::page_up, "PGUP"sv },
    { vkey::escape, "ESCAPE"sv },
    { vkey::page_down, "PGDN"sv },
    { vkey::end, "END"sv },
    { vkey::del, "DEL"sv },
    { vkey::insert, "INS"sv },
    { vkey::print, "PRINTSCREEN"sv },
    { vkey::caps_lock, "CAPS"sv },
    { vkey::pause, "PAUSE"sv },
    { vkey::tab, "TAB"sv },
    { vkey::space, "SPACE"sv },
    { vkey::oem_1, "SEMICOLON"sv },
    { vkey::oem_7, "QUOTE"sv },
    { vkey::scroll_lock, "SCROLL"sv },
    { vkey::f1, "F1"sv },
    { vkey::f2, "F2"sv },
    { vkey::f3, "F3"sv },
    { vkey::f4, "F4"sv },
    { vkey::f5, "F5"sv },
    { vkey::f6, "F6"sv },
    { vkey::f7, "F7"sv },
    { vkey::f8, "F8"sv },
    { vkey::f9, "F9"sv },
    { vkey::f10, "F10"sv },
    { vkey::f11, "F11"sv },
    { vkey::f12, "F12"sv },
    { vkey::oem_comma, ","sv },
    { vkey::oem_period, "."sv },
    { vkey::oem_minus, "-"sv },
    { vkey::oem_plus, "="sv },
    { vkey::oem_1, ";"sv },
    { vkey::oem_2, "/"sv },
    { vkey::oem_3, "`"sv },
    { vkey::oem_4, "["sv },
    { vkey::oem_5, "\\"sv },
    { vkey::oem_6, "]"sv },
    { vkey::oem_7, "'"sv },
  } };
  return special_keys;
}

export std::optional<std::string_view> bind_key_for_vkey(const input_mapping_ex& mapping) noexcept
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

export std::optional<input_mapping_ex> vkey_for_bind_key(const std::string_view& mapping) noexcept
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

export std::optional<std::string_view> pov_key_for_vkey(const input_mapping_ex& mapping) noexcept
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
