#ifndef XML_CONFIG_HPP
#define XML_CONFIG_HPP

#include <siege/configuration/shared.hpp>
#include <istream>
#include <string>
#include <any>

namespace siege::configuration::common::xml
{
  std::optional<text_game_config> load_config(std::istream&, std::size_t);
  void save_config(const std::any&, const std::vector<text_game_config::config_line>& entries, std::ostream& raw_data);
}// namespace siege::configuration::common::xml

#endif