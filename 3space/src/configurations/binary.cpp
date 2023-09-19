#include <limits>
#include "configurations/binary.hpp"
#include "endian_arithmetic.hpp"
#include "jedi.hpp"
#include "id_tech.hpp"

namespace studio::configurations::binary
{
    std::optional<game_config> load_config(std::istream& stream, std::size_t size)
    {
        return jedi::dark_forces::load_config(stream, size)
                .value_or(id_tech_0_0::wolf3d::load_config(stream, size))
                .value_or(id_tech_0_0::blake_stone::load_config(stream, size))
                .value_or(id_tech_0_0::corridor_7::load_config(stream, size))
                .value_or(id_tech_0_0::body_count::load_config(stream, size));
    }

    void save_config(std::istream&, const game_config&)
    {

    }
}