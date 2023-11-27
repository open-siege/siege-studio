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
#include <memory>

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

            using persist = void(&)(const std::vector<config_entry>&, std::ostream&);

            binary_game_config(persist);
            binary_game_config(std::vector<config_entry>&&, persist);
            binary_game_config(binary_game_config&&) = default;
            binary_game_config() = delete;
            binary_game_config(const binary_game_config&) = delete;

            std::vector<std::string_view> keys() const;
            std::optional<config_value> find(std::string_view key) const;
            void emplace(std::string_view key, config_value value);
            void remove(std::string_view key);

            void save(std::ostream&) const;
        private: 
            persist save_config;
            std::vector<config_entry> entries;
    };

    class text_game_config
    {
        public:
            struct config_line
            {
                std::string_view raw_line;
                key_type key_segments;
                key_type value;
            };

            using persist = void(const std::vector<config_line>&, std::ostream&);

            text_game_config(persist&);
            text_game_config(std::unique_ptr<char[]> &&, std::vector<config_line>&&, persist&);

            std::vector<key_type> keys() const;
            key_type find(key_type key) const;
            text_game_config&& emplace(key_type key, key_type value);
            text_game_config&& remove(key_type key);

            void save(std::ostream&) const;
        private:
            persist& save_config;
            std::unique_ptr<char[]>  raw_data;
            std::vector<config_line> line_entries;
    };
}

#endif