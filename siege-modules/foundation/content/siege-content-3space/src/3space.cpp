#include <limits>
#include <memory>
#include <ranges>
#include <siege/configuration/3space.hpp>

namespace siege::configuration::three_space
{
  namespace three_space_3
  {
    using config_line = text_game_config::config_line;
    constexpr static auto end_line = std::string_view("\r\n");

    constexpr static auto average_line_size = 20;


    std::vector<std::string_view> split_into_lines(std::string_view line, std::string_view separator, std::size_t size_hint = 4);

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

      auto lines = split_into_lines(buffer_str, end_line, stream_size / average_line_size);

      std::vector<config_line> config_data;
      config_data.reserve(lines.size());

      for (auto iter = lines.begin(); iter != lines.end(); ++iter)
      {
        auto& line = *iter;
        auto is_comment = false;
        auto first_hash = line.find("#");

        if (first_hash == 0)
        {
          is_comment = true;
        }
        else if (first_hash != std::string_view::npos)
        {
          auto white_space_count = std::count_if(line.begin(), line.begin() + first_hash, [](auto value) { return value == ' ' || value == '\t'; });

          is_comment = white_space_count == first_hash;
        }

        if (is_comment)
        {
          config_data.emplace_back(config_line{ .raw_line = line, .key_segments = {}, .value = {} });
          continue;
        }

        auto first_char = std::find_if(line.begin(), line.end(), [](auto value) { return !(value == ' ' || value == '\t'); });

        if (first_char == line.end())
        {
          continue;
        }

        auto line_ending = std::find_if(line.rbegin(), line.rend(), [](auto value) { return value == ';'; });

        if (line_ending == line.rend())
        {
          continue;
        }

        std::vector<std::string_view> sub_lines;

        std::string_view trimmed(first_char, line_ending.base());

        if (std::count(line.begin(), line.end(), ';') > 1)
        {
          bool inside_quote = false;
          auto count = std::count(line.begin(), line.end(), ';');

          for (auto i = 0; i < count; ++i)
          {
            auto start = trimmed.begin();

            if (!sub_lines.empty())
            {
              auto diff = (std::size_t)(sub_lines.back().data() - trimmed.data());
              diff += sub_lines.back().size() + 1;
              start += diff;
            }

            auto line_ending = std::find_if(start, trimmed.end(), [&](auto value) {
              if (!inside_quote && value == '\"')
              {
                inside_quote = true;
                return false;
              }

              if (inside_quote && value == '\"')
              {
                inside_quote = false;
                return false;
              }

              return value == ';';
            });

            sub_lines.emplace_back(std::string_view(start, line_ending));
          }
        }
        else
        {
          sub_lines.emplace_back(trimmed);
        }

        for (auto& trimmed : sub_lines)
        {
          auto function_ending = std::find_if(trimmed.begin(), trimmed.end(), [](auto value) {
            return value == ' ' || value == '\t' || value == '(';
          });

          if (function_ending == trimmed.end())
          {
            continue;
          }

          std::string_view function_name = trimmed.substr(0, std::distance(trimmed.begin(), function_ending));

          auto first_arg = std::find_if(function_ending, trimmed.end(), [](auto value) {
            return !(value == ' ' || value == '\t' || value == '(');
          });

          auto end_arg = std::find_if(trimmed.rbegin(), trimmed.rend(), [](auto value) {
            return !(value == ' ' || value == '\t' || value == ')' || value == ';');
          });


          if (first_arg == trimmed.end())
          {
            continue;
          }

          if (end_arg == trimmed.rend())
          {
            continue;
          }

          auto start = std::distance(trimmed.begin(), first_arg);
          auto count = std::distance(first_arg, end_arg.base());
          auto args = trimmed.substr(start, count);

          std::vector<std::string_view> keys;
          std::vector<std::string_view> final_args;

          std::vector<std::string_view> temp_args;

          keys.emplace_back(function_name);

          if (args.contains(","))
          {
            bool contains_to = false;
            for (const auto& arg : args | std::views::split(','))
            {
              std::string_view temp(arg.begin(), arg.end());

              auto first_char = std::find_if(temp.begin(), temp.end(), [](auto value) {
                return !(value == ' ' || value == '\t');
              });

              auto end_char = std::find_if(temp.rbegin(), temp.rend(), [](auto value) {
                return !(value == ' ' || value == '\t');
              });

              if (first_char == temp.end())
              {
                continue;
              }

              if (end_char == temp.rend())
              {
                continue;
              }

              temp = temp.substr(std::distance(temp.begin(), first_char), std::distance(first_char, end_char.base()));

              if (!contains_to)
              {
                temp_args.emplace_back(temp);
              }
              else
              {
                final_args.emplace_back(temp);
              }

              if (temp == "TO" || temp == "to")
              {
                contains_to = true;
              }
            }

            for (auto& arg : temp_args)
            {
              keys.emplace_back(arg);
            }
          }
          else
          {
            keys.emplace_back(args);
          }

          config_data.emplace_back(config_line{ line, keys, final_args });
        }
      }


      return std::make_optional<text_game_config>(std::move(buffer), std::move(config_data), save_config);
    }

    void save_config(const std::vector<config_line>& entries, std::ostream& raw_data)
    {
    }

    std::vector<std::string_view> split_into_lines(std::string_view line, std::string_view separator, std::size_t size_hint)
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
  }// namespace three_space_3
}// namespace siege::configuration::three_space