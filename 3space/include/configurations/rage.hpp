#ifndef JEDI_CONFIG_HPP
#define JEDI_CONFIG_HPP

#include "configurations/shared.hpp"
#include <istream>

namespace studio::configurations::rage
{
    namespace hostile_waters
    {
        std::optional<game_config> load_config(std::istream&, std::size_t);

        void save_config(std::istream&, const game_config&);
    }
}

#endif