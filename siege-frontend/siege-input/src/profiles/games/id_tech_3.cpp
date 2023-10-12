
#include <array>
#include <string_view>
// Quake 3

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

constexpr static auto quake3_dual_stick_defaults = std::array<std::array<std::string_view, 2>> {{
    {"r2", attack_on },
    {"l2", "+zoom" },
    {"l1", "weapprev" },
    {"r1", weapnext },
    {"left_y_axis",  move_forward_backward },
    {"left_x_axis", move_left_right },
    {"right_y_axis", look_up_down },
    {"right_x_axis", turn_left_right },
    {"l3", speed_on },
    {"r3", melee_attack_on },
    {"triangle", "vote yes" },
    {"circle",  move_down_on },
    {"square", "vote no"},
    {"cross", move_up_on },
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
    {"r2", attack_on },
    {"l2", "weapalt" },
    {"l1", "itemnext" },
    {"r1", "+useitem" },
    {"left_y_axis",  move_forward_backward },
    {"left_x_axis", move_left_right },
    {"right_y_axis", look_up_down },
    {"right_x_axis", turn_left_right },
    {"l3", "+sprint" },
    {"r3", "+kick" },
    {"triangle", weapnext },
    {"circle",  move_down_on },
    {"square", "+activate"},
    {"cross", move_up_on },
    {"start", "pause"},
    {"select", "+scores"},
}};

// Wolfenstein: Enemy Territory

// Allied Assault

// Call of Duty

// Call of Duty United Offensive

// Soldier of Fortune 2

// Alice

// FAKK 2

// Jedi Knight 2

// Jedi Academy

// Elite Force

// Elite Force 2

// Iron Grip: Warlord

// Dark Salvation

// Space Trader Merchant Marine


