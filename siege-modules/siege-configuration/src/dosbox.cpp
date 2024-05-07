#include <siege/configuration/dosbox.hpp>

namespace siege::configuration::dosbox
{
    constexpr static auto end_line = std::string_view("\r\n");

    namespace mapper
    {
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

        constexpr static auto average_line_size = 100;
        using config_line = text_game_config::config_line;

        std::optional<text_game_config> load_config(std::istream& raw_data, std::size_t stream_size)
        {
            if (stream_size == 0)
            {
                return std::nullopt;
            }


            std::unique_ptr<char[]> buffer(new char[stream_size]);
            std::fill_n(buffer.get(), stream_size, '\0');
            std::string_view buffer_str(buffer.get(), stream_size);

            auto current_pos = raw_data.tellg();
            raw_data.read(buffer.get(),  stream_size);
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

                auto key = segments.front();
                segments.erase(segments.begin(), ++segments.begin());
                config_data.emplace_back(config_line{line, key, segments});
            }

            if (config_data.empty())
            {
                return std::nullopt;
            }
            
            return std::make_optional<text_game_config>(std::move(buffer), std::move(config_data), save_config);
        }

        void save_config(const std::vector<text_game_config::config_line>&, std::ostream&)
        {

        }
    }

    namespace config
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t)
        {
            return std::nullopt;
        }

        void save_config(const std::vector<text_game_config::config_line>&, std::ostream&)
        {

        }
    }
}