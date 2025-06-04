#include <array>
#include <siege/platform/installation_module.hpp>

using namespace siege;
using game_storage_properties = platform::game_storage_properties;
using path_pair = platform::path_pair;
using installation_variable = platform::installation_variable;
using installation_option = platform::installation_option;

extern "C" {
extern std::array<const fs_char*, 5> name_variations = {
  FSL"earthsiege2",
  FSL"Earthsiege2",
  FSL"earthsiege 2"
  FSL"Earthsiege 2"
};

extern game_storage_properties storage_properties = {
  .number_of_discs = 1,
  .disc_names = { FSL "EARTHSIEGE2" },
  .default_install_path = FSL "<systemDrive>/Dynamix/Dynamix/EarthSiege 2"
};

extern std::array<const fs_char*, 2> associated_extensions = {
  FSL"siege-extension-earthsiege-2"
};

extern std::array<path_pair, 32> directory_mappings = {{ 
  { FSL"TAPES", FSL"=" },
  { FSL"DATA", FSL"=" },
  { FSL"SAV", FSL"=" },
  { FSL"SIMVOICE", FSL"=" },
  { FSL"AVI", FSL"=" },
  { FSL"TAPES", FSL"=" },
  { FSL"VOL", FSL"=" },
  { FSL"VER95", FSL"." },
  { FSL"<languageFolder>/BILLBRD", FSL"=" },
  { FSL"<languageFolder>/ES2GUIDE.HLP", FSL"=" },
  { FSL"<languageFolder>/LANGUAGE.INF", FSL"=" },
  { FSL"<languageFolder>/README.WRI", FSL"=" },
  { FSL"BWCC32.DLL", FSL"=" },
  { FSL"CW3220.DLL", FSL"=" },
  { FSL"SOSLIBS3.DLL", FSL"=" },
  { FSL"SOSLIB.INI", FSL"=" },
  { FSL"*.WIN", FSL"=" },
  { FSL"*.STR", FSL"=" } } };


extern std::array<installation_variable, 2> variables = {{
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
      installation_option{ .name = FSL "ENGLISH", .label = FSL"English" },
      installation_option{ .name = FSL"GERMAN", .label = FSL"German" },
      installation_option{ .name = FSL "FRENCH", .label = FSL "French" },
      installation_option{ .name = FSL "SPANISH", .label = FSL "Spanish" },
      installation_option{ .name = FSL "PORTUG", .label = FSL "Portuguese" },
      installation_option{ .name = FSL "ITALIAN", .label = FSL "Italian" }, 
      installation_option{},
    };

    return modes.data();
  }
  return nullptr;
}

std::errc apply_post_install_steps() noexcept
{
  // TODO generate this files after installation
  /*inline static std::unordered_map<std::string_view, game_template> generated_files = {
    { "DATA/DRIVE.CFG", literal_template{ ".\r\n.\r\n" } },
    { "DATA/LANGUAGE.CFG", literal_template{ "E\r\n" } }
  };*/


  /* For Indeo playback
     * [HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows NT\CurrentVersion\Drivers32]
    "vidc.iv31"="ir32_32.dll"
    "vidc.iv32"="ir32_32.dll"
    "vidc.iv41"="ir41_32.ax"
    "vidc.iv50"="ir50_32.dll"
     *
     */
  return std::errc{};
}
}