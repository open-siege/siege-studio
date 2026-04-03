#include <siege/configuration/shared.hpp>
#include <algorithm>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::configuration
{
  namespace endian = siege::platform;

  text_game_config::text_game_config(std::any&& context, text_game_config::persist& save_config)
    : save_config(save_config)
  {
  }

  text_game_config::text_game_config(text_game_config::persist& save_config)
    : text_game_config(std::any{}, save_config)
  {
  }

  text_game_config::text_game_config(std::any&& context, std::unique_ptr<char[]>&& raw, std::vector<config_line>&& entries, persist& save_config)
    : save_config(save_config), context(std::move(context)), raw_data(std::move(raw)), line_entries(std::move(entries))
  {
  }

  std::vector<key_type> text_game_config::keys() const
  {
    std::vector<key_type> results;
    results.reserve(line_entries.size());

    for (auto& entry : line_entries)
    {
      if (entry.key_segments.empty())
      {
        continue;
      }
      results.emplace_back(entry.key_segments);
    }

    return results;
  }

  key_type text_game_config::find(key_type key) const
  {
    auto iter = std::find_if(line_entries.rbegin(), line_entries.rend(), [&](auto& entry) { return entry.key_segments == key; });

    if (iter == line_entries.rend())
    {
      return key_type{ std::string_view("") };
    }

    return iter->value;
  }

  bool text_game_config::contains(key_type key) const
  {
    auto iter = std::find_if(line_entries.rbegin(), line_entries.rend(), [&](auto& entry) { return entry.key_segments == key; });

    if (iter == line_entries.rend())
    {
      return false;
    }

    return true;
  }

  text_game_config&& text_game_config::emplace(key_type key, key_type value)
  {
    auto iter = std::find_if(line_entries.rbegin(), line_entries.rend(), [&](auto& entry) { return entry.key_segments == key; });

    if (iter != line_entries.rend())
    {
      iter->value = value;
    }
    else
    {
      line_entries.emplace_back(config_line{ "", key, value });
    }

    return std::move(*this);
  }

  text_game_config&& text_game_config::remove(key_type key)
  {
    auto iter = std::remove_if(line_entries.begin(), line_entries.end(), [&](auto& entry) { return entry.key_segments == key; });

    line_entries.erase(iter, line_entries.end());

    return std::move(*this);
  }

  void text_game_config::save(std::ostream& stream) const
  {
    save_config(context, line_entries, stream);
  }

  bool is_ascii_text_config(std::istream& raw_data)
  {
    auto current_pos = raw_data.tellg();
    int count = 0;
    bool result = std::all_of(std::istreambuf_iterator<char>{ raw_data }, std::istreambuf_iterator<char>{}, [&](char raw) {
      auto value = static_cast<unsigned char>(raw);
      return count++ != 8192 && std::isalpha(value) || std::isdigit(value) || std::isspace(value) || std::ispunct(value);
    });
    raw_data.seekg(current_pos, std::ios::beg);
    return result;
  }

}// namespace siege::configuration