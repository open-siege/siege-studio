#include <array>
#include <utility>
#include <string_view>
#include <cstdint>

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

constexpr static auto idtech_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"left_y+", forward_on },
    {"left_y-", backward_on },
    {"left_x+", move_left_on },
    {"left_x-", move_right_on },
    {"right_y+", look_up_on },
    {"right_y-", look_down_on },
    {"right_x+", look_up_down },
    {"right_x-", turn_left_right },
    {playstation::r2, attack_on },
    {playstation::l3, speed_on },
    {playstation::r3, melee_attack_on },
    {playstation::triangle, weapon_next},
    {playstation::circle, move_down_on},
    {playstation::cross, move_up_on}
}};


std::string_view find_quake1_button(std::string_view)
{
    constexpr static auto controller_button_mapping = std::array<std::array<std::string_view, 2>> {{
        { playstation::cross, "JOY1" },
        { playstation::circle, "JOY2" },
        { playstation::square, "JOY3" },
        { playstation::triangle, "JOY4" },
        { playstation::l1, "AUX1" },
        { playstation::r1, "AUX2" },
        { playstation::l2, "AUX3" },
        { playstation::r2, "AUX4" },
        { playstation::l3, "AUX5" },
        { playstation::r3, "AUX6" },
        { playstation::start, "AUX7" },
        { playstation::select, "AUX8" },
    }};
}
std::string_view find_quake2_button(std::string_view)
{
    constexpr static auto controller_button_mapping = std::array<std::array<std::string_view, 2>> {{
        { playstation::cross, "JOY1" },
        { playstation::circle, "JOY2" },
        { playstation::square, "JOY3" },
        { playstation::triangle, "JOY4" },
        { playstation::l1, "AUX1" },
        { playstation::r1, "AUX2" },
        { playstation::l2, "AUX3" },
        { playstation::r2, "AUX4" },
        { playstation::l3, "AUX5" },
        { playstation::r3, "AUX6" },
        { playstation::start, "AUX7" },
        { playstation::select, "AUX8" },
    }};
}

std::string_view find_quake3_button(std::string_view)
{
    constexpr static auto controller_button_mapping = std::array<std::array<std::string_view, 2>> {{
        { playstation::cross, "JOY1" },
        { playstation::circle, "JOY2" },
        { playstation::square, "JOY3" },
        { playstation::triangle, "JOY4" },
        { playstation::l1, "JOY5" },
        { playstation::r1, "JOY6" },
        { playstation::l2, "JOy7" },
        { playstation::r2, "JOY8" },
        { playstation::l3, "JOY9" },
        { playstation::r3, "JOY10" },
        { playstation::start, "JOY11" },
        { playstation::select, "JOY12" },
    }};
}
std::string_view find_quake1_axis(std::string_view)
{
        constexpr static auto controller_axis_mapping = std::array<std::array<std::string_view, 2>> {{
            { "left_x", "joyadvaxisx" },
           { "left_y", "joyadvaxisy" },
           { "right_x", "joyadvaxisz" },
           { "right_y", "joyadvaxisr" }

    }};
}
std::string_view find_quake2_axis(std::string_view)
{
    constexpr static auto controller_button_mapping = std::array<std::array<std::string_view, 2>> {{
           { "left_x", "joy_advaxisx" },
           { "left_y", "joy_advaxisy" },
           { "right_x", "joy_advaxisz" },
           { "right_y", "joy_advaxisr" }
    }};
}

std::string_view find_axis_index_for_action(std::string_view)
{
    constexpr static auto controller_button_mapping = std::array<std::array<std::string_view, 2>> {{
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
}


enum struct quake_config_type
{
    unknown,
    quake1,
    quake2,
    quake3
};


enum struct quake1_game
{
    unknown,
    quake1,
    hexen2,
    laser_arena,
    cia_operative,
    battle_metal
};


text_game_config create_quake1_config(quake_config_type type, quake1_game game)
{
    text_game_config config;

    for (auto& input : idtech_dual_stick_defaults)
    {
        auto virtual_input = input.first;

        if (virtual_input.back() == '-' || virtual_input.back() == '+')
        {
            auto index = find_axis_index_for_action(input.second);
            config.emplace(find_quake1_axis(virtual_input), index);
        }
        else
        {
                auto real_button = find_quake1_button(virtual_input);
                config.emplace({"bind", real_button}, input.second);
        }
    }


    return config;
}

enum struct quake2_game
{
    unknown,
    quake2,
    heretic2,
    kingpin,
    daikatana,
    sin,
    soldier_of_fortune
};

// Soldier of Fortune
constexpr static auto sof_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"l2", "+altattack" },
    {"l1", "itemnext" },
    {"r1", "itemuse"},
    {"square", "+use-plus-special"},
    {"start", "menu objectives"},
    {"select", "score"},
}};

