#include <array>
#include <utility>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include "joystick_info.hpp"
#include "game_info.hpp"
#include "configurations/id_tech.hpp"

namespace siege
{
    // movement
    constexpr static auto forward_on = "+forward"; 
    constexpr static auto backward_on = "+back";
    constexpr static auto move_left_on = "+moveleft"; // strafe left
    constexpr static auto move_right_on = "+moveright"; // strafe right
    constexpr static auto move_up_on = "+moveup"; // jump
    constexpr static auto move_down_on = "+movedown"; // crouch
    constexpr static auto speed_on = "+speed"; // walk/run

    // aiming
    constexpr static auto look_up_on = "+lookup"; 
    constexpr static auto look_down_on = "+lookdown"; 
    constexpr static auto look_left_on = "+left"; // turn left
    constexpr static auto look_right_on = "+right"; // turn right

    // shooting
    constexpr static auto attack_on = "+attack";
    constexpr static auto weapon_next = "weapnext";
    constexpr static auto weapon_previous = "weaprev";


    constexpr static auto melee_attack_on = "+melee-attack";

    namespace playstation = siege::playstation;
    using joystick_info = siege::joystick_info;
    using button = siege::button;
    using hat = siege::hat;
    using axis = siege::axis;
    using game_info = siege::game_info;
    using text_game_config = studio::configurations::text_game_config;

    constexpr static auto idtech_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 14> {{
            {"left_y+", forward_on },
            {"left_y-", backward_on },
            {"left_x+", move_left_on },
            {"left_x-", move_right_on },
            {"right_y+", look_up_on },
            {"right_y-", look_down_on },
            {"right_x+", look_right_on },
            {"right_x-", look_left_on },
            {playstation::r2, attack_on },
            {playstation::l3, speed_on },
            {playstation::r3, melee_attack_on },
            {playstation::triangle, weapon_next},
            {playstation::circle, move_down_on},
            {playstation::cross, move_up_on}
    }};


    template<std::size_t ArraySize>
    joystick_info add_actions_to_joystick_info(const std::array<std::array<std::string_view, 2>, ArraySize>& defaults, joystick_info info)
    {
        for (auto& [input, game_action] : defaults)
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

        return std::move(info);
    }

