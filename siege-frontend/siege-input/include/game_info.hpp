#ifndef OPEN_SIEGE_GAME_INFO_HPP
#define OPEN_SIEGE_GAME_INFO_HPP

#include <string_view>
#include "joystick_info.hpp"

namespace siege
{
    struct game_info
    {
        std::string_view english_name;
        joystick_info (&add_input_metadata)(joystick_info);
        joystick_info (&add_default_actions)(joystick_info);
    };
}


#endif