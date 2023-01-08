#ifndef OPEN_SIEGE_FLY_HPP
#define OPEN_SIEGE_FLY_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games/games.hpp"

namespace fly
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "fly",
    "Fly",
    "fly!",
    "Fly!",
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "FLY"
    };

    result.default_install_path = "<systemDrive>/Games/Fly!";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"DATA1.CAB", "."},
    {"_USER1.CAB", "."},
    {"DEMO.DBF", "="}
    {"NOCTURNE.ico", "="},
    {"AVI", "="}
  };
}




#endif// OPEN_SIEGE_FLY_HPP
