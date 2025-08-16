#include <siege/configuration/dosbox.hpp>

namespace siege::configuration::krass
{
    std::optional<text_game_config> load_config(std::istream&, std::size_t)
    {
        return std::nullopt;
    }

    void save_config(const std::vector<text_game_config::config_line>&, std::ostream&)
    {

    }
}