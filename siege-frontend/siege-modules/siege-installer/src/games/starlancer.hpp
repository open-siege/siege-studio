#ifndef OPEN_SIEGE_STARLANCER_HPP
#define OPEN_SIEGE_STARLANCER_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games.hpp"

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
    result.number_of_discs = 2;
    result.disc_names = {
      "SL_CD1",
      "SL_CD2",
    };

    result.default_install_path = "<systemDrive>/Dynamix/ESCD";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"SL_CD1/SL.ICO", "."},
    {"SL_CD1/LANCER.CAB/CAB", "."},
    {"SL_CD1/GAME/CAB", "."},
    {"SL_CD1/GAME/CD1.BIN", "."},
    {"SL_CD1/GAME/CD1.HOG", "."},
    {"SL_CD2/GAME/CD2.BIN", "."},
    {"SL_CD2/GAME/CD2.HOG", "."},
    {"SL_CD2/DOCS/*.HOG", "docs"}
  };
}

#endif// OPEN_SIEGE_STARLANCER_HPP
