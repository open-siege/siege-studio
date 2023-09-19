#include <limits>
#include "configurations/id_tech.hpp"
#include "endian_arithmetic.hpp"

namespace studio::configurations::id_tech
{
    namespace endian = boost::endian;

    namespace id_tech_2
    {
        constexpr static auto end_line = std::string_view("\r\n");

        std::optional<text_game_config> load_config(std::istream& raw_data, std::size_t stream_size)
        {
            if (stream_size == 0)
            {
                return std::nullopt;
            }

            text_game_config result{};
            result.raw_data.assign(stream_size, '\0');
            auto current_pos = raw_data.tellg();
            raw_data.read(result.raw_data.data(), stream_size);
            raw_data.seekg(current_pos, std::ios::beg);

            auto line_count = 0;
            std::string::size_type start = 0;

            while ((start = result.raw_data.find(end_line, start)) != string::npos) {
                ++line_count;
                start += end_line.length();
            }

            text_game_config.config_data.reserve(line_count);

            std::string_view document(result.raw_data);

            std::size_t length = 0;

            for (start = 0; start < document.size(); start += length)
            {
                auto line_end = document.find(end_line, start);

                length = line_end == std::string::npos ? document.size() - index : line_end - index;
                auto line = document.substring(start, length);
                text_game_config.config_data.emplace_back(std::make_pair(line, std::vector<std::string_view>{}));
            }
            
            return result;
        }
        void save_config(std::istream&, const game_config&);
    }
}