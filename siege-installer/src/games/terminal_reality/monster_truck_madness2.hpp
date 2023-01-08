#ifndef OPEN_SIEGE_MTM2_HPP
#define OPEN_SIEGE_MTM2_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace hellbender
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "monster truck",
    "Hellbender"
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "MTM2"
    };

    result.default_install_path = "<systemDrive>/Games/Monster Truck Madness 2";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"MONSTER.EX_", "MONSTER.EXE"},
    {"MONSTER.CNT", "="},
    {"POD.INI", "="},
    {"*.TXT", "."},
    {"*.POD", "."},
    {"*.DLL", "."},
    {"SMACKW32.DLL", "="},
    {"README.TXT", "="},
    {"SYSTEM", "="},
    {"VIDEOS", "="},
  };
}




#endif// OPEN_SIEGE_MTM_HPP
