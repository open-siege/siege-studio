#ifndef OPEN_SIEGE_4x4_EVO_2_HPP
#define OPEN_SIEGE_4x4_EVO_2_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace evo24x4
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "4x4evo2",
    "4x4Evo2",
    "4x4evolution2",
    "4x4Evolution2",
    "4x4 evolution 2",
    "4x4 Evolution 2",
    "fourxfour evolution 2",
    "FourXFour Evolution 2",
    "four by four evolution 2",
    "Four by Four Evolution 2",
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "4x4 Evo 2"
    };

    result.default_install_path = "<systemDrive>/Games/4x4 Evolution 2  ";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"data1.cab", "."},
    {"4x42.exe", "."},
    {"4x4evo2.ICO", "."},
    {"_user1.CAB", "assets"},
    {"*.bmp", "assets"}
  };
}




#endif// OPEN_SIEGE_4x4_EVO_2_HPP
