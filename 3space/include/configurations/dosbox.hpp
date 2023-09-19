#ifndef DOSBOX_CONFIG_HPP
#define DOSBOX_CONFIG_HPP

#include "configurations/shared.hpp"
#include <istream>

namespace studio::configurations::dosbox
{
    namespace mapper
    {
        std::optional<game_config> load_config(std::istream&, std::size_t);

        void save_config(std::istream&, const game_config&);
    }

    namespace config
    {
        std::optional<game_config> load_config(std::istream&, std::size_t);

        void save_config(std::istream&, const game_config&);
    }
}

#endif