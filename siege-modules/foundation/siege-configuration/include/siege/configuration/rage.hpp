#ifndef JEDI_CONFIG_HPP
#define JEDI_CONFIG_HPP

#include "configurations/shared.hpp"
#include <istream>

namespace siege::configuration::rage
{
    namespace hostile_waters
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);

        void save_config(std::istream&, const text_game_config&);
    }
}

#endif