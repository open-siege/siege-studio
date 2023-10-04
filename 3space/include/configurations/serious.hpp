#ifndef SERIOUS_CONFIG_HPP
#define SERIOUS_CONFIG_HPP

#include "configurations/shared.hpp"
#include <istream>

namespace studio::configurations::serious
{
    namespace serious_1
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);

        void save_config(std::ostream&, const text_game_config&);
    }
}

#endif