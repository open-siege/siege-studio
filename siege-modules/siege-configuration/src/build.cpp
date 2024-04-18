#include <siege/configuration/dosbox.hpp>
#include <iostream>

namespace siege::configuration::build
{
    using config_line = text_game_config::config_line;
    constexpr static auto end_line = std::string_view("\r\n");

    constexpr static auto average_line_size = 100;

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

    bool is_root(std::string_view root)
    {
        return root.find('[') == 0 && root.rfind(']') == root.size() - 1;
    }

    std::string_view get_root(std::string_view root)
    {
        if (is_root(root))
        {
            return root.substr(1, root.size() - 2);
        }

        return "";
    }

    std::array<std::string_view, 2> split_into_key_value(std::string_view line)
    {
            std::array<std::string_view, 2> segments;
            
            if (line.empty())
            {
                return segments;
            }

            segments[0] = line.substr(0, line.find('='));
            segments[0] = segments[0].substr(0, segments[0].find_last_not_of(' ') + 1);

            segments[1] = line.substr(line.find('=') + 1);
            segments[1] = segments[1].substr(segments[1].find_first_not_of(' '));
            segments[1] = segments[1].substr(segments[1].find_first_not_of('\"'), segments[1].find_last_not_of('\"') - segments[1].find_first_not_of('\"') + 1);

            return segments;
    }

    void save_config(const std::vector<text_game_config::config_line>&, std::ostream&)
    {

    }

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

            std::string_view root;

            for (auto& line : lines)
            {
                if (is_root(line))
                {
                    root = get_root(line);
                    continue;
                }

                auto [key, value] = split_into_key_value(line);

                if (key.empty() && value.empty())
                {
                    continue;
                }

                config_data.emplace_back(config_line{line, {root, key}, value});
            }

            if (config_data.empty())
            {
                return std::nullopt;
            }
            
            return std::make_optional<text_game_config>(std::move(buffer), std::move(config_data), save_config);
    }

    
}