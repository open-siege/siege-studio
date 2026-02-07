#ifndef SHARED_CONFIG_HPP
#define SHARED_CONFIG_HPP

#include <siege/platform/shared.hpp>
#include <siege/configuration/config_types.hpp>
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

namespace siege::configuration
{
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
            bool contains(key_type key) const;
            key_type find(key_type key) const;
            text_game_config&& emplace(key_type key, key_type value);
            text_game_config&& remove(key_type key);

            void save(std::ostream&) const;
        private:
            persist& save_config;
            std::shared_ptr<const char[]>  raw_data;
            std::vector<config_line> line_entries;
    };

    bool is_ascii_text_config(std::istream&);
}

#endif