#ifndef SIEGE_INSTALLATION_MODULE_HPP
#define SIEGE_INSTALLATION_MODULE_HPP

#include <optional>
#include <filesystem>
#include <array>
#include <span>
#include <siege/platform/win/module.hpp>
#include <siege/platform/shared.hpp>

namespace siege::platform
{
  struct game_storage_properties
  {
    std::size_t struct_size = sizeof(game_storage_properties);
    std::uint32_t number_of_discs = 0;
    std::array<const fs_char*, 16> disc_names{};
    std::array<fs_char, 1024> default_install_path{};
  };

  struct path_pair
  {
    const fs_char* source;
    const fs_char* destination;
  };

  struct installation_variable
  {
    const fs_char* name;
    const fs_char* label;
  };

  using installation_option = installation_variable;

}// namespace siege::platform

#endif