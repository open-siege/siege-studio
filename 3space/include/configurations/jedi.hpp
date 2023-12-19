#ifndef JEDI_CONFIG_HPP
#define JEDI_CONFIG_HPP

#include "configurations/shared.hpp"
#include <istream>

namespace studio::configurations::jedi
{
    namespace dark_forces
    {
        std::optional<binary_game_config> load_config(std::istream&, std::size_t);

        void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data);
    }

    namespace jedi_knight
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);

        void save_config(std::ostream&, const text_game_config&);
    }

    namespace racer
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);

        void save_config(std::ostream&, const text_game_config&);
    }

    namespace shadows_of_the_empire
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);

        void save_config(std::ostream&, const text_game_config&);
    }
}

#endif