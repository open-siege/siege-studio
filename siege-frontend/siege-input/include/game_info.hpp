#ifndef OPEN_SIEGE_GAME_INFO_HPP
#define OPEN_SIEGE_GAME_INFO_HPP

#include <string_view>
#include <type_traits>
#include <variant>
#include <optional>
#include <filesystem>
#include <array>
#include <functional>
#include "configurations/shared.hpp"
#include "joystick_info.hpp"

namespace siege
{
    struct environment_info
    {
        std::string_view config_dir;
        std::string_view content_dir;
        std::array<std::string_view, 4> root_dir_hints;
        std::unordered_map<std::string_view, std::string_view> file_hints;
    };


    struct game_config
    {
        using text_game_config = studio::configurations::text_game_config;
        using binary_game_config = studio::configurations::binary_game_config;

        inline game_config(std::filesystem::path path, text_game_config config)
        : path(std::move(path)), config(std::move(config))
        {
        }

        inline game_config(std::filesystem::path path, binary_game_config config)
        : path(std::move(path)), config(std::move(config))
        {
        }
        
        std::filesystem::path path;
        std::variant<text_game_config, binary_game_config> config;
    };

    struct game_info
    {
        std::string_view english_name;
        std::array<std::string_view, 2> supported_controller_types;
        joystick_info (&add_input_metadata)(joystick_info);
        joystick_info (&add_default_actions)(joystick_info);
        std::vector<game_config> (&create_game_configs)(std::vector<joystick_info>);
        std::array<std::function<game_info()>, 4> additional_runtimes;

        inline game_info(std::string_view english_name, 
                        std::string_view supported_controller_type, 
                        joystick_info (&add_input_metadata)(joystick_info),
                        joystick_info (&add_default_actions)(joystick_info),
                        std::vector<game_config> (&create_game_configs)(std::vector<joystick_info>))
              :  english_name(english_name),
                supported_controller_types({supported_controller_type}),
                add_input_metadata(add_input_metadata),
                add_default_actions(add_default_actions),
                create_game_configs(create_game_configs)
        {
        }

        inline game_info(std::string_view english_name, 
                        std::string_view supported_controller_type, 
                        joystick_info (&add_input_metadata)(joystick_info),
                        joystick_info (&add_default_actions)(joystick_info),
                        std::vector<game_config> (&create_game_configs)(std::vector<joystick_info>),
                        std::function<game_info()> create_additional_runtime)
              : game_info(english_name, supported_controller_type, add_input_metadata, add_default_actions, create_game_configs)
        {
            additional_runtimes[0] = create_additional_runtime;
        }

        inline game_info(const game_info& info, 
                        std::string_view english_name, 
                        joystick_info (&add_default_actions)(joystick_info))
              :  game_info(english_name, 
                         info.supported_controller_types[0],
                         info.add_input_metadata,
                         add_default_actions,
                         info.create_game_configs )
        {
        }
    };

    std::vector<game_info> get_supported_games();
    std::optional<environment_info> environment_for_game(const game_info&);
}


#endif