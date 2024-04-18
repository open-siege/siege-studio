#ifndef OPEN_SIEGE_FURY3_HPP
#define OPEN_SIEGE_FURY3_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace fury3
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "fury3",
    "Fury3"
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "MSFURY3"
    };

    result.default_install_path = "<systemDrive>/Games/Fury3";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"FURY3/FURY3.CNT", "."},
    {"FURY3/FURY3.HLP", "."},
    {"FURY3/FURY3_16.HLP", "."},
    {"FURY3/FURY3.EXE", "."},
    {"FURY3/SYSTEM", "."},
    {"SMACKW32.DLL", "."},
    {"HELLBEND.HLP", "."},
    {"readme.doc", "."},
    {"SOUND", "."},
    {"system", "."}
  };
}




#endif// OPEN_SIEGE_FURY3_HPP
