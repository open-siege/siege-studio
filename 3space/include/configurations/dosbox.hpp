#ifndef DOSBOX_CONFIG_HPP
#define DOSBOX_CONFIG_HPP

#include "configurations/shared.hpp"
#include <istream>

namespace studio::configurations::dosbox
{
    namespace mapper
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);

        void save_config(const std::vector<text_game_config::config_line>&, std::ostream&);
    }

    namespace config
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);

        void save_config(const std::vector<text_game_config::config_line>&, std::ostream&);
    }
}

#endif