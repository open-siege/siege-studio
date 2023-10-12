#include <array>
#include <utility>
#include <string_view>
#include <cstdint>


// Soldier of Fortune

constexpr static auto attack_on = "+attack";
constexpr static auto speed_on = "+speed";
constexpr static auto move_up_on = "+moveup";
constexpr static auto move_down_on = "+movedown";
constexpr static auto melee_attack_on = "+melee-attack";
constexpr static auto weapnext = "weapnext";
constexpr static auto move_forward_backward = "move_forward_backward";
constexpr static auto move_left_right = "move_left_right";
constexpr static auto look_up_down = "look_up_down";
constexpr static auto turn_left_right = "turn_left_right";

constexpr static auto sof_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"r2", attack_on },
    {"l2", "+altattack" },
    {"l1", "itemnext" },
    {"r1", "itemuse"},
    {"left_y_axis",  move_forward_backward },
    {"left_x_axis", move_left_right },
    {"right_y_axis", look_up_down },
    {"right_x_axis", turn_left_right },
    {"l3", speed_on },
    {"r3", melee_attack_on },
    {"triangle", weapnext },
    {"circle",  move_down_on },
    {"square", "+use-plus-special"},
    {"cross", move_up_on },
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
    {"r2", attack_on },
    {"l2", "holster" },
    {"l1", "invnext" },
    {"r1", "invuse"},
    {"left_y_axis", move_forward_backward},
    {"left_x_axis", move_left_right },
    {"right_y_axis", look_up_down},
    {"right_x_axis", turn_left_right },
    {"l3", speed_on },
    {"r3", melee_attack_on },
    {"triangle", weapnext},
    {"circle", move_down_on },
    {"square", "+activate"},
    {"cross", move_up_on},
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
    {"r2", attack_on },
    {"l2", "weaponuse" },
    {"l1", "invnext" },
    {"r1", "invuse"},
    {"left_y_axis", move_forward_backward},
    {"left_x_axis", move_left_right },
    {"right_y_axis", look_up_down },
    {"right_x_axis", turn_left_right },
    {"l3", speed_on },
    {"r3", melee_attack_on },
    {"triangle", weapnext},
    {"circle", move_down_on},
    {"square", "+use"},
    {"cross", move_up_on},
    {"start", "showinfo"},
    {"select", "toggleviewmode"} 
}};

// Heretic 2
// base/Default.cfg
// base/user.cfg
// base/autoexec.cfg

constexpr static auto heretic2_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"r2", attack_on },
    {"l2", "+defend" },
    {"l1", "defprev" },
    {"r1", "defnext"},
    {"left_y_axis", move_forward_backward },
    {"left_x_axis", move_left_right },
    {"right_y_axis", look_up_down },
    {"right_x_axis", turn_left_right },
    {"l3", speed_on },
    {"r3", melee_attack_on},
    {"triangle", weapnext},
    {"circle", "+creep"},
    {"square", "+action"},
    {"cross", move_up_on},
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
    {"r2", attack_on },
    {"l2", "invuse" },
    {"l1", "invnext" },
    {"r1", "+throw-grenade"},
    {"left_y_axis", move_forward_backward },
    {"left_x_axis", move_left_right },
    {"right_y_axis", look_up_down },
    {"right_x_axis", turn_left_right },
    {"l3", "+speed" },
    {"triangle", weapnext},
    {"circle", move_down_on},
    {"square", "inven"},
    {"cross", move_up_on},
    {"start", "cmd help"},
    {"select", "score"} 
}};

//alias +throw-grenade "use grenades; +attack"
//alias -throw-grenade "weapprev; -attack"



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

