#include <array>
#include <siege/platform/installation_module.hpp>

using namespace siege;
using game_storage_properties = platform::game_storage_properties;
using path_rule = platform::path_rule;

extern "C" {
extern std::array<const fs_char*, 5> name_variations = {
  FSL"earthsiege",
  FSL"Earthsiege",
  FSL"earthsiege 1"
  FSL"Earthsiege 1"
};

extern game_storage_properties storage_properties = {
  .number_of_discs = 1,
  .disc_names = {FSL"ESCD_SO" },
  .default_install_path = FSL"<systemDrive>/Dynamix/ESCD" 
};

extern std::array<const fs_char*, 2> associated_extensions = {
  FSL "siege-extension-earthsiege"
};

// TODO get the correct mappings
extern std::array<path_rule, 32> directory_mappings = { { { FSL "TAPES", FSL "=" },
  { FSL "DATA", FSL "=" },
  { FSL "SAV", FSL "=" },
  { FSL "SIMVOICE", FSL "=" },
  { FSL "AVI", FSL "=" },
  { FSL "TAPES", FSL "=" },
  { FSL "VOL", FSL "=" },
  { FSL "VER95", FSL "." },
  { FSL "<languageFolder>/BILLBRD", FSL "=" },
  { FSL "<languageFolder>/ES2GUIDE.HLP", FSL "=" },
  { FSL "<languageFolder>/LANGUAGE.INF", FSL "=" },
  { FSL "<languageFolder>/README.WRI", FSL "=" },
  { FSL "BWCC32.DLL", FSL "=" },
  { FSL "CW3220.DLL", FSL "=" },
  { FSL "SOSLIBS3.DLL", FSL "=" },
  { FSL "SOSLIB.INI", FSL "=" },
  { FSL "*.WIN", FSL "=" },
  { FSL "*.STR", FSL "=" } } };
}