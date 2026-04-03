#ifndef ID_TECH_CONFIG_HPP
#define ID_TECH_CONFIG_HPP

#include <siege/configuration/shared.hpp>
#include <istream>
#include <any>

namespace siege::configuration::id_tech
{
  namespace id_tech_2
  {
    struct id_tech_context
    {
      std::string default_end_line = "\r\n";
    };

    std::optional<text_game_config> load_config(std::istream&, std::size_t);
    void save_config(const std::any&, const std::vector<text_game_config::config_line>& entries, std::ostream& raw_data);
  }// namespace id_tech_2
}// namespace siege::configuration::id_tech

#endif