// user/config.cfg
// user/CURRENT.cfg
// base/autoexec.cfg
//alias +use-plus-special1 "+weaponextra1; +use"
//alias -use-plus-special1 "-weaponextra1; -use; alias +use-plus-special +use-plus-special2; alias -use-plus-special -use-plus-special2"
//alias +use-plus-special2 "+weaponextra2; +use"
//alias -use-plus-special2 "-weaponextra2; -use; alias +use-plus-special +use-plus-special1; alias -use-plus-special -use-plus-special1"
//alias +use-plus-special +use-plus-special1
//alias -use-plus-special -use-plus-special1

//alias +melee-attack "weaponselect 1; +attack"
//alias -melee-attack "weaponbestsafe; -attack"

// Kingpin

constexpr static auto kingpin_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"l2", "holster" },
    {"l1", "invnext" },
    {"r1", "invuse"},
    {"square", "+activate"},
    {"start", "cmd help"},
    {"select", "inven"},
    {"d_pad_up", "flashlight"},
    {"d_pad_down", "key2"},
    {"d_pad_left", "key1"},
    {"d_pad_right", "key3"}
}};


// main/config.cfg
// main/autoexec.cfg
//alias +melee-attack "use pipe; +attack"
//alias -melee-attack "weapprev; -attack"

// SIN
// base/default.cfg
// base/players/*/config.cfg
// base/autoexec.cfg
//alias +melee-attack "use Fists; +attack"
//alias -melee-attack "weapprev; -attack"

constexpr static auto sin_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"l2", "weaponuse" },
    {"l1", "invnext" },
    {"r1", "invuse"},
    {"square", "+use"},
    {"start", "showinfo"},
    {"select", "toggleviewmode"} 
}};

// Heretic 2
// base/Default.cfg
// base/user.cfg
// base/autoexec.cfg

constexpr static auto heretic2_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"l2", "+defend" },
    {"l1", "defprev" },
    {"r1", "defnext"},
    {"circle", "+creep"},
    {"square", "+action"},
    {"start", "menu_objectives"},
    {"select", "menu_city_map"} 
}};

//alias +melee-attack "use staff; +attack"
//alias -melee-attack "weapprev; -attack"

// need to swap menu_city_map with menu_world_map

// Quake 2
// baseq2/pak0.pak/default.cfg
// baseq2/config.cfg
// baseq2/autoexec.cfg

constexpr static auto quake2_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"l2", "invuse" },
    {"l1", "invnext" },
    {"r1", "+throw-grenade"},
    {"square", "inven"},
    {"start", "cmd help"},
    {"select", "score"} 
}};

//alias +throw-grenade "use grenades; +attack"
//alias -throw-grenade "weapprev; -attack"



text_game_config create_quake2_config(quake_config_type type, quake2_game game)
{
    text_game_config config;

    auto apply_config = [&](const auto& input) {
            auto virtual_input = input.first;

            if (virtual_input.back() == '-' || virtual_input.back() == '+')
            {
                auto index = find_axis_index_for_action(input.second);
                config.emplace({"set", find_quake2_axis(virtual_input)}, index);
            }
            else
            {
                auto real_button = find_quake2_button(virtual_input);
                config.emplace({"bind", real_button}, input.second);
            }
    };

    std::for_each(idtech_dual_stick_defaults.begin(), idtech_dual_stick_defaults.end(), apply_config);

    if (game == quake2_game::quake2)
    {
        std::for_each(quake2_dual_stick_defaults.begin(), quake2_dual_stick_defaults.end(), apply_config);
    }

    if (game == quake2_game::soldier_of_fortune)
    {
        std::for_each(sof_dual_stick_defaults.begin(), sof_dual_stick_defaults.end(), apply_config);
    }

    if (game == quake2_game::heretic2)
    {
        std::for_each(heretic2_dual_stick_defaults.begin(), heretic2_dual_stick_defaults.end(), apply_config);
    }

    if (game == quake2_game::kingpin)
    {
        std::for_each(kingpin_dual_stick_defaults.begin(), kingpin_dual_stick_defaults.end(), apply_config);
    }

    if (game == quake2_game::sin)
    {
        std::for_each(sin_dual_stick_defaults.begin(), sin_dual_stick_defaults.end(), apply_config);
    }

    return config;
}

