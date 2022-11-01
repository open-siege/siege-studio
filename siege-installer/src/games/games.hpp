#ifndef OPEN_SIEGE_GAMES_HPP
#define OPEN_SIEGE_GAMES_HPP

#include <array>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <optional>
#include <variant>
#include <system_error>

struct game_storage_properties
{
  std::size_t number_of_discs;
  std::array<std::string_view, 32> disc_names;
  std::string_view default_install_path;
};

struct literal_template
{
  std::string_view value;
};

struct external_template
{
  std::string_view filename;
};


struct template_args
{
  const char* original_path;
  std::size_t original_path_size;

  const char* file_path;
  std::size_t file_path_size;
};

using template_function = std::errc(template_args* args) noexcept;

struct external_generated_template
{
  std::string_view function_name;
};

struct internal_generated_template
{
  template_function& generate_template;
};

using game_template = std::variant<literal_template, external_template, external_generated_template, internal_generated_template>;

struct game_info
{
  std::unordered_map<std::string_view, std::vector<std::string_view>>& variables;
  std::unordered_map<std::string_view, game_template>& generated_files;
  std::unordered_map<std::string_view, std::string_view>& directory_mappings;
  game_storage_properties& storage_properties;
};


std::vector<std::string_view> get_supported_games();

bool is_supported_game(std::string_view game_name);

std::optional<game_info> get_info_for_game(std::string_view game_name);

#endif// OPEN_SIEGE_GAMES_HPP
