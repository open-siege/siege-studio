#include <array>
#include <utility>
#include <string_view>
#include <cstdint>



constexpr static auto dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"r2", "+attack" },
    {"l2", "+altattack" },
    {"l1", "itemnext" },
    {"r1", "itemuse"},
    {"left_y_axis", "move_forward_backward"},
    {"left_x_axis", "move_left_right" },
    {"right_y_axis", "look_up_down"},
    {"right_x_axis", "turn_left_right"},
    {"l3", "+speed" },
    {"r3", "+melee-attack"},
    {"triangle", "weapnext"},
    {"circle", "+movedown"},
    {"square", "+use-plus-special"},
    {"cross", "+moveup"},
    {"start", "menu objectives"},
    {"select", "score"},
}};

//alias +use-plus-special1 "+weaponextra1; +use"
//alias -use-plus-special1 "-weaponextra1; -use; alias +use-plus-special +use-plus-special2; alias -use-plus-special -use-plus-special2"
//alias +use-plus-special2 "+weaponextra2; +use"
//alias -use-plus-special2 "-weaponextra2; -use; alias +use-plus-special +use-plus-special1; alias -use-plus-special -use-plus-special1"
//alias +use-plus-special +use-plus-special1
//alias -use-plus-special -use-plus-special1

//alias +melee-attack "weaponselect 1; +attack"
//alias -melee-attack "weaponbestsafe; -attack"

// user/config.cfg
// user/CURRENT.cfg
// base/autoexec.cfg

// bind BUTTON "action"
constexpr static auto joystick_buttons = std::array<std::string_view> {{
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
    "AUX12"
}};

// bind BUTTON "action"
constexpr static auto joystick_dpad = std::array<std::string_view> {{
    "AUX29", // d-pad up
    "AUX30", // d-pad right
    "AUX31", // d-pad down
    "AUX32" // d-pad left
}};

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


enum struct weapon_assignment : std::uint_8
{
    nothing = 0,
    knife,
    pistolLight,
    pistolHeavy,
    shotgun,
    sniper,
    machineGunLight,
    machineGunLight2,
    machineGunHeavy,
    machineGunHeavy2,
    rocketLauncher,
    flameThrower,
    lightningGun
};

