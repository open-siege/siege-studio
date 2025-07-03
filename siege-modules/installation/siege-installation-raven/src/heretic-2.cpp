#include <array>
#include <siege/platform/installation_module.hpp>

using namespace siege;
using game_storage_properties = platform::game_storage_properties;
using path_rule = platform::path_rule;

extern "C" {
extern std::array<const fs_char*, 3> name_variations = {
  FSL"Heretic 2",
  FSL"Heretic II"
};

extern game_storage_properties storage_properties = {
  .number_of_discs = 1,
  .disc_names = { FSL"HERETIC_II" },
  .default_install_path = FSL"<systemDrive>/Games/Heretic II"
};

extern std::array<const fs_char*, 2> associated_extensions = {
  FSL"siege-extension-heretic-2"
};

extern std::array<path_rule, 32> directory_mappings = {{ 
  { FSL"setup/data1.cab/*", FSL"." },
  { FSL"setup/Base/VIDEO/*.smk", FSL"Base/VIDEO" },
  { FSL"setup/Toolkit/data1.cab/*", FSL"Toolkit" },
  { FSL"setup/setup.bmp", FSL"=", path_rule::optional },
}};
}