#ifndef BUILD_CONFIG_HPP
#define BUILD_CONFIG_HPP

#include "configurations/shared.hpp"
#include <istream>

namespace studio::configurations::build
{
    std::optional<game_config> load_config(std::istream&, std::size_t);

    void save_config(std::istream&, const game_config&);
}

#endif