#ifndef SHARED_CONFIG_HPP
#define SHARED_CONFIG_HPP

#include "configurations/config_types.hpp"
#include <vector>
#include <array>
#include <variant>
#include <utility>
#include <string>
#include <string_view>
#include <cstdint>
#include <functional>
#include <istream>
#include <ostream>

namespace studio::configurations
{
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

    void write(little_endian_stream_writer&, const config_value&);

    class binary_game_config
    {
        public:
            struct config_entry
            {
                std::string_view key;
                config_value value;
            };

            binary_game_config() = default;
            binary_game_config(std::vector<config_entry>&&);
            binary_game_config(const binary_game_config&);
            binary_game_config(binary_game_config&&) = delete;

            std::vector<std::string_view> keys() const;
            std::optional<config_value> find(std::string_view key) const;
            void emplace(std::string_view key, config_value value);
            void remove(std::string_view key);

            void persist(std::function<void(const std::vector<config_entry>&)>) const;
        private: 
            std::vector<config_entry> entries;
    };

    class text_game_config
    {
        public:
            struct config_line
            {
                std::string_view raw_line;
                key_type key_segments;
                std::string_view value;
            };

            text_game_config() = default;
            text_game_config(std::string&&, std::vector<config_line>&&);
            text_game_config(const text_game_config&);
            text_game_config(text_game_config&&) = delete;

            std::vector<key_type> keys() const;
            std::string_view find(key_type key) const;
            void emplace(key_type key, std::string_view value);
            void remove(key_type key);

            void persist(std::function<void(const std::vector<config_line>&)>) const;

        private:
            std::string raw_data;
            std::vector<config_line> line_entries;
    };
}

#endif