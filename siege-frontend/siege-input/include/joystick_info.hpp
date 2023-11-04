#ifndef OPEN_SIEGE_CONTROLLER_MAPPING_HPP
#define OPEN_SIEGE_JOYSTICK_INFO_HPP

#include <optional>
#include <variant>
#include <array>
#include <vector>
#include <string_view>
#include <variant>
#include <array>
#include <utility>
#include <algorithm>

namespace siege
{
    namespace common
    {
        using namespace std::literals;
        constexpr static auto left_x = "left_x"sv;
        constexpr static auto left_y = "left_y"sv;
        constexpr static auto right_x = "right_x"sv;
        constexpr static auto right_y = "right_y"sv;
        constexpr static auto d_pad_up = "d_pad_up"sv;
        constexpr static auto d_pad_down = "d_pad_down"sv;
        constexpr static auto d_pad_left = "d_pad_left"sv;
        constexpr static auto d_pad_right = "d_pad_right"sv;

        namespace types
        {
            constexpr static auto xbox = "xbox"sv;
            constexpr static auto playstation = "playstation"sv;
            constexpr static auto game_controller = "game_controller"sv;
            constexpr static auto flight_stick = "flight_stick"sv;
            constexpr static auto throttle = "throttle"sv;
            constexpr static auto steering_wheel = "steering_wheel"sv;
        }
    }

    namespace playstation
    {
        using namespace std::literals;
        constexpr static auto cross = "cross"sv;
        constexpr static auto circle = "circle"sv;
        constexpr static auto square = "square"sv;
        constexpr static auto triangle = "triangle"sv;
        constexpr static auto l1 = "l1"sv;
        constexpr static auto r1 = "r1"sv;
        constexpr static auto l2 = "l2"sv;
        constexpr static auto r2 = "r2"sv;
        constexpr static auto l3 = "l3"sv;
        constexpr static auto r3 = "r3"sv;
        constexpr static auto start = "start"sv;
        constexpr static auto select = "select"sv;
    }

    namespace xbox
    {
        using namespace std::literals;
        constexpr static auto a = "a"sv;
        constexpr static auto b = "b"sv;
        constexpr static auto x = "x"sv;
        constexpr static auto y = "y"sv;
        constexpr static auto lb = "lb"sv;
        constexpr static auto rb = "rb"sv;
        constexpr static auto lt = "lt"sv;
        constexpr static auto rt = "rt"sv;
        constexpr static auto ls = "ls"sv;
        constexpr static auto rs = "rs"sv;
        constexpr static auto start = "start"sv;
        constexpr static auto select = "select"sv;
    }


    constexpr static auto xbox_mapping = std::array<std::array<std::string_view, 2>, 10> {{
        { xbox::a, playstation::cross },
        { xbox::b, playstation::circle },
        { xbox::x, playstation::square },
        { xbox::y, playstation::triangle },
        { xbox::lb, playstation::l1 },
        { xbox::rb, playstation::r1 },
        { xbox::lt, playstation::l2 },
        { xbox::rt, playstation::r2 },
        { xbox::ls, playstation::l3 },
        { xbox::rs, playstation::r3 }
    }};

    constexpr std::string_view to_xbox(std::string_view value)
    {
        for (auto i = 0; i < xbox_mapping.size(); ++i)
        {
            if (xbox_mapping[i][1] == value)
            {
                return xbox_mapping[i][0];
            }

            if (xbox_mapping[i][1] == value)
            {
                return xbox_mapping[i][1];
            }
        }

        return value;
    }

    static_assert(to_xbox(playstation::cross) == xbox::a);
    static_assert(to_xbox(playstation::circle) == xbox::b);
    static_assert(to_xbox(playstation::square) == xbox::x);
    static_assert(to_xbox(playstation::triangle) == xbox::y);
    static_assert(to_xbox(playstation::l1) == xbox::lb);
    static_assert(to_xbox(playstation::r1) == xbox::rb);
    static_assert(to_xbox(playstation::l2) == xbox::lt);
    static_assert(to_xbox(playstation::r2) == xbox::rt);
    static_assert(to_xbox(playstation::l3) == xbox::ls);
    static_assert(to_xbox(playstation::r3) == xbox::rs);
    static_assert(to_xbox(xbox::a).data() == xbox::a.data());

    constexpr std::string_view to_playstation(std::string_view value)
    {
        for (auto i = 0; i < xbox_mapping.size(); ++i)
        {
            if (xbox_mapping[i][0] == value)
            {
                return xbox_mapping[i][1];
            }

            if (xbox_mapping[i][1] == value)
            {
                return xbox_mapping[i][1];
            }
        }

        return value;
    }

    static_assert(to_playstation(xbox::a) == playstation::cross);
    static_assert(to_playstation(xbox::b) == playstation::circle);
    static_assert(to_playstation(xbox::x) == playstation::square);
    static_assert(to_playstation(xbox::y) == playstation::triangle);
    static_assert(to_playstation(xbox::lb) == playstation::l1);
    static_assert(to_playstation(xbox::rb) == playstation::r1);
    static_assert(to_playstation(xbox::lt) == playstation::l2);
    static_assert(to_playstation(xbox::rt) == playstation::r2);
    static_assert(to_playstation(xbox::ls) == playstation::l3);
    static_assert(to_playstation(xbox::rs) == playstation::r3);
    static_assert(to_playstation(playstation::cross).data() == playstation::cross.data());

    struct button
    {
        std::size_t index;
        std::optional<std::string_view> button_type;
        bool is_left_trigger;
        bool is_right_trigger;
    };

    struct axis
    {
        std::size_t index;
        std::optional<std::string_view> axis_type;
    };

    struct hat
    {
        std::size_t index;
        bool is_controller_dpad;
    };

    struct joystick_info
    {
        std::string name;
        std::optional<std::string_view> type;
        std::optional<std::string_view> controller_type;

        std::vector<button> buttons;
        std::vector<axis> axes;
        std::vector<hat> hats;
    };

    std::vector<joystick_info> get_all_joysticks();
}


#endif// OPEN_SIEGE_JOYSTICK_INFO_HPP