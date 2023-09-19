#include <limits>
#include "configurations/binary.hpp"
#include "endian_arithmetic.hpp"
#include "id_tech.hpp"
#include "unreal.hpp"
#include "build.hpp"

namespace studio::configurations::text
{
    std::optional<game_config> load_config(std::istream& stream, std::size_t size)
    {
        return unreal::unreal_1::load_config(stream, size)
                .value_or(serious::serious_1::load_config(stream, size))
                .value_or(krass::load_config(stream, size))
                .value_or(id_tech::id_tech_2::load_config(stream, size))
                .value_or(id_tech::id_tech_1::load_config(stream, size))
                .value_or(build::load_config(stream, size))
                .value_or(id_tech::rott::load_config(stream, size));
    }

    void save_config(std::istream&, const game_config&)
    {

    }
}