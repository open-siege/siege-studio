#ifndef OPEN_SIEGE_4x4_EVO_HPP
#define OPEN_SIEGE_4x4_EVO_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace evo4x4
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "4x4evo",
    "4x4Evo",
    "4x4evolution",
    "4x4Evolution",
    "4x4 evolution",
    "4x4 Evolution",
    "fourxfour evolution",
    "FourXFour Evolution",
    "four by four evolution",
    "Four by Four Evolution",
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "4X4EVOLUTION"
    };

    result.default_install_path = "<systemDrive>/Games/4x4 Evolution";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"DATA1.CAB", "."},
    {"_USER1.CAB", "assets"},
    {"4x4.ico", "."}
  };
}




#endif// OPEN_SIEGE_4x4_EVO_HPP
