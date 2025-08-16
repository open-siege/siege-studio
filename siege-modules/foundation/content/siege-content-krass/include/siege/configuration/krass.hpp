#ifndef KRASS_CONFIG_HPP
#define KRASS_CONFIG_HPP

#include <siege/configuration/shared.hpp>
#include <istream>
#include <ostream>

namespace siege::configuration::krass
{
    std::optional<text_game_config> load_config(std::istream&, std::size_t);

    void save_config(const std::vector<text_game_config::config_line>&, std::ostream&);
}

#endif