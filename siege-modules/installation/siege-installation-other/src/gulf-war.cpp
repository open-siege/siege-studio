#include <array>
#include <siege/platform/installation_module.hpp>

using namespace siege;
using game_storage_properties = platform::game_storage_properties;
using path_rule = platform::path_rule;

extern "C" {
extern std::array<const fs_char*, 3> name_variations = {
  FSL"Gulf War",
  FSL"Gulf War"
};

extern game_storage_properties storage_properties = {
  .number_of_discs = 1,
  .disc_names = { FSL"Gulf_war" },
  .default_install_path = FSL"<systemDrive>/Games/Gulf War - Operation Desert Hammer"
};

extern std::array<const fs_char*, 2> associated_extensions = {
  FSL"siege-extension-gulf-war"
};

extern std::array<path_rule, 32> directory_mappings = { { 
  { FSL"install/data1.cab/*", FSL"." },
  { FSL"_setup/ramlockC.VXD", FSL "." },
  { FSL"_setup/SMACKW32.DLL", FSL "." } } };
}