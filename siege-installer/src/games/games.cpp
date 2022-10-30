#include <algorithm>
#include "games.hpp"
#include "earthsiege2.hpp"


std::optional<game_info> get_info_for_game(std::string_view game_name)
{
  if (game_name.empty())
  {
    return std::nullopt;
  }

  auto is_earthsiege = std::find(earthsiege2::name_variations.begin(), earthsiege2::name_variations.end(), game_name);

  if (is_earthsiege != earthsiege2::name_variations.end())
  {
    return game_info {
      earthsiege2::variables,
      earthsiege2::generated_files,
      earthsiege2::directory_mappings,
      earthsiege2::storage_properties
    };
  }

  return std::nullopt;
}