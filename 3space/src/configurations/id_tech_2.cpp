#include <limits>
#include "configurations/id_tech.hpp"
#include "endian_arithmetic.hpp"

namespace studio::configurations::id_tech
{
    namespace endian = boost::endian;

    namespace id_tech_2
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

            bool is_valid = result.raw_data.find("bind ") != std::string::npos;
            bool is_idtech_2_0 = false;
            bool is_idtech_2_5 = false;
            bool is_idtech_3_0 = false;

            if (is_valid)
            {
                is_idtech_2_0 = 
                            result.raw_data.find("bind \"") != std::string::npos &&
                            result.raw_data.find("cl_forwardspeed \"") != std::string::npos &&
                            result.raw_data.find("cl_backspeed \"") != std::string::npos;

                is_idtech_2_5 = 
                            result.raw_data.find("bind ") != std::string::npos &&
                            result.raw_data.find("set ") != std::string::npos;

                is_idtech_3_0 = 
                            result.raw_data.find("bind ") != std::string::npos &&
                            result.raw_data.find("seta ") != std::string::npos;
            }

            if (!(is_idtech_2_0 || is_idtech_2_5 || is_idtech_3_0))
            {
                is_valid = false;
            }

            if (!is_valid)
            {
                return std::nullopt;
            }

            result.config_data.reserve(std::count(result.raw_data.begin(), result.raw_data.end(), end_line[0]));

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

                auto key = std::string_view(result.raw_data.begin() + key_start, result.raw_data.begin() + key_end);

                if (key.find("//") == 0)
                {
                    continue;
                }
                
                if (key == "bind")
                {
                    value_start = key_end + 1;
                    value_end = result.raw_data.find(' ', value_start);
                    
                    if (value_end == std::string::npos)
                    {
                        break;
                    }

                    std::array<std::string_view, 2> value_pair{};
                    value_pair[0] = std::string_view(result.raw_data.begin() + value_start, result.raw_data.begin() + value_end);

                    if (has_quotes(value_pair[0]))
                    {
                        value_pair[0] = unquote(value_pair[0]);
                        result.add("_BindFirstValueHasQuotes", true);
                    }

                    value_start = value_end + 1;
                    value_end = result.raw_data.find("\r\n", value_start);

                    if (value_end == std::string::npos)
                    {
                        break;
                    }

                    auto value = std::string_view(result.raw_data.begin() + value_start, result.raw_data.begin() + value_end);                    
                    value_pair[1] = try_parse(value);
                    result.add(key, value_pair);
                }
                // TODO this block needs to properly add
                else if (key == "set" || key == "seta")
                {
                    value_start = key_end + 1;
                    value_end = result.raw_data.find(' ', value_start);
                    
                    if (value_end == std::string::npos)
                    {
                        break;
                    }

                    result.add("_SetIsUsed", key == "set");
                    result.add("_SetaIsUsed", key == "seta");

                    key = std::string_view(result.raw_data.begin() + value_start, result.raw_data.begin() + value_end);

                    value_start = value_end + 1;
                    value_end = result.raw_data.find(end_line, value_start);

                    if (value_end == std::string::npos)
                    {
                        break;
                    }

                    auto value = std::string_view(result.raw_data.begin() + value_start, result.raw_data.begin() + value_end);

                    result.add(key, try_parse(value));
                }
                else
                {
                    value_start = key_end + 1;
                    value_end = result.raw_data.find(end_line, value_start);

                    if (value_end == std::string::npos)
                    {
                        break;
                    }

                    auto value = std::string_view(result.raw_data.begin() + value_start, result.raw_data.begin() + value_end);                    
                    result.add(key, try_parse(value));
                }

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