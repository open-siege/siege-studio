#ifndef OPEN_SIEGE_EARTHSIEGE1_HPP
#define OPEN_SIEGE_EARTHSIEGE1_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace earthsiege1
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "earthsiege",
    "Earthsiege",
    "earthsiege 1"
    "Earthsiege 1"
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "ESCD_SO"
    };

    result.default_install_path = "<systemDrive>/Games/Gulf War - Operation Desert Hammer";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"readme.txt", "="},
    {"readme.rtf", "="},
    {"gw.ico", "="},
    {"_setup/data1.cab", "."},
    {"_setup/_user1.cab/*.bmp", "assets"},
  };
}

#endif// OPEN_SIEGE_EARTHSIEGE1_HPP
