#ifndef OPEN_SIEGE_GAME_INFO_HPP
#define OPEN_SIEGE_GAME_INFO_HPP

#include <string_view>
#include <type_traits>
#include <variant>
#include <optional>
#include <filesystem>
#include <array>
#include <functional>
#include <siege/configuration/shared.hpp>
#include <siege/platform/joystick_info.hpp>

namespace siege
{
    struct game_config
    {
        using text_game_config = siege::configuration::text_game_config;
        using binary_game_config = siege::configuration::binary_game_config;

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

    struct environment_info_hint
    {
        std::optional<std::filesystem::path> working_dir;
        std::string_view config_dir;
        std::string_view exe_dir;
        std::array<std::string_view, 4> working_dir_hints;
        std::unordered_map<std::string_view, std::string_view> file_hints;
    };

    struct environment_info
    {
        std::filesystem::path working_dir;
        std::filesystem::path config_dir;
        std::filesystem::path exe_dir;
    };

    std::vector<game_info> get_supported_games();
    environment_info_hint environment_for_game(const game_info&);
    std::vector<std::filesystem::path> get_common_search_paths();
    std::vector<environment_info_hint> find_installed_game_hints(const std::vector<std::filesystem::path>&, environment_info_hint info);
    std::vector<environment_info> verity_game_hints(const std::vector<environment_info_hint>&);
}


#endif