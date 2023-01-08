#ifndef OPEN_SIEGE_FIFA97_HPP
#define OPEN_SIEGE_FIFA97_HPP

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
      "FIFAPCCD"
    };

    result.default_install_path = "<systemDrive>/Dynamix/ESCD";

    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {};

  inline static std::unordered_map<std::string_view, game_template> generated_files = {};

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"FIFA.ICO", "="},
    {"README.TXT", "="},
    {"FIFADOS.EXE", "="},
    {"FIFAWIN.EXE", "="},
    {"DOS4GW.EXE", "="},
    {"SETUP.EXE", "="},
    {"SETUP.BMP", "="},
    {"SETUP.INS", "="},
    {"SETUP.PKG", "="},
    {"VIDEO", "="},
    {"GAMEART", "="},
    {"FEDATA", "="},
    {"FEART", "="},
    {"AUDIO", "="}
  };
}

#endif// OPEN_SIEGE_FIFA97_HPP
