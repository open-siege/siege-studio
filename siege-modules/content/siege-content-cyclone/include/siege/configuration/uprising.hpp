#ifndef UPRISING_CONFIG_HPP
#define UPRISING_CONFIG_HPP

#include <string>
#include <array>
#include <optional>
#include <istream>
#include <ostream>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::configuration::cyclone
{
  using int32_le = siege::platform::little_uint32_t;

  struct uprising_1_settings
  {
    std::array<int32_le, 2> unknown = { { 0, 1 } };
    int32_le fov = 60;
    int32_le horizon_distance = 22000;
    int32_le unknown2;
    int32_le sky_enabled = 1;
    int32_le unknown4;
    int32_le shadows_enabled = 1;
    int32_le smoke_level = 1;
    int32_le debris = 1;
    std::string player_name = "PLAYER";
    std::string last_map = "ARANON";
    int32_le unknown6;
    int32_le unknown7;
    int32_le screen_size = 100;
    int32_le unknown8[3];
    int32_le flip_mouse = 0;
    float mouse_acceleration;
    float mouse_sensitivity;
    int32_le joystick_type = 0;
    int32_le music_volume;
    int32_le sound_effects_volume;
  };

  std::optional<uprising_1_settings> read_settings(std::istream&);
  void write_settings(std::ostream&, const uprising_1_settings&);

  struct uprising_1_key_map
  {
    enum struct action
    {
      none,
      move_forward,
      move_back,
      strafe_right,
      strafe_left,
      turn_right,
      turn_left,
      fire_weapon_1,
      fire_weapon_2,
      deploy_soldier,
      deploy_tank,
      deploy_aav,
      deploy_bomber,
      call_in_turret,
      call_in_sams,
      deploy_killersat,
      auto_call_in,
      overhead_map,
      jump_to_citadel,
      change_tank_view,
      turret_lock,
      gun_lock,
      call_in_citadel,
      weapon_1_up,
      weapon_1_down,
      weapon_2_up,
      weapon_2_down,
      sell_back,
      increase_radar,
      decrease_radar,
      toggle_hud_type,
      unassigned2,
      look_up,
      look_down,
      repair_wraith,
      screenshot,
      power_triangle,
      cycle_power_triangle,
      toggle_target_lock,
      select_ai_unit,
      unit_menu_popup,
      object_view,
      weapon_menu_popup,
      pause_game = 46,
      pause_menu,
      chat,
      display_call_in_keys = 51,
      call_soldier_building,
      call_tank_building,
      call_aav_building,
      call_bomber_building,
      call_power_building,
      call_ksat_building,
      weapon_laser,
      weapon_hasm,
      weapon_remote_health,
      weapon_spline,
      weapon_helix,
      weapon_cupid,
      weapon_mortar,
      weapon_mine,
      weapon_moletorp,
      weapon_flame,
      weapon_mine_layer,
      weapon_anti_matt_disc,
      weapon_hive,
      weapon_bfm_9000
    };
    std::array<action, 255> keyboard_mappings;

    struct
    {
      int32_le fire_weapon_1;
      int32_le fire_weapon_2;
      int32_le weapon_1_up;
      int32_le weapon_1_down;
      int32_le weapon_2_up;
      int32_le weapon_2_down;
      int32_le overhead_map;
      int32_le call_in_something_1;
      int32_le call_in_something_2;
      int32_le call_in_something_3;
      int32_le call_in_something_4;
      int32_le call_in_something_5;
      int32_le call_in_something_6;
      int32_le unknown1;
      int32_le unknown2;
      int32_le unknown3;
      int32_le unknown4;
      int32_le unknown[15];
    } joystick_mappings;
  };

  std::optional<uprising_1_key_map> read_key_map(std::istream&);
  void write_key_map(std::ostream&, const uprising_1_key_map&);

}// namespace siege::configuration::cyclone


#endif// !UPRISING_CONFIG_HPP
