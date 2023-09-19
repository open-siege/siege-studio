#ifndef SHARED_CONFIG_HPP
#define SHARED_CONFIG_HPP

#include <vector>
#include <utility>
#include <string>
#include <string_view>
#include <cstdint>

namespace studio::configurations
{
    using number_variant = std::variant<std::int8_t, 
                                        std::uint8_t, 
                                        std::uint16_t, 
                                        std::int16_t, 
                                        std::uint32_t, 
                                        std::int32_t, 
                                        std::uint64_t, 
                                        std::int64_t, 
                                        float, 
                                        double>;
    using config_value = std::variant<string_variant, number_variant, bool>;


    struct binary_game_config
    {

    }

    struct text_game_config
    {
        std::string raw_data;
        std::vector<std::pair<std::string_view, std::vector<std::string_view>>> config_data;

        std::optional<std::reference_wrapper<config_value>> find(std::string_view key) const;
        std::string find_string(std::string_view key) const;
        std::optional<bool> find_bool(std::string_view key);
        std::optional<double> find_double(std::string_view key);
        std::optional<double> find_float(std::string_view key);
        std::optional<std::uint32_t> find_uint32_t(std::string_view key);
        std::optional<std::uint8_t> find_uint8_t(std::string_view key);
    };

    struct little_endian_stream_reader
    {
        std::istream& stream;

        std::uint32_t read_uint32();
        std::uint8_t read_uint8();
    };

    struct little_endian_stream_writer
    {
        std::ostream& stream;
        void write(const std::uint32_t&);
        void write(const std::uint8_t&);
        void write(const std::string_view);
    };
}

#endif