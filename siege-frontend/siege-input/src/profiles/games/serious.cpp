#include <array>
#include <utility>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include "joystick_info.hpp"

namespace siege
{

    using game_info = siege::game_info;
    using joystick_info = siege::joystick_info;
    using button = siege::button;
    using hat = siege::hat;
    using axis = siege::axis;

    button add_serious_button_metadata(button result)
    {
        constexpr static auto button_names = std::array<std::string_view, 16> {{
            "Joy 1 Button 0",
            "Joy 1 Button 1",
            "Joy 1 Button 2",
            "Joy 1 Button 3",
            "Joy 1 Button 4",
            "Joy 1 Button 5",
            "Joy 1 Button 6",
            "Joy 1 Button 7",
            "Joy 1 Button 8",
            "Joy 1 Button 9",
            "Joy 1 Button 10",
            "Joy 1 Button 11",
            "Joy 1 Button 12",
            "Joy 1 Button 13",
            "Joy 1 Button 14",
            "Joy 1 Button 15"
        }};

        if (result.index < button_names.size())
        {
            result.meta_name.emplace(button_names[result.index]);
        }

        return result;
    }

    axis add_serious_axis_metadata(axis result)
    {

        constexpr static auto axis_names = std::array<std::string_view, 6> {{
            "Joy 1 Axis X",
            "Joy 1 Axis Y",
            "Joy 1 Axis Z",
            "Joy 1 Axis R",
            "Joy 1 Axis U",
            "Joy 1 Axis Z"
        }};

        if (result.index < axis_names.size())
        {
            result.meta_name_positive.emplace(axis_names[result.index]);
            result.meta_name_negative.emplace(axis_names[result.index]);
        }

        return result;
    }

    hat add_serious_hat_metadata(hat result)
    {
        if (result.index == 0)
        {
            result.meta_name_up = "JoyPovUp";
            result.meta_name_down = "JoyPovDown";
            result.meta_name_right = "JoyPovRight";
            result.meta_name_left = "JoyPovLeft";
        }

        return result;
    }

    joystick_info add_serious_input_metadata(joystick_info info)
    {
        std::transform(info.buttons.begin(), info.buttons.end(), info.buttons.begin(), add_serious_button_metadata);
        std::transform(info.axes.begin(), info.axes.end(), info.axes.begin(), add_serious_axis_metadata);
        std::transform(info.hats.begin(), info.hats.end(), info.hats.begin(), add_serious_hat_metadata);

        return info;
    }

    joystick_info add_serious_default_actions(joystick_info info)
    {
        return info;
    }

    std::vector<game_config> convert_to_serious_config(joystick_info joystick)
    {
        text_game_config config(studio::configurations::id_tech::id_tech_2::save_config);
        return { game_config{ "autoexec.cfg", config } };
    }

    std::vector<game_info> get_serious_games()
    {
        constexpr static auto serious_sam = game_info {
                    "Serious Sam: The First Encounter", 
                    { common::types::playstation },
                    add_serious_input_metadata, 
                    add_serious_default_actions, 
                    convert_to_serious_config
        };

        return { 
            serious_sam,
            game_info { serious_sam, "Serious Sam: The Second Encounter", add_serious_default_actions },
            game_info { serious_sam, "Nitro Family", add_serious_default_actions },
            game_info { serious_sam, "Alpha Black Zero", add_serious_default_actions },
            game_info { serious_sam, "Euro Cops", add_serious_default_actions },
            game_info { serious_sam, "Carnivores: Cityscape", add_serious_default_actions },
        };
    }
}