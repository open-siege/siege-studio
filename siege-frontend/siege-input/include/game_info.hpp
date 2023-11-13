#ifndef OPEN_SIEGE_GAME_INFO_HPP
#define OPEN_SIEGE_GAME_INFO_HPP

#include <string_view>
#include <type_traits>
#include <variant>
#include <optional>
#include "configurations/shared.hpp"
#include "joystick_info.hpp"

namespace siege
{
    struct game_info
    {
        struct binary_config
        {
            using binary_game_config = studio::configurations::binary_game_config;
            binary_game_config config;
            std::optional<binary_game_config>(&load_config)(std::istream&, std::size_t);
            void(&save_config)(std::ostream&, const binary_game_config&);
        };

        struct text_config
        {
            using text_game_config = studio::configurations::text_game_config;
            text_game_config config;
            std::optional<text_game_config>(&load_config)(std::istream&, std::size_t);
            void(&save_config)(std::ostream&, const text_game_config&);
        };

        using config_info = std::variant<text_config, binary_config>;

        std::string_view english_name;
        joystick_info (&add_input_metadata)(joystick_info);
        joystick_info (&add_default_actions)(joystick_info);
        config_info (&create_game_config)(joystick_info);
    };
}


#endif