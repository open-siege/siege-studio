#include <array>
#include <utility>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include <siege/configuration/game_info.hpp>
#include <siege/configuration/id_tech.hpp>

namespace siege
{
    using namespace std::literals;
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

    using joystick_info = siege::joystick_info;
    using button = siege::button;
    using hat = siege::hat;
    using axis = siege::axis;
    using alias = siege::alias;
    using game_info = siege::game_info;
    using text_game_config = siege::configuration::text_game_config;

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
    joystick_info add_aliases_to_joystick_info(const std::array<std::array<std::string_view, 2>, ArraySize>& defaults, joystick_info info)
    {
        info.aliases.reserve(ArraySize);

        std::transform(defaults.begin(), defaults.end(), std::back_inserter(info.aliases), [](const auto& values) { return alias{values[0], values[1]}; });
        return info;
    }

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

        return info;
    }

    joystick_info add_quake_1_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_battlemetal_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_hexen_2_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_half_life_default_actions(joystick_info info)
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

        constexpr static auto quake2_aliases = std::array<std::array<std::string_view, 2>, 2> {{
                {"+throw-grenade", "use grenades; +attack"},
                {"-throw-grenade", "-weapprev; -attack"}
        }};

        return add_aliases_to_joystick_info(quake2_aliases, 
                    add_actions_to_joystick_info(quake2_dual_stick_defaults, 
                        add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info))));
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

        constexpr static auto sof_aliases = std::array<std::array<std::string_view, 2>, 8> {{
                {"+melee-attack", "weaponselect 1; +attack"},
                {"-melee-attack", "weaponbestsafe; -attack"},
                {"+use-plus-special1", "+weaponextra1; +use"},
                {"-use-plus-special1", "-weaponextra1; -use; alias +use-plus-special +use-plus-special2; alias -use-plus-special -use-plus-special2"},
                {"+use-plus-special2", "+weaponextra2; +use"},
                {"-use-plus-special2", "-weaponextra2; -use; alias +use-plus-special +use-plus-special1; alias -use-plus-special -use-plus-special1"},
                {"+use-plus-special", "+use-plus-special1"},
                {"-use-plus-special", "-use-plus-special1"}
        }};

        return add_aliases_to_joystick_info(sof_aliases, 
                    add_actions_to_joystick_info(sof_dual_stick_defaults, 
                        add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info))));
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

        constexpr static auto kingpin_aliases = std::array<std::array<std::string_view, 2>, 2> {{
                {"+melee-attack", "use pipe; +attack"},
                {"-melee-attack", "weapprev; -attack"}
        }};

        return add_aliases_to_joystick_info(kingpin_aliases, 
                    add_actions_to_joystick_info(kingpin_dual_stick_defaults, 
                        add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info))));
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

        constexpr static auto sin_aliases = std::array<std::array<std::string_view, 2>, 2> {{
                {"+melee-attack", "use Fists; +attack"},
                {"-melee-attack", "weapprev; -attack"}
        }};

        return add_aliases_to_joystick_info(sin_aliases, 
                    add_actions_to_joystick_info(sin_dual_stick_defaults, 
                        add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info))));
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

        constexpr static auto heretic2_aliases = std::array<std::array<std::string_view, 2>, 2> {{
                {"+melee-attack", "use staff; +attack"},
                {"-melee-attack", "weapprev; -attack"}
        }};

        return add_aliases_to_joystick_info(heretic2_aliases, 
                    add_actions_to_joystick_info(heretic2_dual_stick_defaults, 
                        add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info))));
    }

    joystick_info add_daikatana_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_action_quake_2_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_space_trader_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_call_of_duty_default_action(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_medal_of_honor_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_soldier_of_fortune_2_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_castle_wolfenstein_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_jedi_academy_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_jedi_outcast_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_elite_force_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_elite_force_2_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
    }

    joystick_info add_iron_grip_default_actions(joystick_info info)
    {
        return add_actions_to_joystick_info(idtech_dual_stick_defaults, info);
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

        constexpr static auto quake3_aliases = std::array<std::array<std::string_view, 2>, 2> {{
                {"+melee-attack", "weapon 1; +attack"},
                {"-melee-attack", "weapnext; -attack"}
        }};

        return add_aliases_to_joystick_info(quake3_aliases, 
                    add_actions_to_joystick_info(quake3_dual_stick_defaults, 
                        add_actions_to_joystick_info(idtech_dual_stick_defaults, std::move(info))));
    }

    joystick_info add_quake_1_input_metadata(joystick_info info);
    joystick_info add_quake_2_input_metadata(joystick_info info);
    joystick_info add_quake_3_input_metadata(joystick_info info);
    std::vector<game_config> convert_to_quake_config(std::vector<joystick_info> joystick);
    std::vector<game_config> convert_to_quake_2_config(std::vector<joystick_info> joystick);
    std::vector<game_config> convert_to_quake_3_config(std::vector<joystick_info> joystick);

    std::vector<game_info> get_id_tech_games()
    {
        const static auto quake = game_info {
                    "Quake"sv, 
                    common::types::playstation,
                    add_quake_1_input_metadata, 
                    add_quake_1_default_actions, 
                    convert_to_quake_config
        };

        const static auto quake_2 = game_info {
                "Quake II"sv, 
                common::types::playstation,
                add_quake_2_input_metadata, 
                add_quake_2_default_actions,
                convert_to_quake_2_config
        };

        const static auto quake_3 = game_info {
                "Quake III Arena"sv, 
                common::types::playstation,
                add_quake_3_input_metadata, 
                add_quake_3_default_actions, 
                convert_to_quake_3_config  
        };

        const static auto half_life = game_info {
                    "Half-Life"sv, 
                    common::types::playstation,
                    add_quake_2_input_metadata, 
                    add_half_life_default_actions, 
                    convert_to_quake_2_config
        };

        return {
            quake,
            game_info {quake, "battleMETAL"sv, add_battlemetal_default_actions},
            game_info {quake, "Hexen II"sv, add_hexen_2_default_actions},
            quake_2,
            game_info {quake_2, "Heretic 2"sv, add_heretic_2_default_actions},
            game_info {quake_2, "SiN"sv, add_sin_default_actions},
            game_info {quake_2, "Kingpin: Life of Crime"sv, add_kingpin_default_actions},
            game_info {quake_2, "Daikatana"sv, add_daikatana_default_actions},
            game_info {quake_2, "AQtion"sv, add_action_quake_2_default_actions},
            game_info {quake_2, "Soldier of Fortune"sv, add_soldier_of_fortune_default_actions},
            half_life,
            game_info {half_life, "Half-Life: Opposing Force"sv, add_half_life_default_actions},
            game_info {half_life, "Half-Life: Blue Shift"sv, add_half_life_default_actions},
            game_info {half_life, "Counter-Strike"sv, add_half_life_default_actions},
            game_info {half_life, "Counter-Strike: Condition Zero"sv, add_half_life_default_actions},
            game_info {half_life, "Counter-Strike: Condition Zero Deleted Scenes"sv, add_half_life_default_actions},
            game_info {half_life, "Cry of Fear"sv, add_half_life_default_actions},
            game_info {half_life, "Day of Defeat"sv, add_half_life_default_actions},
            game_info {half_life, "Deathmatch Classic"sv, add_half_life_default_actions},
            game_info {half_life, "Ricochet"sv, add_half_life_default_actions},
            quake_3,
            game_info {quake_3, "Quake Live"sv, add_quake_3_default_actions},
            game_info {quake_3, "Iron Grip: Warlord"sv, add_iron_grip_default_actions},
            game_info {quake_3, "Star Trek: Voyager - Elite Force"sv, add_elite_force_default_actions},
            game_info {quake_3, "Star Trek: Elite Force II"sv, add_elite_force_2_default_actions},
            game_info {quake_3, "Star Wars: Jedi Knight II - Jedi Outcast"sv, add_jedi_outcast_default_actions},
            game_info {quake_3, "Star Wars: Jedi Knight - Jedi Academy"sv, add_jedi_academy_default_actions},
            game_info {quake_3, "Return to Castle Wolfenstein"sv, add_castle_wolfenstein_default_actions},
            game_info {quake_3, "Soldier of Fortune II: Double Helix"sv, add_soldier_of_fortune_2_default_actions},
            game_info {quake_3, "Medal of Honor: Allied Assault"sv, add_medal_of_honor_default_actions},
            game_info {quake_3, "Call of Duty"sv, add_call_of_duty_default_action},
            game_info {quake_3, "Space Trader: Merchant Marine"sv, add_space_trader_default_actions}
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

    std::vector<game_config> convert_to_quake_config(std::vector<joystick_info> joysticks)
    {
        std::vector<game_config> results;
        results.reserve(joysticks.size() + 1);

        results.emplace_back("autoexec.cfg", [&] () {
            text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

            config.emplace("exec"sv, "joystick.cfg"sv);

            return config;
        }());

        std::transform(joysticks.begin(), joysticks.end(), std::back_inserter(results), [](auto& joystick) {
            constexpr static auto extension = ".cfg"sv;
            std::string file_name;
            file_name.reserve(joystick.name.size() + extension.size());
            file_name.assign(joystick.name);
            file_name.append(extension);
            return game_config{ file_name, [&] () {
            text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

            auto create_bind = [&] (const auto& action) {
                config.emplace({"bind", action.target_meta_name}, action.name);
            };

            std::for_each(joystick.buttons.cbegin(), joystick.buttons.cend(), [&] (const auto& button) {
                std::for_each(button.actions.cbegin(), button.actions.cend(), create_bind);
            });

            std::for_each(joystick.hats.cbegin(), joystick.hats.cend(), [&] (const auto& hat) {
                std::for_each(hat.actions.cbegin(), hat.actions.cend(), create_bind);
            });

            config.emplace({"joyadvanced"}, "1"sv);

            std::for_each(joystick.axes.cbegin(), joystick.axes.cend(), [&] (const auto& axis) {
                std::for_each(axis.actions.cbegin(), axis.actions.cend(), [&](const auto& action) {
                    config.emplace(action.target_meta_name, find_axis_index_for_action(action.name));
                });
            });

            return config;
        }() };
        });

        return results;
    }

    std::vector<game_config> convert_to_quake_2_config(std::vector<joystick_info> joysticks)
    {
        std::vector<game_config> results;
        results.reserve(joysticks.size() + 1);

        results.emplace_back("autoexec.cfg", [&] () {
            text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

            config.emplace("exec"sv, "joystick.cfg"sv);

            return config;
        }());

        std::transform(joysticks.begin(), joysticks.end(), std::back_inserter(results), [](auto& joystick) {
            constexpr static auto extension = ".cfg"sv;
            std::string file_name;
            file_name.reserve(joystick.name.size() + extension.size());
            file_name.assign(joystick.name);
            file_name.append(extension);
            return game_config{ file_name, [&] () {
            text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

            auto create_bind = [&] (const auto& action) {
                config.emplace({"bind", action.target_meta_name}, action.name);
            };

            std::for_each(joystick.buttons.cbegin(), joystick.buttons.cend(), [&] (const auto& button) {
                std::for_each(button.actions.cbegin(), button.actions.cend(), create_bind);
            });

            std::for_each(joystick.hats.cbegin(), joystick.hats.cend(), [&] (const auto& hat) {
                std::for_each(hat.actions.cbegin(), hat.actions.cend(), create_bind);
            });

            config.emplace({"set", "joy_advanced"}, "1"sv);

            std::for_each(joystick.axes.cbegin(), joystick.axes.cend(), [&] (const auto& axis) {
                std::for_each(axis.actions.cbegin(), axis.actions.cend(), [&](const auto& action) {
                    config.emplace({"set", action.target_meta_name}, find_axis_index_for_action(action.name));
                });
            });

            return config;
        }() };
        });

        return results;
    }

    std::vector<game_config> convert_to_quake_3_config(std::vector<joystick_info> joysticks)
    {
        std::vector<game_config> results;
        results.reserve(joysticks.size() + 1);

        results.emplace_back("autoexec.cfg", [&] () {
            text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

            config.emplace("exec"sv, "joystick.cfg"sv);

            return config;
        }());

        std::transform(joysticks.begin(), joysticks.end(), std::back_inserter(results), [](auto& joystick) {
            constexpr static auto extension = ".cfg"sv;
            std::string file_name;
            file_name.reserve(joystick.name.size() + extension.size());
            file_name.assign(joystick.name);
            file_name.append(extension);
            return game_config{ file_name, [&] () {
            text_game_config config(siege::configuration::id_tech::id_tech_2::save_config);

            auto create_bind = [&] (const auto& action) {
                config.emplace({"bind", action.target_meta_name}, action.name);
            };

            std::for_each(joystick.buttons.cbegin(), joystick.buttons.cend(), [&] (const auto& button) {
                std::for_each(button.actions.cbegin(), button.actions.cend(), create_bind);
            });

            std::for_each(joystick.hats.cbegin(), joystick.hats.cend(), [&] (const auto& hat) {
                std::for_each(hat.actions.cbegin(), hat.actions.cend(), create_bind);
            });

            std::for_each(joystick.axes.cbegin(), joystick.axes.cend(), [&] (const auto& axis) {
                std::for_each(axis.actions.cbegin(), axis.actions.cend(), create_bind);
            });

            return config;
        }() };
        });

        return results;
    }

    constexpr static auto binding_sensitivity = std::array<std::string_view, 5> {{
        "joy_forwardsensitivity",
        "joy_upsensitivity",
        "joy_yawsensitivity", 
        "joy_sidesensitivity"
        "joy_pitchsensitivity", 
    }};

    constexpr static auto binding_deadzone = std::array<std::string_view, 5> {{
        "joy_forwardthreshold",
        "joy_upthreshold",
        "joy_yawthreshold", 
        "joy_sidethreshold"
        "joy_pitchthreshold", 
    }};

}