#include <algorithm>
#include <siege/installation/games.hpp>
#include "dynamix/earthsiege2.hpp"

std::vector<std::string_view> get_supported_games()
{
    const static std::vector<std::string_view> results = { earthsiege2::name_variations[0] };

    return results;
}

bool is_supported_game(std::string_view game_name)
{
  auto is_earthsiege = std::find(earthsiege2::name_variations.begin(), earthsiege2::name_variations.end(), game_name);
  return is_earthsiege != earthsiege2::name_variations.end();
}


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