    joystick_info add_quake_1_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_hexen_2_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_quake_2_default_actions(joystick_info info)
    {
        constexpr static auto quake2_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 6> {{
            {playstation::l2, "invuse" },
            {playstation::l1, "invnext" },
            {playstation::r1, "+throw-grenade"},
            {playstation::square, "inven"},
            {playstation::start, "cmd help"},
            {playstation::select, "score"} 
        }};

        return add_actions_to_joystick_info(quake2_dual_stick_defaults, add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info)));
    }

    joystick_info add_soldier_of_fortune_default_actions(joystick_info info)
    {
        constexpr static auto sof_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 6> {{
            {playstation::l2, "+altattack" },
            {playstation::l1, "itemnext" },
            {playstation::r1, "itemuse"},
            {playstation::square, "+use-plus-special"},
            {playstation::start, "menu objectives"},
            {playstation::select, "score"},
        }};

        return add_actions_to_joystick_info(sof_dual_stick_defaults, add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info)));
    }

    joystick_info add_kingpin_default_actions(joystick_info info)
    {
        constexpr static auto kingpin_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 10> {{
            {playstation::l2, "holster" },
            {playstation::l1, "invnext" },
            {playstation::r1, "invuse"},
            {playstation::square, "+activate"},
            {playstation::start, "cmd help"},
            {playstation::select, "inven"},
            {playstation::d_pad_up, "flashlight"},
            {playstation::d_pad_down, "key2"},
            {playstation::d_pad_left, "key1"},
            {playstation::d_pad_right, "key3"}
        }};

        return add_actions_to_joystick_info(kingpin_dual_stick_defaults, add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info)));
    }

    joystick_info add_sin_default_actions(joystick_info info)
    {
        constexpr static auto sin_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 6> {{
            {playstation::l2, "weaponuse" },
            {playstation::l1, "invnext" },
            {playstation::r1, "invuse"},
            {playstation::square, "+use"},
            {playstation::start, "showinfo"},
            {playstation::select, "toggleviewmode"} 
        }};

        return add_actions_to_joystick_info(sin_dual_stick_defaults, add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info)));
    }

    joystick_info add_heretic_2_default_actions(joystick_info info)
    {
        constexpr static auto heretic2_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 10> {{
            {playstation::l2, "+defend" },
            {playstation::l1, "defprev" },
            {playstation::r1, "defnext"},
            {playstation::circle, "+creep"},
            {playstation::square, "+action"},
            {playstation::start, "menu_objectives"},
            {playstation::select, "menu_city_map"} 
        }};

        return add_actions_to_joystick_info(heretic2_dual_stick_defaults, add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info)));
    }

    joystick_info add_quake_3_default_actions(joystick_info info)
    {
        constexpr static auto quake3_dual_stick_defaults = std::array<std::array<std::string_view, 2>, 7> {{
            {playstation::l2, "+zoom" },
            {playstation::l1, "weapprev" },
            {playstation::r1, "weapnext" },
            {playstation::triangle, "vote yes" },
            {playstation::square, "vote no"},
            {playstation::start, "pause"},
            {playstation::select, "+scores"},
        }};
        return add_actions_to_joystick_info(quake3_dual_stick_defaults, add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info)));
    }

    joystick_info add_quake_1_input_metadata(joystick_info info);
    joystick_info add_quake_2_input_metadata(joystick_info info);
    joystick_info add_quake_3_input_metadata(joystick_info info);
    game_info::config_info convert_to_quake_config(joystick_info joystick);
    game_info::config_info convert_to_quake_2_config(joystick_info joystick);
    game_info::config_info convert_to_quake_3_config(joystick_info joystick);

    std::vector<game_info> get_id_tech_games()
    {
        return {
            game_info {"Quake", add_quake_1_input_metadata, add_quake_1_default_actions, convert_to_quake_config },
            game_info {"battleMETAL", add_quake_1_input_metadata, add_quake_1_default_actions, convert_to_quake_config },
            game_info {"Hexen II", add_quake_2_input_metadata, add_hexen_2_default_actions, convert_to_quake_config },
            game_info {"Quake II", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Heretic 2", add_quake_2_input_metadata, add_heretic_2_default_actions, convert_to_quake_2_config},
            game_info {"SiN", add_quake_2_input_metadata, add_sin_default_actions, convert_to_quake_2_config },
            game_info {"Kingpin: Life of Crime", add_quake_2_input_metadata, add_kingpin_default_actions, convert_to_quake_2_config },
            game_info {"Daikatana", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"AQtion", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Soldier of Fortune", add_quake_2_input_metadata, add_soldier_of_fortune_default_actions, convert_to_quake_2_config },
            game_info {"Half-Life", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Half-Life: Opposing Force", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Half-Life: Blue Shift", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Counter-Strike", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Counter-Strike: Condition Zero", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Counter-Strike: Condition Zero Deleted Scenes", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Cry of Fear", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Day of Defeat", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Deathmatch Classic", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Ricochet", add_quake_2_input_metadata, add_quake_2_default_actions, convert_to_quake_2_config },
            game_info {"Quake III Arena", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Quake Live", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Iron Grip: Warlord", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Star Trek: Voyager - Elite Force", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Star Trek: Elite Force II", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Star Wars: Jedi Knight II - Jedi Outcast", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Star Wars: Jedi Knight - Jedi Academy", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Return to Castle Wolfenstein", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Soldier of Fortune II: Double Helix", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Medal of Honor: Allied Assault", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Call of Duty", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config },
            game_info {"Space Trader: Merchant Marine", add_quake_3_input_metadata, add_quake_3_default_actions, convert_to_quake_3_config }
        };
    }

    std::string_view find_axis_index_for_action(std::string_view action)
    {
        constexpr static auto controller_button_mapping = std::array<std::array<std::string_view, 2>, 10> {{
            { forward_on, "1" },
            { backward_on, "1" },
            { look_up_on, "2" },
            { look_down_on, "2" },
            { move_left_on, "3" },
            { move_right_on, "3" },
            { look_left_on, "4" },
            { look_right_on, "4" },
            { move_up_on, "5" },
            { move_down_on, "5" }
        }};


        auto existing_mapping = std::find_if(controller_button_mapping.begin(), controller_button_mapping.end(), [&](auto& mapping) {
            return mapping[0] == action;
        });

        if (existing_mapping != controller_button_mapping.end())
        {
            return (*existing_mapping)[1];
        }

        return "0";
    }

    game_info::config_info convert_to_quake_config(joystick_info joystick)
    {
        text_game_config config;

        for (auto& button : joystick.buttons)
        {
            for (auto& action : button.actions)
            {
                config.emplace({"bind", action.target_meta_name}, action.name);
            }
        }

        for (auto& hat : joystick.hats)
        {
            for (auto& action : hat.actions)
            {
                config.emplace({"bind", action.target_meta_name}, action.name);
            }
        }

        for (auto& axis : joystick.axes)
        {
            for (auto& action : axis.actions)
            {
                config.emplace(action.target_meta_name, find_axis_index_for_action(action.name));
            }
        }

        return game_info::text_config { std::move(config), studio::configurations::id_tech::id_tech_2::load_config, studio::configurations::id_tech::id_tech_2::save_config };
    }

    game_info::config_info convert_to_quake_2_config(joystick_info joystick)
    {
        text_game_config config;

        for (auto& button : joystick.buttons)
        {
            for (auto& action : button.actions)
            {
                config.emplace({"bind", action.target_meta_name}, action.name);
            }
        }

        for (auto& hat : joystick.hats)
        {
            for (auto& action : hat.actions)
            {
                config.emplace({"bind", action.target_meta_name}, action.name);
            }
        }

        for (auto& axis : joystick.axes)
        {
            for (auto& action : axis.actions)
            {
                config.emplace({"set", action.target_meta_name}, find_axis_index_for_action(action.name));
            }
        }

        return game_info::text_config { std::move(config), studio::configurations::id_tech::id_tech_2::load_config, studio::configurations::id_tech::id_tech_2::save_config };
    }

    game_info::config_info convert_to_quake_3_config(joystick_info joystick)
    {
        text_game_config config;

        for (auto& button : joystick.buttons)
        {
            for (auto& action : button.actions)
            {
                config.emplace({"bind", action.target_meta_name}, action.name);
            }
        }

        for (auto& hat : joystick.hats)
        {
            for (auto& action : hat.actions)
            {
                config.emplace({"bind", action.target_meta_name}, action.name);
            }
        }

        for (auto& axis : joystick.axes)
        {
            for (auto& action : axis.actions)
            {
                config.emplace({"bind", action.target_meta_name}, action.name);
            }
        }

        return game_info::text_config { std::move(config), studio::configurations::id_tech::id_tech_2::load_config, studio::configurations::id_tech::id_tech_2::save_config };
    }
}