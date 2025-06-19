#include <array>
#include <siege/platform/installation_module.hpp>

using namespace siege;
using game_storage_properties = platform::game_storage_properties;
using path_rule = platform::path_rule;

extern "C" {
extern std::array<const fs_char*, 3> name_variations = {
  FSL "starsiege",
  FSL "Starsiege"
};

extern game_storage_properties storage_properties = {
  .number_of_discs = 2,
  .disc_names = {
    FSL "STARSIEGE1",
    FSL "STARSIEGE2" },
  .default_install_path = FSL "<systemDrive>/Dynamix/Starsiege"
};

extern std::array<const fs_char*, 2> associated_extensions = {
  FSL "siege-extension-starsiege"
};

extern std::array<path_rule, 32> directory_mappings = { {
  { FSL "STARSIEGE1/setup/data1.cab/*", FSL "." },
  { FSL "STARSIEGE1/setup/ss.ico", FSL "." },
  { FSL "STARSIEGE1/setup/_user1.cab/Install.avi", FSL "movies/Install.avi", path_rule::optional },
  { FSL "STARSIEGE1/data/movies", FSL "movies" },
  { FSL "STARSIEGE1/Extras/Miscellaneous/*.rec", FSL "recordings", path_rule::optional },
  { FSL "STARSIEGE1/Extras/Miscellaneous/*.mis", FSL "multiplayer", path_rule::optional },
  { FSL "STARSIEGE1/Extras/Miscellaneous/*.cs", FSL "multiplayer", path_rule::optional },
  { FSL "STARSIEGE1/Extras/Skins/24bit_BMP's/*", FSL "skins/24bit_BMP's", path_rule::optional },
  { FSL "STARSIEGE1/Extras/Skins/Outlines/*", FSL "skins/Outlines", path_rule::optional },
  { FSL "STARSIEGE1/Extras/Skins/24bit_BMP's/*", FSL "skins/24bit_BMP's", path_rule::optional },
  { FSL "STARSIEGE1/Extras/Skins/PhotoShop_PSD's/*", FSL "skins/PhotoShop_PSD's", path_rule::optional },
  { FSL "STARSIEGE1/Extras/Reference Cards/*", FSL "keyMaps", path_rule::optional },
  { FSL "STARSIEGE2/data/movies", FSL "movies" },
  { FSL "STARSIEGE2/data/sounds", FSL "sounds" },
} };
}