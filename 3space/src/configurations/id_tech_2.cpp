#include <limits>
#include "configurations/id_tech.hpp"
#include "endian_arithmetic.hpp"

namespace studio::configurations::id_tech
{
    namespace endian = boost::endian;

    namespace id_tech_2
    {
        constexpr static auto end_line = std::string_view("\r\n");

        constexpr static auto average_line_size = 20;

        std::vector<std::string_view> split_string_view(std::string_view line, std::string_view separator, std::size_t size_hint = 4)
        {
            auto start = 0u;
            auto space = line.find(separator, start);

            if (space == std::string_view::npos)
            {
                return { line };
            }

            std::vector<std::string_view> segments;
            segments.reserve(size_hint);   

            return segments;
        }

        std::vector<std::string_view> split_line(std::string_view line)
        {
            std::vector<std::string_view> segments;
            results.reserve(3);
            std::optional<std::size_t> first_quote;
            auto start = 0u;
            for (auto i = 0u; i < line.size(); ++i)
            {
                if (i == ' ' && !first_quote.has_value())
                {
                    results.emplace_back(line.substr(start, i - start));
                    start = i + 1;
                }

                if (line[i] == '\"' && !first_quote.has_value())
                {
                    first_quote.emplace(i);
                }

                if (line[i] == '\"' && first_quote.has_value())
                {
                    start = first_quote.value() + 1;
                    results.emplace_back(line.substr(start,  - 1 - start));
                    first_quote.reset();
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

            std::string buffer;
            buffer.assign(stream_size, '\0');
            auto current_pos = raw_data.tellg();
            raw_data.read(buffer.data(), stream_size);
            raw_data.seekg(current_pos, std::ios::beg);

            auto lines = split_string_view(buffer, end_line, stream_size / average_line_size);

            std::vector<config_line> config_data;
            config_data.reserve(lines.size());

            for (auto& line : lines)
            {
                auto segments = split_line(line);

                if (segments.empty())
                {
                    continue;
                }

                if (segments.size() == 1)
                {
                    config_data.emplace_back(config_line{line, segments, ""});
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
            
            return text_game_config(std::move(buffer), std::move(config_data));
        }

        std::string join_line(const config_line& line)
        {
            auto segments = line.key_segments.to_array<2>();

            std::string result;
            result.reserve(segments[0].size() + segments[1].size() + line.value.size() + 2 + 4); // 2 spaces and 4 quotes

            for (auto segment : segments)
            {
                if (segment.empty())
                {
                    continue;
                }

                if (segment.find(' ') != std::string_view::npos)
                {
                    result.append('\"');
                }

                result.append(segment);

                if (segment.find(' ') != std::string_view::npos)
                {
                    result.append('\"');
                }

                result.append(' ');
            }

            result.append('\"');
            result.append(line.value);
            result.append('\"');
        }


        void save_config(std::istream& raw_data, const text_game_config& config)
        {
            config.persist([&] (const auto& entries) 
            {
                for (auto& entry : entries)
                {
                    auto line = join_line(entry);

                    raw_data.write(line.data(), line.size());
                    raw_data.write(end_line.data(), end_line.size());
                }
            });
        }
    }
}