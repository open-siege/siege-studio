#include <array>
#include <utility>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include "joystick_info.hpp"
#include "game_info.hpp"
#include <siege/configuration/jedi.hpp>

namespace siege
{
    using joystick_info = siege::joystick_info;
    using button = siege::button;
    using hat = siege::hat;
    using axis = siege::axis;
    using alias = siege::alias;
    using game_info = siege::game_info;
    using text_game_config = siege::configuration::text_game_config;

    button add_racer_button_metadata(button result)
    {
        constexpr static auto button_names = std::array<std::string_view, 10> {{
            "BUTTON=1",
            "BUTTON=2",
            "BUTTON=3",
            "BUTTON=4",
            "BUTTON=5",
            "BUTTON=6",
            "BUTTON=7",
            "BUTTON=8",
            "BUTTON=9",
            "BUTTON=10",
        }};

        if (result.index < button_names.size())
        {
            result.meta_name.emplace(button_names[result.index]);
        }

        return result;
    }

    axis add_racer_axis_metadata(axis result)
    {
        constexpr static auto axis_names = std::array<std::string_view, 6> {{
            "AXIS=X",
            "AXIS=Y ",
            "AXIS=Z",
            "AXIS=RX",
            "AXIS=RY",
            "AXIS=RZ"
        }};

        if (result.index < axis_names.size())
        {
            result.meta_name_positive.emplace(axis_names[result.index]);
            result.meta_name_negative.emplace(axis_names[result.index]);
        }

        return result;
    }

    hat add_racer_hat_metadata(hat result)
    {
        if (result.index == 0)
        {
            result.meta_name_up = "BUTTON=HAT_UP";
            result.meta_name_down = "BUTTON=HAT_DOWN";
            result.meta_name_right = "BUTTON=HAT_RIGHT";
            result.meta_name_left = "BUTTON=HAT_LEFT";
        }

        return result;
    }

    joystick_info add_racer_input_metadata(joystick_info info)
    {
        std::transform(info.buttons.begin(), info.buttons.end(), info.buttons.begin(), add_racer_button_metadata);
        std::transform(info.axes.begin(), info.axes.end(), info.axes.begin(), add_racer_axis_metadata);
        std::transform(info.hats.begin(), info.hats.end(), info.hats.begin(), add_racer_hat_metadata);

        return info;
    }

    joystick_info add_racer_default_actions(joystick_info info)
    {
        return info;
    }

    std::vector<game_config> convert_to_racer_config(std::vector<joystick_info> joysticks)
    {
        std::vector<game_config> results;
        results.emplace_back("User.ini", siege::configuration::jedi::racer::save_config);
        return results;
    }

    std::vector<game_info> get_star_wars_games()
    {
        return {
            game_info {
                "Star Wars Racer", 
                common::types::playstation,
                add_racer_input_metadata, 
                add_racer_default_actions, 
                convert_to_racer_config
            }
         };
    }
}