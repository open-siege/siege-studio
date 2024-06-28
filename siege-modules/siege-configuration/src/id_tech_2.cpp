#include <limits>
#include <memory>
#include <siege/configuration/id_tech.hpp>

namespace siege::configuration::id_tech
{
    namespace id_tech_2
    {
        using config_line = text_game_config::config_line;
        constexpr static auto end_line = std::string_view("\r\n");

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

        std::vector<std::string_view> split_into_segments(std::string_view line)
        {
            std::vector<std::string_view> segments;
            segments.reserve(3);
            std::optional<std::size_t> first_quote;

            auto start = 0u;

            for (auto i = 0u; i < line.size(); ++i)
            {
                if (first_quote.has_value())
                {
                    if (line[i] == '\"')
                    {
                        start = first_quote.value() + 1;
                        first_quote.reset();
                        segments.emplace_back(line.substr(start, i - start));
                        start = i + 1;

                        if (i < line.size() - 1 && line[i + 1] == ' ')
                        {
                            start++;
                        }
                    }
                    continue;
                }


                if (line[i] == '\"')
                {
                    first_quote.emplace(i);
                    continue;
                }

                if (i < line.size() - 1 && line[i + 1] == ' ')
                {
                    segments.emplace_back(line.substr(start, i + 1 - start));
                    start = i + 2;
                }
                else if (i == line.size() - 1)
                {
                    segments.emplace_back(line.substr(start, i));
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

            auto current_pos = raw_data.tellg();
            int count = 0;
            if (!std::all_of(std::istreambuf_iterator<char>{ raw_data }, std::istreambuf_iterator<char>{}, [&](char raw) {
               
                  auto value = static_cast<unsigned char>(raw);
                  return count++ != 4096 && std::isalpha(value) || std::isdigit(value) || std::isspace(value) || std::ispunct(value);
                }))
            {
              raw_data.seekg(current_pos, std::ios::beg);
              return std::nullopt;
            }

            std::unique_ptr<char[]> buffer(new char[stream_size]);
            std::fill_n(buffer.get(), stream_size, '\0');
            std::string_view buffer_str(buffer.get(), stream_size);

            raw_data.seekg(current_pos, std::ios::beg);
            raw_data.read(buffer.get(), stream_size);
            raw_data.seekg(current_pos, std::ios::beg);

            auto lines = split_into_lines(buffer_str, end_line, stream_size / average_line_size);

            std::vector<config_line> config_data;
            config_data.reserve(lines.size());

            for (auto& line : lines)
            {
                auto segments = split_into_segments(line);

                if (segments.empty())
                {
                    continue;
                }

                if (segments.size() == 1)
                {
                    config_data.emplace_back(config_line{line, segments, std::string_view{""}});
                    continue;
                }

                auto value = segments.back();
                segments.pop_back();
                config_data.emplace_back(config_line{line, segments, value});
            }

            if (config_data.empty())
            {
                return std::nullopt;
            }
            
            return std::make_optional<text_game_config>(std::move(buffer), std::move(config_data), save_config);
        }

        std::string join_line(const config_line& line)
        {
            auto segments = line.key_segments.to_array<2>();

            std::string result;
            result.reserve(segments[0].size() + segments[1].size() + line.value.at(0).size() + 2 + 4); // 2 spaces and 4 quotes

            for (auto segment : segments)
            {
                if (segment.empty())
                {
                    continue;
                }

                if (segment.find(' ') != std::string_view::npos)
                {
                    result.push_back('\"');
                }

                result.append(segment);

                if (segment.find(' ') != std::string_view::npos)
                {
                    result.push_back('\"');
                }

                result.push_back(' ');
            }

            result.push_back('\"');
            result.append(line.value.at(0));
            result.push_back('\"');

            return result;
        }

        void save_config(const std::vector<config_line>& entries, std::ostream& raw_data)
        {
            for (auto& entry : entries)
            {
                auto line = join_line(entry);

                raw_data.write(line.data(), line.size());
                raw_data.write(end_line.data(), end_line.size());
            }
        }
    }
}