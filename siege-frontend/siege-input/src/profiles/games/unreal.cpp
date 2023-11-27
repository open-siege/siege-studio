#include <array>
#include <utility>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include "joystick_info.hpp"
#include "game_info.hpp"
#include "configurations/unreal.hpp"

namespace siege
{
    // movement
    constexpr static auto move = "Axis aBaseY"; //forward/backward
    constexpr static auto strafe = "Axis aStrafe";
    constexpr static auto jump = "Axis aUp"; // jump/crouch
    constexpr static auto run = "Button bRun"; // walk/run

    // aiming
    constexpr static auto vertical_look = "Axis aBaseY"; 
    constexpr static auto turn = "Axis aBaseX";

    // shooting
    constexpr static auto attack = "Button bFire";
    constexpr static auto alt_attack = "Button bAltFire";
    constexpr static auto weapon_next = "NextWeapon";

    constexpr static auto melee_attack = "SwitchWeapon 1";


    constexpr static auto unreal_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 15> {{
            {"left_y+", move },
            {"left_y-", move },
            {"left_x+", strafe },
            {"left_x-", strafe },
            {"right_y+", vertical_look },
            {"right_y-", vertical_look },
            {"right_x+", turn },
            {"right_x-", turn },
            {playstation::l2, attack },
            {playstation::r2, alt_attack },
            {playstation::l3, run },
            {playstation::r3, melee_attack },
            {playstation::triangle, weapon_next},
            {playstation::circle, jump},
            {playstation::cross, jump}
    }};

    using joystick_info = siege::joystick_info;
    using button = siege::button;
    using hat = siege::hat;
    using axis = siege::axis;
    using game_info = siege::game_info;
    using text_game_config = studio::configurations::text_game_config;
    using game_info = siege::game_info;

    button add_unreal_button_metadata(button result)
    {
        constexpr static auto button_names = std::array<std::string_view, 16> {{
            "Joy1",
            "Joy2",
            "Joy3",
            "Joy4",
            "Joy5",
            "Joy6",
            "Joy7",
            "Joy8",
            "Joy9",
            "Joy10",
            "Joy11",
            "Joy12",
            "Joy13",
            "Joy14",
            "Joy15",
            "Joy16"
        }};

        if (result.index < button_names.size())
        {
            result.meta_name.emplace(button_names[result.index]);
        }

        return result;
    }

    axis add_unreal_axis_metadata(axis result)
    {
        constexpr static auto axis_names = std::array<std::string_view, 6> {{
            "JoyX",
            "JoyY",
            "JoyZ",
            "JoyR",
            "JoyU",
            "JoyV"
        }};

        if (result.index < axis_names.size())
        {
            result.meta_name_positive.emplace(axis_names[result.index]);
            result.meta_name_negative.emplace(axis_names[result.index]);
        }

        return result;
    }

    hat add_unreal_hat_metadata(hat result)
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

    joystick_info add_unreal_input_metadata(joystick_info info)
    {
        std::transform(info.buttons.begin(), info.buttons.end(), info.buttons.begin(), add_unreal_button_metadata);
        std::transform(info.axes.begin(), info.axes.end(), info.axes.begin(), add_unreal_axis_metadata);
        std::transform(info.hats.begin(), info.hats.end(), info.hats.begin(), add_unreal_hat_metadata);

        return info;
    }

    joystick_info add_unreal_default_actions(joystick_info info)
    {
        for (auto& [input, game_action] : unreal_dual_stick_defaults)
        {
            if (input.rfind('+') == input.size() - 1)
            {
                auto real_input = input.substr(0, input.size() - 1);
                auto axis_iter = std::find_if(info.axes.begin(), info.axes.end(), [&](const auto& axis) {
                    return axis.axis_type.has_value() && axis.meta_name_positive.has_value() && axis.axis_type.value() == real_input;
                });

                if (axis_iter != info.axes.end())
                {
                    auto& controller_action = axis_iter->actions.emplace_back();
                    controller_action.name = game_action;
                    controller_action.target_meta_name = axis_iter->meta_name_positive.value();
                }
                continue;
            }

            if (input.rfind('-') == input.size() - 1)
            {
                auto real_input = input.substr(0, input.size() - 1);
                auto axis_iter = std::find_if(info.axes.begin(), info.axes.end(), [&](const auto& axis) {
                    return axis.axis_type.has_value() && axis.meta_name_negative.has_value() && axis.axis_type.value() == real_input;
                });

                if (axis_iter != info.axes.end())
                {
                    auto& controller_action = axis_iter->actions.emplace_back();
                    controller_action.name = game_action;
                    controller_action.target_meta_name = axis_iter->meta_name_negative.value();
                }
                continue;
            }

            auto button_iter = std::find_if(info.buttons.begin(), info.buttons.end(), [input](const auto& button) {
                    return button.button_type.has_value() && button.meta_name.has_value() && button.button_type.value() == input;
            });

            if (button_iter != info.buttons.end())
            {
                auto& controller_action = button_iter->actions.emplace_back();
                controller_action.name = game_action;
                controller_action.target_meta_name = button_iter->meta_name.value();
            }
        }

        return info;
    }