enum struct quake3_game
{
    unknown,
    quake3,
    elite_force,
    elite_force2,
    jedi_knight2,
    jedi_academy,
    soldier_of_fortune2,
    return_to_castle_wolfenstein,
    wolfenstein_enemy_territory
    allied_assault,
    call_of_duty
};

// Quake 3
constexpr static auto quake3_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"l2", "+zoom" },
    {"l1", weapprev },
    {"r1", weapnext },
    {"triangle", "vote yes" },
    {"square", "vote no"},
    {"start", "pause"},
    {"select", "+scores"},
}};

//alias +melee-attack "weapon 1; +attack"
//alias -melee-attack "weapnext; -attack"
// baseq3

// Quake Live

// Return to Castle Wolfenstein
// Main
constexpr static auto wolf_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"l2", "weapalt" },
    {"l1", "itemnext" },
    {"r1", "+useitem" },
    {"l3", "+sprint" },
    {"r3", "+kick" },
    {"square", "+activate"},
    {"start", "pause"},
    {"select", "+scores"},
}};


// Wolfenstein: Enemy Territory
constexpr static auto wolfet_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
}};

// Allied Assault
constexpr static auto moh_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
}};

// Call of Duty
constexpr static auto cod_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
}};

// Call of Duty United Offensive

// Soldier of Fortune 2
constexpr static auto sof2_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
}};

// Alice

// FAKK 2

// Jedi Knight 2
constexpr static auto jk2_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
}};

// Jedi Academy
constexpr static auto jka_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
}};

// Elite Force
constexpr static auto ef_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
}};


// Elite Force 2
constexpr static auto ef2_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
}};


// Iron Grip: Warlord

// Dark Salvation

// Space Trader Merchant Marine

text_game_config create_quake3_config(quake_config_type type, quake3_game game)
{
    text_game_config config;

    for (auto& input : idtech_dual_stick_defaults)
    {
            auto virtual_input = input.first;

            auto real_button = find_quake3_button(virtual_input);
            config.emplace({"bind", real_button}, input.second);
    }

    if (game == quake3_game::quake3)
    {
        std::for_each(quake3_dual_stick_defaults.begin(), quake3_dual_stick_defaults.end(), apply_config);
    }

    if (game == quake3_game::elite_force)
    {
        std::for_each(ef_dual_stick_defaults.begin(), ef_dual_stick_defaults.end(), apply_config);
    }

    if (game == quake3_game::elite_force2)
    {
        std::for_each(ef2_dual_stick_defaults.begin(), ef2_dual_stick_defaults.end(), apply_config);
    }

    if (game == quake3_game::jedi_knight2)
    {
        std::for_each(jk2_dual_stick_defaults.begin(), jk2_dual_stick_defaults.end(), apply_config);
    }

    if (game == quake3_game::jedi_academy)
    {
        std::for_each(j.begin(), sin_dual_stick_defaults.end(), apply_config);
    }

    return config;
}


// Quake

// Hexen 2
constexpr static auto joystick_flags = std::array<std::string_view> {{
    "in_joystick", 
    "joy_advanced",
    "use_ff"
}};

enum struct axis_binding : std::uint_8
{
    unbound = 0,
    move_forward_backward,
    look_up_down, // trackball only
    move_left_right,
    turn_left_right,
    crouch_jump
};

// set FLAG "1"

// set AXIS "BINDING"
constexpr static auto joystick_axis = std::array<std::string_view> {{
    "joy_advaxisx", 
    "joy_advaxisy",
    "joy_advaxisz", 
    "joy_advaxisr", 
    "joy_advaxisu",
    "joy_advaxisv"
}};


// from -2 to 2
// negative inverts the axis
constexpr static auto binding_sensitivity = std::array<std::string_view> {{
    "joy_forwardsensitivity",
    "joy_upsensitivity",
    "joy_yawsensitivity", 
    "joy_sidesensitivity"
    "joy_pitchsensitivity", 
}};

constexpr static auto binding_deadzone = std::array<std::string_view> {{
    "joy_forwardthreshold",
    "joy_upthreshold",
    "joy_yawthreshold", 
    "joy_sidethreshold"
    "joy_pitchthreshold", 
}};


// 007 Nightfire
// bond/config.cfg

// Half Life Day One

// Gunman Chronicles

// Half Life (Steam)

// Counter Strike (Steam)

// Day of Defeat (Steam)

// Ricochet (Steam)

// Deathmatch Classic (Steam)