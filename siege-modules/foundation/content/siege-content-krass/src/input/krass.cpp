#include <array>
#include <utility>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include <siege/configuration/game_info.hpp>
#include <siege/configuration/krass.hpp>

namespace siege
{
    using joystick_info = siege::joystick_info;
    using button = siege::button;
    using hat = siege::hat;
    using axis = siege::axis;
    using game_info = siege::game_info;
    using text_game_config = siege::configuration::text_game_config;

    button add_krass_button_metadata(button result)
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

    axis add_krass_axis_metadata(axis result)
    {
        constexpr static auto axis_names = std::array<std::string_view, 8> {{
            "AxisX",
            "AxisY",
            "AxisZ",
            "RotationX",
            "RotationY",
            "RotationZ",
            "Slider0",
            "Slider1"
        }};

        if (result.index < axis_names.size())
        {
            result.meta_name_positive.emplace(axis_names[result.index]);
            result.meta_name_negative.emplace(axis_names[result.index]);
        }

        return result;
    }

    hat add_krass_hat_metadata(hat result)
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

    joystick_info add_krass_input_metadata(joystick_info info)
    {
        std::transform(info.buttons.begin(), info.buttons.end(), info.buttons.begin(), add_krass_button_metadata);
        std::transform(info.axes.begin(), info.axes.end(), info.axes.begin(), add_krass_axis_metadata);
        std::transform(info.hats.begin(), info.hats.end(), info.hats.begin(), add_krass_hat_metadata);

        return info;
    }

    joystick_info add_krass_default_actions(joystick_info info)
    {
        return info;
    }

    std::vector<game_config> convert_to_krass_config(std::vector<joystick_info> joysticks)
    {
        std::vector<game_config> results;
        results.emplace_back("User.ini", siege::configuration::krass::save_config);
        return results;
    }

    std::vector<game_info> get_krass_games()
    {
       return {
        game_info {
                    "AquaNox", 
                    common::types::playstation,
                    add_krass_input_metadata, 
                    add_krass_default_actions, 
                    convert_to_krass_config
        },
        game_info {
                    "AquaNox 2: Revelation", 
                    common::types::playstation,
                    add_krass_input_metadata, 
                    add_krass_default_actions, 
                    convert_to_krass_config
        }
       };
    }

}