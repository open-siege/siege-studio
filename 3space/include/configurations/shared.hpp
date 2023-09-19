#ifndef SHARED_CONFIG_HPP
#define SHARED_CONFIG_HPP

#include <vector>
#include <array>
#include <utility>
#include <string>
#include <string_view>
#include <variant>
#include <array>
#include <cstdint>

namespace studio::configurations
{
    using string_variant = std::variant<std::array<std::string_view, 1>, 
                                        std::array<std::string_view, 2>,
                                        std::array<std::string_view, 3>,
                                        std::vector<std::string_view> segments>
                                        >;
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
    using config_vector = std::vector<std::pair<std::string_view, config_value>>;

    config_value try_parse(std::string_view);

    string_variant to_string_variant(std::string_view);
    std::string to_string(const string_variant&);
    std::string to_string(const std::optional<std::reference_wrapper<config_value>>&);

    bool has_quotes(std::string_view);
    std::string_view unquote(std::string_view);

    std::optional<std::int8_t> get_int8_t(const number_variant&);
    std::optional<std::uint8_t> get_uint8_t(const number_variant&);
    std::optional<std::int16_t> get_int16_t(const number_variant&);
    std::optional<std::uint16_t> get_uint16_t(const number_variant&);
    std::optional<std::int32_t> get_int32_t(const number_variant&);
    std::optional<std::int64_t> get_int64_t(const number_variant&);
    std::optional<std::uint64_t> get_uint64_t(const number_variant&);
    std::optional<float> get_float(const number_variant&);
    std::optional<double> get_double(const number_variant&);
    int to_int(const number_variant&);
    uint to_uint(const number_variant&);
    float to_float(const number_variant&);
    double to_double(const number_variant&);
    std::string to_string(const number_variant&);
    

    std::optional<std::int8_t> get_int8_t(const config_value&);
    std::optional<std::uint8_t> get_uint8_t(const config_value&);
    std::optional<std::int16_t> get_int16_t(const config_value&);
    std::optional<std::uint16_t> get_uint16_t(const config_value&);
    std::optional<std::int32_t> get_int32_t(const config_value&);
    std::optional<std::uint32_t> get_uint32_t(const config_value&);
    std::optional<std::int64_t> get_int64_t(const config_value&);
    std::optional<std::uint64_t> get_uint64_t(const config_value&);
    std::optional<float> get_float(const config_value&);
    std::optional<double> get_double(const config_value&);
    std::optional<bool> get_bool(const config_value&);
    int to_int(const config_value&);
    uint to_uint(const config_value&);
    float to_float(const config_value&);
    double to_double(const config_value&);

    std::optional<std::reference_wrapper<config_value>> find(const config_vector&, std::string_view key);
    std::string find_to_string(const config_vector&, std::string_view key);
    std::optional<bool> find_bool(const config_vector&, std::string_view key);
    std::optional<std::uint32_t> find_uint32_t(const config_vector&, std::string_view key);
    std::optional<std::uint8_t> find_uint8_t(const config_vector&, std::string_view key);

    struct game_config
    {
        std::string raw_data;
        config_vector config_data;

        void add(std::string_view, std::string_view);
        void add(std::string_view, std::uint32_t);
        void add(std::string_view, std::uint8_t);
        void add(std::string_view, bool);

        std::optional<std::reference_wrapper<config_value>> find(std::string_view key) const;
        std::string find_to_string(std::string_view key) const;
        std::optional<bool> find_bool(std::string_view key);
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