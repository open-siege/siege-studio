#ifndef SERIOUS_CONFIG_HPP
#define SERIOUS_CONFIG_HPP

#include <siege/configuration/shared.hpp>
#include <istream>

namespace siege::configuration::serious
{
    namespace serious_1
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);

        void save_config(const std::vector<text_game_config::config_line>&, std::ostream&);
    }
}

#endif