#include <limits>
#include "configurations/id_tech.hpp"
#include "endian_arithmetic.hpp"

namespace studio::configurations::id_tech
{
    namespace endian = boost::endian;

    namespace id_tech_0_5
    {
        namespace rott
        {
            constexpr static auto end_line = std::string_view("\r\n");
            std::optional<game_config> load_config(std::istream& raw_data, std::size_t stream_size)
            {
                if (stream_size == 0)
                {
                    return std::nullopt;
                }

                game_config result{};
                result.raw_data.reserve(stream_size);
                auto current_pos = raw_data.tellg();
                raw_data.read(result.raw_data.data(), result.raw_data.size());
                raw_data.seekg(current_pos, std::ios::beg);

                bool is_valid = result.raw_data.find("Version ") != std::string::npos &&
                                result.raw_data.find("MouseEnabled ") != std::string::npos &&
                                result.raw_data.find("MouseAdjustment ") != std::string::npos &&
                                result.raw_data.find("BobbingOn ") != std::string::npos &&
                                result.raw_data.find("GammaIndex ") != std::string::npos;


                if (!is_valid)
                {
                    return std::nullopt;
                }

                result.config_data.reserve(std::count(result.raw_data.begin(), result.raw_data.end(), '\r'));

                auto key_start = 0u;
                auto key_end = 0u;
                auto value_start = 0u;
                auto value_end = 0u;

                do
                {
                    key_end = result.raw_data.find(' ', key_start);

                    if (key_end == std::string::npos)
                    {
                        break;
                    }

                    value_end = result.raw_data.find(end_line, key_end);

                    if (value_end == std::string::npos)
                    {
                        break;
                    }

                    value_start = result.rfind(' ', value_end) + 1;

                    auto key = std::string_view(result.raw_data.begin() + key_start, result.raw_data.begin() + key_end);

                    if (key.find(';') == 0)
                    {
                        continue;
                    }

                    auto value = std::string_view(result.raw_data.begin() + value_start, result.raw_data.begin() + value_end);
                    result.add(key, try_parse(value_str));

                    if (value_end + end_line.size() > result.raw_data.size())
                    {
                        break;
                    }
                    
                    key_start = value_end + end_line.size();
                }

                while (true);

                return result;
            }

            void save_config(std::istream&, const game_config&);
        }
    }
}