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
            void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data);
        }

        namespace blake_stone
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data);
        }

        namespace corridor_7
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data);
        }

        namespace body_count
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data);
        }
    }

    namespace id_tech_0_5
    {
        namespace shadowcaster
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data);
        }

        namespace cyclones
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data);
        }

        namespace rott
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data);
        }

        namespace greed
        {
            std::optional<binary_game_config> load_config(std::istream&, std::size_t);
            void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data);
        }
    }

    namespace id_tech_1
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);
        void save_config(const std::vector<text_game_config::config_line>& entries, std::ostream& raw_data);
    }

    namespace id_tech_2
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t);
        void save_config(const std::vector<text_game_config::config_line>& entries, std::ostream& raw_data);
    }
}

#endif