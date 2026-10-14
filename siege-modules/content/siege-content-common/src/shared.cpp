export module siege.configuration.shared;

import std;

import siege.platform.endian;
import siege.platform.shared;

namespace siege::configuration
{
  namespace endian = siege::platform;

  export struct key_type
  {
    key_type(std::string_view key) : data(std::array<std::string_view, 1>{ key }) {}

    key_type(std::vector<std::string_view> key) : data(std::move(key)) {}

    key_type(std::initializer_list<std::string_view> key)
    {
      if (key.size() == 1)
      {
        data = std::array<std::string_view, 1>{ *key.begin() };
      }
      else if (key.size() == 2)
      {
        data = std::array<std::string_view, 2>{ *key.begin(), *std::next(key.begin()) };
      }
      else
      {
        data = std::vector(key);
      }
    }

    bool operator==(std::string_view rhs) const
    {
      return std::visit([&](auto& a) {
        if (a.size() > 0)
        {
          return a[0] == rhs;
        }

        return false;
      },
        data);
    }

    bool empty() const
    {
      return std::visit([&](auto& a) -> bool {
        return a.empty();
      },
        data);
    }

    std::string_view at(std::size_t index) const
    {
      return std::visit([&](auto& a) -> std::string_view {
        if (a.empty())
        {
          return "";
        }

        if (index > a.size() - 1)
        {
          return "";
        }

        return a[index];
      },
        data);
    }

    bool operator==(const key_type& rhs) const
    {
      if (data.index() == rhs.data.index())
      {
        return data == rhs.data;
      }

      return std::visit([&](auto& a) {
        return std::visit([&](auto& b) {
          if (a.size() == b.size())
          {
            return std::equal(a.begin(), a.end(), b.begin());
          }

          return false;
        },
          rhs.data);
      },
        data);
    }

    bool operator!=(const key_type& rhs) const
    {
      return !(*this == rhs);
    }

    template<std::size_t ArraySize>
    std::array<std::string_view, ArraySize> to_array() const
    {
      if (std::holds_alternative<std::array<std::string_view, ArraySize>>(data))
      {
        return std::get<std::array<std::string_view, ArraySize>>(data);
      }

      return std::visit([](auto& value) {
        std::array<std::string_view, ArraySize> result;

        if (value.size() > ArraySize)
        {
          std::copy_n(value.begin(), ArraySize, result.begin());
        }
        else
        {
          std::copy_n(value.begin(), value.size(), result.begin());
        }

        return result;
      },
        data);
    }

    std::variant<std::array<std::string_view, 1>, std::array<std::string_view, 2>, std::vector<std::string_view>> data;
  };

  export class text_game_config
  {
  public:
    struct config_line
    {
      std::string_view raw_line;
      key_type key_segments;
      key_type value;
    };

    using persist = void(const std::any&, const std::vector<config_line>&, std::ostream&);

    text_game_config(std::any&& context, persist&);
    text_game_config(persist&);
    text_game_config(std::any&& context, std::unique_ptr<char[]>&&, std::vector<config_line>&&, persist&);

    std::vector<key_type> keys() const;
    bool contains(key_type key) const;
    key_type find(key_type key) const;
    std::vector<config_line> entries_for(key_type key) const;
    text_game_config&& emplace(key_type key, key_type value);
    text_game_config&& remove(key_type key);

    void save(std::ostream&) const;

  private:
    persist& save_config;
    std::any context;
    std::shared_ptr<const char[]> raw_data;
    std::vector<config_line> line_entries;
  };

  export bool is_ascii_text_config(std::istream&);

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

  std::vector<text_game_config::config_line> text_game_config::entries_for(key_type key) const
  {
    auto count = std::count_if(line_entries.rbegin(), line_entries.rend(), [&](auto& entry) { return entry.key_segments == key; });

    if (count == 0)
    {
      return {};
    }

    std::vector<text_game_config::config_line> results;
    results.reserve(count);

    for (const auto& line : line_entries)
    {
      if (line.key_segments == key)
      {
        results.emplace_back(line);
      }
    }

    return results;
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
    return count > 0 && result;
  }

}// namespace siege::configuration
