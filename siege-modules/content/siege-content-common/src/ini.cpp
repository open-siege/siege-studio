export module siege.configuration.ini;

import std;

import siege.configuration.shared;

namespace siege::configuration::common::ini
{

  export struct ini_context
  {
    std::string default_end_line = "\r\n";
  };

  using config_line = text_game_config::config_line;

  export void save_config(const std::any&, const std::vector<config_line>& entries, std::ostream& raw_data);

  constexpr static auto average_line_size = 20;

  std::vector<std::string_view> split_into_lines(std::string_view line, std::string_view separator, std::size_t size_hint = 4)
  {
    std::vector<std::string_view> segments;
    segments.reserve(size_hint);

    auto start = 0u;

    for (auto i = 0u; i < line.size(); ++i)
    {
      if (i < line.size() - separator.size() && std::string_view(line.data() + 1 + i, separator.size()) == separator)
      {
        segments.emplace_back(line.substr(start, i + 1 - start));
        start = i + separator.size() + 1;
      }
      else if (i == line.size() - separator.size())
      {
        segments.emplace_back(line.substr(start, i + separator.size()));
      }
    }

    return segments;
  }

  std::string_view trim_trailing_cr(std::string_view line)
  {
    if (!line.empty() && line.back() == '\r')
    {
      return line.substr(0, line.size() - 1);
    }
    return line;
  }

  bool is_section_header(std::string_view line)
  {
    line = trim_trailing_cr(line);
    auto trimmed = line;
    while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t'))
    {
      trimmed.remove_prefix(1);
    }
    return trimmed.starts_with('[') && trimmed.contains(']') && !trimmed.contains('=');
  }

  bool is_passthrough_line(std::string_view line)
  {
    line = trim_trailing_cr(line);
    if (line.empty())
    {
      return true;
    }
    auto trimmed = line;
    while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t'))
    {
      trimmed.remove_prefix(1);
    }
    return trimmed.empty() || trimmed.starts_with(';') || trimmed.starts_with("//");
  }

  export std::optional<text_game_config> load_config(std::istream& raw_data, std::size_t stream_size)
  {
    if (stream_size == 0)
    {
      return std::nullopt;
    }

    if (!is_ascii_text_config(raw_data))
    {
      return std::nullopt;
    }

    std::unique_ptr<char[]> buffer(new char[stream_size]);
    std::fill_n(buffer.get(), stream_size, '\0');
    std::string_view buffer_str(buffer.get(), stream_size);

    auto current_pos = raw_data.tellg();
    raw_data.seekg(current_pos, std::ios::beg);
    raw_data.read(buffer.get(), stream_size);
    raw_data.seekg(current_pos, std::ios::beg);

    ini_context context{};

    auto lines = split_into_lines(buffer_str, context.default_end_line, stream_size / average_line_size);

    if (lines.size() == 1)
    {
      lines = split_into_lines(buffer_str, "\n", stream_size / average_line_size);

      if (lines.size() > 1)
      {
        context.default_end_line = "\n";
      }
    }

    std::vector<config_line> config_data;
    config_data.reserve(lines.size());

    std::string_view current_group{};
    bool saw_setting = false;

    for (auto& line : lines)
    {
      auto visible = trim_trailing_cr(line);

      if (is_section_header(visible))
      {
        auto left_bracket = visible.find('[');
        auto right_bracket = visible.rfind(']');
        if (right_bracket > left_bracket + 1)
        {
          current_group = visible.substr(left_bracket + 1, right_bracket - left_bracket - 1);
        }
        config_data.emplace_back(config_line{ line, key_type{ "" }, key_type{ "" } });
        continue;
      }

      if (is_passthrough_line(visible))
      {
        config_data.emplace_back(config_line{ line, key_type{ "" }, key_type{ "" } });
        continue;
      }

      if (current_group.empty())
      {
        config_data.emplace_back(config_line{ line, key_type{ "" }, key_type{ "" } });
        continue;
      }

      auto equals = visible.find('=');
      if (equals == std::string_view::npos)
      {
        config_data.emplace_back(config_line{ line, key_type{ "" }, key_type{ "" } });
        continue;
      }

      auto key = visible.substr(0, equals);
      auto value = visible.substr(equals + 1);
      saw_setting = true;
      config_data.emplace_back(config_line{ line, std::vector{ current_group, key }, value });
    }

    if (!saw_setting)
    {
      return std::nullopt;
    }

    return std::make_optional<text_game_config>(std::make_any<ini_context>(context), std::move(buffer), std::move(config_data), save_config);
  }

  void save_config(const std::any& context, const std::vector<config_line>& entries, std::ostream& raw_data)
  {
    ini_context real_context{};
    if (context.has_value() && context.type() == typeid(ini_context))
    {
      real_context = std::any_cast<ini_context>(context);
    }

    std::string_view current_group{};

    for (auto& entry : entries)
    {
      if (entry.key_segments.empty())
      {
        if (!entry.raw_line.empty())
        {
          auto visible = trim_trailing_cr(entry.raw_line);
          if (is_section_header(visible))
          {
            auto left_bracket = visible.find('[');
            auto right_bracket = visible.rfind(']');
            if (right_bracket > left_bracket + 1)
            {
              current_group = visible.substr(left_bracket + 1, right_bracket - left_bracket - 1);
            }
          }
          raw_data.write(entry.raw_line.data(), entry.raw_line.size());
          if (!entry.raw_line.ends_with('\n') && !entry.raw_line.ends_with('\r'))
          {
            raw_data.write(real_context.default_end_line.data(), real_context.default_end_line.size());
          }
        }
        else
        {
          raw_data.write(real_context.default_end_line.data(), real_context.default_end_line.size());
        }
        continue;
      }

      auto group = entry.key_segments.at(0);
      auto key = entry.key_segments.at(1);
      auto value = entry.value.at(0);

      if (group != current_group)
      {
        current_group = group;
        raw_data.put('[');
        raw_data.write(group.data(), group.size());
        raw_data.put(']');
        raw_data.write(real_context.default_end_line.data(), real_context.default_end_line.size());
      }

      raw_data.write(key.data(), key.size());
      raw_data.put('=');
      raw_data.write(value.data(), value.size());
      raw_data.write(real_context.default_end_line.data(), real_context.default_end_line.size());
    }
  }
}// namespace siege::configuration::common::ini