    std::vector<game_config> convert_to_unreal_config(joystick_info joystick)
    {
        text_game_config config(studio::configurations::unreal::unreal_1::save_config);

        for (auto& button : joystick.buttons)
        {
            for (auto& action : button.actions)
            {
                config.emplace({"[Engine.Input]", action.target_meta_name}, action.name);
            }
        }

        for (auto& hat : joystick.hats)
        {
            for (auto& action : hat.actions)
            {
                config.emplace({"[Engine.Input]", action.target_meta_name}, action.name);
            }
        }

        for (auto& axis : joystick.axes)
        {
            for (auto& action : axis.actions)
            {
                config.emplace({"[Engine.Input]", action.target_meta_name}, action.name);
            }
        }

        return { game_config{ "User.ini", config } };
    }

    std::vector<game_info> get_unreal_games()
    {
        constexpr static auto unreal = game_info {
                    "Unreal", 
                    { common::types::playstation },
                    add_unreal_input_metadata, 
                    add_unreal_default_actions, 
                    convert_to_unreal_config 
        };

        constexpr static auto unreal_tournament_2003 = game_info {
                    "Unreal Tournament 2003", 
                    { common::types::playstation },
                    add_unreal_input_metadata, 
                    add_unreal_default_actions, 
                    convert_to_unreal_config 
        };

        constexpr static auto unreal_tournament_3 = game_info {
                    "Unreal Tournament 3", 
                    { common::types::xbox, common::types::playstation },
                    add_unreal_input_metadata, 
                    add_unreal_default_actions, 
                    convert_to_unreal_config
        };

        return {
            unreal,
            game_info {unreal, "Star Trek: The Next Generation: Klingon Honor Guard", add_unreal_default_actions},
            game_info {unreal, "TNN Outdoors Pro Hunter", add_unreal_default_actions},
            game_info {unreal, "Dr. Brain: Action Reaction", add_unreal_default_actions},
            game_info {unreal, "Nerf Arena Blast", add_unreal_default_actions},
            game_info {unreal, "The Wheel of Time", add_unreal_default_actions},
            game_info {unreal, "Unreal Tournament", add_unreal_default_actions},
            game_info {unreal, "Deus Ex", add_unreal_default_actions},
            game_info {unreal, "Rune", add_unreal_default_actions},
            game_info {unreal, "Star Trek: Deep Space Nine: The Fallen", add_unreal_default_actions},
            game_info {unreal, "Adventure Pinball: Forgotten Island", add_unreal_default_actions},
            game_info {unreal, "Clive Barker's Undying", add_unreal_default_actions},
            game_info {unreal, "Harry Potter and the Philosopher's Stone", add_unreal_default_actions},
            game_info {unreal, "X-COM: Enforcer", add_unreal_default_actions},
            game_info {unreal, "Harry Potter and the Chamber of Secrets", add_unreal_default_actions},
            game_info {unreal, "Mobile Forces", add_unreal_default_actions},
            game_info {unreal, "Tactical Ops: Assault on Terror", add_unreal_default_actions},
            game_info {unreal, "Disney's Brother Bear", add_unreal_default_actions},
            unreal_tournament_2003,
            game_info {unreal_tournament_2003, "Tom Clancy's Splinter Cell", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Deus Ex: Invisible War", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Unreal II: The Awakening", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "XIII", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Dead Man's Hand", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Harry Potter and the Prisoner of Azkaban", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Lemony Snicket's A Series of Unfortunate Events", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Men of Valor: Vietnam", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Shadow Ops: Red Mercury", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Shark Tale", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Shrek 2", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Unreal Tournament 2004", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Pariah", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Tom Clancy's Splinter Cell: Pandora Tomorrow", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Star Wars: Republic Commando", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "SWAT 4", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Tom Clancy's Splinter Cell: Chaos Theory", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Red Orchestra: Ostfront 41-45", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Tom Clancy's Splinter Cell: Double Agent", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Warpath", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Killing Floor", add_unreal_default_actions},
            game_info {unreal_tournament_2003, "Tom Clancy's Splinter Cell: Conviction", add_unreal_default_actions},
            unreal_tournament_3,
            game_info {unreal_tournament_3, "Damnation", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Frontlines: Fuel of War", add_unreal_default_actions},
            game_info {unreal_tournament_3, "X-Men Origins: Wolverine", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Enslaved: Odyssey to the West", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Singularity", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Homefront", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Red Orchestra 2: Heroes of Stalingrad", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Rising Storm", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Chivalry: Medieval Warfare", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Tribes: Ascend", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Batman: Arkham Origins", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Brothers: A Tale of Two Sons", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Killing Floor 2", add_unreal_default_actions},
            // unreal engine 4
            game_info {unreal_tournament_3, "Fighting EX Layer", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Mirage: Arcane Warfare", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Dahalo", add_unreal_default_actions},
            game_info {unreal_tournament_3, "NBA Playgrounds", add_unreal_default_actions},
            game_info {unreal_tournament_3, "Stray", add_unreal_default_actions}
        };
    }
}