#ifndef ID_TECH_CONFIG_HPP
#define ID_TECH_CONFIG_HPP

#include "configurations/shared.hpp"
#include <istream>

namespace studio::configurations::id_tech
{
    namespace id_tech_0_0
    {
        namespace wolf3d
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const binary_game_config&);
        }

        namespace blake_stone
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const binary_game_config&);
        }

        namespace corridor_7
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const binary_game_config&);
        }

        namespace body_count
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);

            void save_config(std::istream&, const binary_game_config&);
        }
    }

    namespace id_tech_0_5
    {
        namespace shadowcaster
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const binary_game_config&);
        }

        namespace cyclones
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const binary_game_config&);
        }

        namespace rott
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const binary_game_config&);
        }

        namespace greed
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const binary_game_config&);
        }
    }

    namespace id_tech_1
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);
        void save_config(std::istream&, const text_game_config&);
    }

    namespace id_tech_2
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);
        void save_config(std::istream&, const text_game_config&);
    }
}

#endif