#ifndef OPEN_SIEGE_EARTHSIEGE2_HPP
#define OPEN_SIEGE_EARTHSIEGE2_HPP

#include <vector>
#include <string_view>
#include <array>
#include <unordered_map>
#include "games.hpp"

/* For Indeo playback
 * [HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows NT\CurrentVersion\Drivers32]
"vidc.iv31"="ir32_32.dll"
"vidc.iv32"="ir32_32.dll"
"vidc.iv41"="ir41_32.ax"
"vidc.iv50"="ir50_32.dll"
 *
 */

namespace earthsiege2
{
  constexpr static std::array<std::string_view, 16> name_variations = {
    "earthsiege2",
    "Earthsiege2",
    "earthsiege 2"
    "Earthsiege 2"
  };

  inline static game_storage_properties storage_properties = []{
    game_storage_properties result;
    result.number_of_discs = 1;
    result.disc_names = {
      "EARTHSIEGE2"
    };
    return result;
  }();

  inline static std::unordered_map<std::string_view, std::vector<std::string_view>> variables = {
    { "languageFolder", {
                          "ENGLISH",
                          "GERMAN",
                          "FRENCH",
                          "SPANISH",
                          "PORTUG",
                          "ITALIAN"
                        }
    }
  };

  inline static std::unordered_map<std::string_view, game_template> generated_files = {
    { "DATA/DRIVE.CFG", literal_template { ".\r\n.\r\n" }} ,
    { "DATA/LANGUAGE.CFG", literal_template { "E\r\n" } }
  };

  inline static std::unordered_map<std::string_view, std::string_view> directory_mappings = {
    {"TAPES", "="},
    {"DATA", "="},
    {"SAV", "="},
    {"SIMVOICE", "="},
    {"AVI", "="},
    {"TAPES", "="},
    {"VOL", "="},
    { "VER95", "."},
    {"<languageFolder>/BILLBRD", "="},
    {"<languageFolder>/ES2GUIDE.HLP", "="},
    {"<languageFolder>/LANGUAGE.INF", "="},
    {"<languageFolder>/README.WRI", "="},
    { "BWCC32.DLL", "="},
    { "CW3220.DLL", "="},
    { "SOSLIBS3.DLL", "="},
    { "SOSLIB.INI", "="},
    { "*.WIN", "="},
    { "*.STR", "="}
  };
}




#endif// OPEN_SIEGE_EARTHSIEGE2_HPP
