#ifndef OPEN_SIEGE_STARSIEGE_HPP
#define OPEN_SIEGE_STARSIEGE_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace starsiege
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "starsiege",
    "Starsiege"
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 2;
    result.disc_names = {
      "STARSIEGE1",
      "STARSIEGE2"
    };

    result.default_install_path = "<systemDrive>/Dynamix/Starsiege";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"STARSIEGE1/setup/data1.cab", "."},
    {"STARSIEGE1/setup/ss.ico", "."},
    {"STARSIEGE1/setup/_user1.cab/Install.avi", "movies/Install.avi"},
    {"STARSIEGE1/data/movies", "movies"},
    {"STARSIEGE1/Extras/Miscellaneous/*.rec", "recordings"},
    {"STARSIEGE1/Extras/Miscellaneous/*.mis", "multiplayer"},
    {"STARSIEGE1/Extras/Miscellaneous/*.cs", "multiplayer"},
    {"STARSIEGE1/Extras/Skins/24bit_BMP's/*", "skins/24bit_BMP's"},
    {"STARSIEGE1/Extras/Skins/Outlines/*", "skins/Outlines"},
    {"STARSIEGE1/Extras/Skins/24bit_BMP's/*", "skins/24bit_BMP's"},
    {"STARSIEGE1/Extras/Skins/PhotoShop_PSD's/*", "skins/PhotoShop_PSD's"},
    {"STARSIEGE1/Extras/Reference Cards/*", "keyMaps"},
    {"STARSIEGE2/data/movies", "movies"},
    {"STARSIEGE2/data/sounds", "sounds"},
  };
}




#endif// OPEN_SIEGE_STARSIEGE_HPP
