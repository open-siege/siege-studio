#include <array>
#include <siege/platform/installation_module.hpp>

using namespace siege;
using game_storage_properties = platform::game_storage_properties;
using path_pair = platform::path_pair;

extern "C" {
extern std::array<const fs_char*, 5> name_variations = {
  FSL"tribes",
  FSL"Tribes",
  FSL"Starsiege: Tribes"
  FSL"Tribes 1"
};

extern game_storage_properties storage_properties = {
  .number_of_discs = 1,
  .disc_names = {FSL"TRIBES" },
  .default_install_path = FSL"<systemDrive>/Dynamix/Tribes" 
};

extern std::array<const fs_char*, 2> associated_extensions = {
  FSL "siege-extension-tribes"
};

extern std::array<path_pair, 32> directory_mappings = {{ 
  { FSL "Tribes", FSL "." },
 }};
}