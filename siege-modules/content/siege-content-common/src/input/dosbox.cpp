#include <array>
#include <utility>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include <siege/configuration/game_info.hpp>
#include <siege/configuration/dosbox.hpp>

namespace siege
{
    using joystick_info = siege::joystick_info;
    using button = siege::button;
    using hat = siege::hat;
    using axis = siege::axis;
    using game_info = siege::game_info;
    using text_game_config = siege::configuration::text_game_config;

    button add_dosbox_button_metadata(button result)
    {
        constexpr static auto button_names = std::array<std::string_view, 16> {{
            "stick_0 button 0",
            "stick_0 button 1",
            "stick_0 button 2",
            "stick_0 button 3",
            "stick_0 button 4",
            "stick_0 button 5",
            "stick_0 button 6",
            "stick_0 button 7",
            "stick_0 button 8",
            "stick_0 button 9",
            "stick_0 button 10",
            "stick_0 button 11",
            "stick_0 button 12",
            "stick_0 button 13",
            "stick_0 button 14",
            "stick_0 button 15"
        }};

        if (result.index < button_names.size())
        {
            result.meta_name.emplace(button_names[result.index]);
        }

        return result;
    }

    axis add_dosbox_axis_metadata(axis result)
    {
        constexpr static auto axis_names_positive = std::array<std::string_view, 6> {{
            "stick_0 axis 0 1",
            "stick_0 axis 1 1",
            "stick_0 axis 2 1",
            "stick_0 axis 3 1",
            "stick_0 axis 4 1",
            "stick_0 axis 5 1"
        }};

        constexpr static auto axis_names_negative = std::array<std::string_view, 6> {{
            "stick_0 axis 0 0",
            "stick_0 axis 1 0",
            "stick_0 axis 2 0",
            "stick_0 axis 3 0",
            "stick_0 axis 4 0",
            "stick_0 axis 5 0"
        }};

        if (result.index < axis_names_positive.size())
        {
            result.meta_name_positive.emplace(axis_names_positive[result.index]);
        }

        if (result.index < axis_names_negative.size())
        {
            result.meta_name_negative.emplace(axis_names_negative[result.index]);
        }

        return result;
    }

    hat add_dosbox_hat_metadata(hat result)
    {
        if (result.index == 0)
        {
            result.meta_name_up = "stick_0 hat 0 0 1";
            result.meta_name_down = "stick_0 hat 0 4";
            result.meta_name_right = "stick_0 hat 0 2";
            result.meta_name_left = "stick_0 hat 0 8";
        }

        return result;
    }

    joystick_info add_dosbox_input_metadata(joystick_info info)
    {
        std::transform(info.buttons.begin(), info.buttons.end(), info.buttons.begin(), add_dosbox_button_metadata);
        std::transform(info.axes.begin(), info.axes.end(), info.axes.begin(), add_dosbox_axis_metadata);
        std::transform(info.hats.begin(), info.hats.end(), info.hats.begin(), add_dosbox_hat_metadata);

        return info;
    }

    joystick_info add_dosbox_default_actions(joystick_info info)
    {
        return info;
    }

    std::vector<game_config> convert_to_dosbox_config(std::vector<joystick_info> joysticks)
    {
        std::vector<game_config> results;
        
        results.emplace_back("dosbox.cfg", text_game_config(siege::configuration::dosbox::config::save_config));
        results.emplace_back("mapper.cfg", text_game_config(siege::configuration::dosbox::mapper::save_config));

        return results;
    }

    joystick_info as_is(joystick_info info)
    {
        return info;
    }

    std::vector<game_config> as_is(std::vector<joystick_info> joysticks)
    {
        return std::vector<game_config>{};
    }


    std::vector<game_info> get_dosbox_games()
    {
        const static auto dosbox = game_info {
                    "DOSBox 0.74-3", 
                    common::types::playstation,
                    add_dosbox_input_metadata, 
                    add_dosbox_default_actions, 
                    convert_to_dosbox_config
        };

        auto get_dosbox = []{ return dosbox; };

        return {
            game_info {"1942: The Pacific Air War", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Across the Rhine", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"An Elder Scrolls Legend: Battlespire", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Archimedean Dynasty", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Blake Stone: Aliens of Gold", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Blake Stone: Planet Strike", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Carmageddon", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Catacombs", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Corridor 7: Alien Invasion", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Cyclemania", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"CyClones", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Descent", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Descent 2", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Discoveries of the Deep", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Doom", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Doom II", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Eradicator", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"F-117A Nighthawk Stealth Fighter 2.0", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"F-19 Stealth Fighter", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Falcon", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Falcon A.T.", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Final Doom", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Fleet Defender: The F-14 Tomcat Simulation", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Heart of China", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Hexen: Beyond Heretic", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Hexen: Deathkings of the Dark Citadel", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Last Rites", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Operation Body Count", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Powerslave", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Privateer 2", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Quake", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Rayman Forever", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Realms of the Haunting", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Red Baron", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Redneck Rampage", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Rise of the Dragon", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Rise of the Triad", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Shadow Warrior Classic Complete", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Shattered Steel", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Silent Service", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Silent Service 2", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Slipstream 5000", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Star Wars Tie Fighter (1994)", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Star Wars X-Wing (1993)", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Star Wars Dark Forces", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Star Gunner", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Strike Commander", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Stunt Island", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Subwar 2050", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"System Shock Classic", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Terminal Velocity Legacy", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Terra Nova: Strike Force Centauri", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Wing Commander", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Wing Commander II", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Wing Commander Privateer", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Wing Commander 3 Heart of the Tiger", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Wing Commander 4 The Price of Freedom", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Wing Commander Academy", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Wing Commander Armada", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Witchaven", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Witchaven II: Blood Vengeance", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Wolfenstein 3D", common::types::playstation, as_is, as_is, as_is, get_dosbox },
            game_info {"Wolfenstein: Spear of Destiny", common::types::playstation, as_is, as_is, as_is, get_dosbox }
        };
    }

}