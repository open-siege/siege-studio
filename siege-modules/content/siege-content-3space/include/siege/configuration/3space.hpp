#ifndef THREE_SPACE_CONFIG_HPP
#define THREE_SPACE_CONFIG_HPP

#include <siege/configuration/shared.hpp>
#include <istream>

namespace siege::configuration::three_space
{
  namespace three_space_3
  {
    std::optional<text_game_config> load_config(std::istream&, std::size_t);
    void save_config(const std::vector<text_game_config::config_line>& entries, std::ostream& raw_data);
  }// namespace three_space_3
}// namespace siege::configuration::three_space

#endif