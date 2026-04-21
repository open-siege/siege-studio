#include <limits>
#include <memory>
#include <siege/configuration/ini.hpp>
#include <any>

namespace siege::configuration::common::ini
{
  using config_line = text_game_config::config_line;

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

  std::optional<text_game_config> load_config(std::istream& raw_data, std::size_t stream_size)
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

    for (auto& line : lines)
    {
      if (line.contains("[") && line.contains("]") && !line.contains("="))
      {
        auto left_bracket = line.find("[");
        auto right_bracket = line.rfind("]");

        if (right_bracket > left_bracket + 1)
        {
          current_group = line.substr(left_bracket + 1, right_bracket - left_bracket - 1);
        }
      }

      if (current_group.empty())
      {
        continue;
      }

      auto equals = line.find("=");

      if (equals == std::string_view::npos)
      {
        continue;
      }
      auto key = line.substr(0, equals);
      auto value = line.substr(equals + 1);

      config_data.emplace_back(config_line{ line, std::vector{ current_group, key }, value });
    }

    if (config_data.empty())
    {
      return std::nullopt;
    }

    return std::make_optional<text_game_config>(std::make_any<ini_context>(context), std::move(buffer), std::move(config_data), save_config);
  }

  void save_config(const std::any& context, const std::vector<config_line>& entries, std::ostream& raw_data)
  {
  }
}// namespace siege::configuration::common::ini