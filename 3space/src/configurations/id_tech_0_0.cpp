#include <limits>
#include "configurations/id_tech.hpp"
#include "endian_arithmetic.hpp"

namespace studio::configurations::id_tech
{
    namespace endian = boost::endian;

    namespace id_tech_0_0
    {
        namespace wolf3d
        {
            std::optional<game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const game_config&);
        }

        namespace blake_stone
        {
            std::optional<game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const game_config&);
        }

        namespace corridor_7
        {
            std::optional<game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const game_config&);
        }

        namespace body_count
        {
            std::optional<game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const game_config&);
        }
    }

    namespace id_tech_0_5
    {
        namespace rott
        {
            std::optional<game_config> load_config(std::istream&, std::size_t);
            void save_config(std::istream&, const game_config&);
        }
    }
}