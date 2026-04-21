#ifndef ID_TECH_CONFIG_HPP
#define ID_TECH_CONFIG_HPP

#include <siege/configuration/shared.hpp>
#include <istream>
#include <string>
#include <any>

namespace siege::configuration::common::ini
{
  struct ini_context
  {
    std::string default_end_line = "\r\n";
  };

  std::optional<text_game_config> load_config(std::istream&, std::size_t);
  void save_config(const std::any&, const std::vector<text_game_config::config_line>& entries, std::ostream& raw_data);
}// namespace siege::configuration::id_tech

#endif