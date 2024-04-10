#ifndef OPEN_SIEGE_CART_PRECISION_RACING_HPP
#define OPEN_SIEGE_CART_PRECISION_RACING_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace cart_precision
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "cart precision racing",
    "Cart Precision Racing"
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "CART"
    };

    result.default_install_path = "<systemDrive>/Games/Cart Precision Racing";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"CART.EX_", "CART.EXE"},
    {"README.TXT", "="},
    {"FAQ.TXT", "="},
    {"CART.CNT", "="},
    {"PSS.CNT", "="},
    {"POD.INI", "="},
    {"TRID3D.DLL", "="},
    {"SMACKW32.DLL", "="},
    {"*.POD", "."},
    {"*.HLP", "."},
    {"VIDEOS", "="},
    {"SYSTEM", "="},
    {"RACES", "="},
    {"DRIVER", "="},
    {"GARAGE", "="}
  };
}




#endif// OPEN_SIEGE_CART_PRECISION_RACING_HPP
