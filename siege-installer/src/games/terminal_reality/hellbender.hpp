#ifndef OPEN_SIEGE_HELLBENDER_HPP
#define OPEN_SIEGE_HELLBENDER_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace hellbender
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "hellbender",
    "Hellbender"
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "HELLBENDER"
    };

    result.default_install_path = "<systemDrive>/Games/Hellbender";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"HELLBEND.EXE", "="},
    {"SMACKW32.DLL", "="},
    {"HELLBEND.HLP", "="},
    {"readme.doc", "="},
    {"SOUND", "="},
    {"system", "="}
  };
}




#endif// OPEN_SIEGE_HELLBENDER_HPP
