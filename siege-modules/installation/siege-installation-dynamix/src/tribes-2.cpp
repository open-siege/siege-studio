#include <array>
#include <siege/platform/installation_module.hpp>

using namespace siege;
using game_storage_properties = platform::game_storage_properties;
using path_rule = platform::path_rule;

extern "C" {
extern std::array<const fs_char*, 5> name_variations = {
  FSL"tribes2",
  FSL"Tribes 2"
};

extern game_storage_properties storage_properties = {
  .number_of_discs = 1,
  .disc_names = {FSL"Tribes2_25034" },
  .default_install_path = FSL"<systemDrive>/Dynamix/Tribes 2" 
};

extern std::array<const fs_char*, 2> associated_extensions = {
  FSL "siege-extension-tribes-2"
};

extern std::array<path_rule, 32> directory_mappings = {{ 
  { FSL "Tribes2/GameData", FSL "." },
 }};
}