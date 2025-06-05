#include <array>
#include <siege/platform/installation_module.hpp>

using namespace siege;
using game_storage_properties = platform::game_storage_properties;
using path_pair = platform::path_pair;
using installation_variable = platform::installation_variable;
using installation_option = platform::installation_option;

extern "C" {
extern std::array<const fs_char*, 3> name_variations = {
  FSL"FAKK2",
  FSL"F.A.K.K. 2"
};

extern game_storage_properties storage_properties = {
  .number_of_discs = 1,
  .disc_names = {FSL"HEAVYMETAL"},
  .default_install_path = FSL"<systemDrive>/Games/Heavy Metal FAKK 2"
};

extern std::array<const fs_char*, 2> associated_extensions = {
  FSL"siege-extension-fakk-2"
};

extern std::array<path_pair, 32> directory_mappings = { {
  { FSL"install/data1.cab", FSL"." },
  { FSL"install/data2.cab", FSL"." },
  { FSL"install/fakk2.exe", FSL"." },
  { FSL"install/fakk", FSL"fakk" },
  { FSL"install/fakk/<languageFolder>", FSL"fakk" }
} };

extern std::array<installation_variable, 2> installation_variables = { {
  { .name = FSL"languageFolder", .label = FSL"Language" }
}};

installation_option* get_options_for_variable(const fs_char* variable_name) noexcept
{
  if (variable_name == nullptr)
  {
    return nullptr;
  }

  auto name_str = std::basic_string_view<fs_char>(variable_name);


  if (name_str == L"languageFolder")
  {
    static auto modes = std::array<installation_option, 8>{
      installation_option{ .name = FSL "enu", .label = FSL "English" },
      installation_option{ .name = FSL "deu", .label = FSL "German" },
      installation_option{ .name = FSL "fra", .label = FSL "French" },
      installation_option{ .name = FSL "esp", .label = FSL "Spanish" },
      installation_option{ .name = FSL "ita", .label = FSL "Italian" },
      installation_option{},
    };

    return modes.data();
  }
  return nullptr;
}
}