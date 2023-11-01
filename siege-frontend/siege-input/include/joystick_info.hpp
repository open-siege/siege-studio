#ifndef OPEN_SIEGE_CONTROLLER_MAPPING_HPP
#define OPEN_SIEGE_JOYSTICK_INFO_HPP

#include <SDL.h>
#include <optional>
#include <variant>
#include <array>
#include <vector>


namespace siege
{
    struct button
    {
        std::optional<SDL_GameControllerButton> button_type;
        bool is_left_trigger;
        bool is_right_trigger;
    };

    struct axis
    {
        std::optional<SDL_GameControllerAxis> axis_type;
    };

    struct hat
    {
        bool is_controller_dpad;
    };

    struct joystick_info
    {
        std::string name;
        SDL_JoystickType type;
        std::optional<SDL_GameControllerType> controller_type;

        std::vector<button> buttons;
        std::vector<axis> axes;
        std::vector<hat> hats;
    };

    std::vector<joystick_info> get_all_joysticks();
}


#endif// OPEN_SIEGE_JOYSTICK_INFO_HPP