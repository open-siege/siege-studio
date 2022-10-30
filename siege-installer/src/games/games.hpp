#ifndef OPEN_SIEGE_GAMES_HPP
#define OPEN_SIEGE_GAMES_HPP

#include <array>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <optional>

struct game_storage_properties
{
  std::size_t number_of_discs;
  std::array<std::string_view, 32> disc_names;
};

struct game_info
{
  std::unordered_map<std::string_view, std::vector<std::string_view>>& variables;
  std::vector<std::string_view>& generated_files;
  std::unordered_map<std::string_view, std::string_view>& directory_mappings;
  game_storage_properties& storage_properties;
};


std::optional<game_info> get_info_for_game(std::string_view game_name);

#endif// OPEN_SIEGE_GAMES_HPP
