#include <array>
#include <utility>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include "joystick_info.hpp"
#include "game_info.hpp"

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

axis add_quake_1_axis_metadata(axis result)
{
    constexpr static auto axis_names = std::array<std::string_view, 6> {{
        "joyadvaxisx",
        "joyadvaxisy",
        "joyadvaxisz",
        "joyadvaxisr",
        "joyadvaxisu",
        "joyadvaxisv",
    }};

    if (result.index < axis_names.size())
    {
        result.meta_name_positive.emplace(axis_names[result.index]);
        result.meta_name_negative.emplace(axis_names[result.index]);
    }

    return result;
}

button add_quake_1_button_metadata(button result)
{
    constexpr static auto button_names = std::array<std::string_view, 15> {{
        "JOY1",
        "JOY2",
        "JOY3",
        "JOY4",
        "AUX5",
        "AUX6",
        "AUX7",
        "AUX8",
        "AUX9",
        "AUX10",
        "AUX11",
        "AUX12",
        "AUX13",
        "AUX14",
        "AUX15"
    }};

    if (result.index < button_names.size())
    {
        result.meta_name.emplace(button_names[result.index]);
    }

    return result;
}


hat add_quake_1_hat_metadata(hat result)
{
    if (result.index == 0)
    {
        result.meta_name_up = "AUX29";
        result.meta_name_down = "AUX31";
        result.meta_name_right = "AUX30";
        result.meta_name_left = "AUX32";
    }

    return result;
}


joystick_info add_quake_1_input_metadata(joystick_info info)
{
    std::transform(info.buttons.begin(), info.buttons.end(), info.buttons.begin(), add_quake_1_button_metadata);
    std::transform(info.axes.begin(), info.axes.end(), info.axes.begin(), add_quake_1_axis_metadata);
    std::transform(info.hats.begin(), info.hats.end(), info.hats.begin(), add_quake_1_hat_metadata);

    return std::move(info);
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

    return std::move(info);
}

axis add_quake_2_axis_metadata(axis result)
{
    constexpr static auto axis_names = std::array<std::string_view, 6> {{
        "joy_advaxisx",
        "joy_advaxisy",
        "joy_advaxisz",
        "joy_advaxisr",
        "joy_advaxisu",
        "joy_advaxisv"
    }};


    if (result.index < axis_names.size())
    {
        result.meta_name_positive.emplace(axis_names[result.index]);
        result.meta_name_negative.emplace(axis_names[result.index]);
    }

    return result;
}

joystick_info add_quake_2_input_metadata(joystick_info info)
{
    std::transform(info.buttons.begin(), info.buttons.end(), info.buttons.begin(), add_quake_1_button_metadata);
    std::transform(info.axes.begin(), info.axes.end(), info.axes.begin(), add_quake_2_axis_metadata);
    std::transform(info.hats.begin(), info.hats.end(), info.hats.begin(), add_quake_1_hat_metadata);

    return std::move(info);
}


button add_quake_3_button_metadata(button result)
{
    constexpr static auto button_names = std::array<std::string_view, 15> {{
        "JOY1",
        "JOY2",
        "JOY3",
        "JOY4",
        "JOY5",
        "JOY6",
        "JOY7",
        "JOY8",
        "JOY9",
        "JOY10",
        "JOY11",
        "JOY12",
        "JOY13",
        "JOY14",
        "JOY15"
    }};

    if (result.index < button_names.size())
    {
        result.meta_name.emplace(button_names[result.index]);
    }

    return result;
}

axis add_quake_3_axis_metadata(axis result)
{
    if (result.index == 0)
    {
        result.meta_name_positive = "UPARROW";
        result.meta_name_negative = "DOWNARROW";
    }

    if (result.index == 1)
    {
        result.meta_name_positive = "RIGHTARROW";
        result.meta_name_negative = "LEFTARROW";
    }

    if (result.index == 2)
    {
        result.meta_name_positive = "JOY18";
        result.meta_name_negative = "JOY19";
    }

    if (result.index == 3)
    {
        result.meta_name_positive = "JOY17";
        result.meta_name_negative = "JOY16";
    }

    return std::move(result);
}

hat add_quake_3_hat_metadata(hat result)
{
    if (result.index == 0)
    {
        result.meta_name_up = "JOY24";
        result.meta_name_down = "JOY25";
        result.meta_name_right = "JOY26";
        result.meta_name_left = "JOY27";
    }

    return std::move(result);
}

joystick_info add_quake_3_input_metadata(joystick_info info)
{
    std::transform(info.buttons.begin(), info.buttons.end(), info.buttons.begin(), add_quake_3_button_metadata);
    std::transform(info.axes.begin(), info.axes.end(), info.axes.begin(), add_quake_3_axis_metadata);
    std::transform(info.hats.begin(), info.hats.end(), info.hats.begin(), add_quake_3_hat_metadata);

    return std::move(info);
}

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

std::vector<game_info> get_id_tech_games()
{
    return {
        game_info {"Quake", add_quake_1_input_metadata, add_quake_1_default_actions },
        game_info {"Quake 2", add_quake_2_input_metadata, add_quake_2_default_actions },
        game_info {"Quake 3", add_quake_3_input_metadata, add_quake_3_default_actions },
        game_info {"SiN", add_quake_2_input_metadata, add_sin_default_actions },
        game_info {"Soldier of Fortune", add_quake_2_input_metadata, add_soldier_of_fortune_default_actions },
        game_info {"Heretic 2", add_quake_2_input_metadata, add_heretic_2_default_actions },
        game_info {"Hexen 2", add_quake_2_input_metadata, add_hexen_2_default_actions },
        game_info {"Kingpin", add_quake_2_input_metadata, add_kingpin_default_actions }
    };
}

std::string_view find_axis_index_for_action(std::string_view)
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

    return "";
}

