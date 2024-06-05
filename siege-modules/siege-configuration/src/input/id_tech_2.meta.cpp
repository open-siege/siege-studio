#include <array>
#include <utility>
#include <string_view>
#include <cstdint>
#include <algorithm>
#include <siege/platform/joystick_info.hpp>

namespace siege
{
    using joystick_info = siege::joystick_info;
    using button = siege::button;
    using hat = siege::hat;
    using axis = siege::axis;

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

        return info;
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

        return info;
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

        return result;
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

        return result;
    }

    joystick_info add_quake_3_input_metadata(joystick_info info)
    {
        std::transform(info.buttons.begin(), info.buttons.end(), info.buttons.begin(), add_quake_3_button_metadata);
        std::transform(info.axes.begin(), info.axes.end(), info.axes.begin(), add_quake_3_axis_metadata);
        std::transform(info.hats.begin(), info.hats.end(), info.hats.begin(), add_quake_3_hat_metadata);

        return info;
    }
}