#include "configurations/shared.hpp"
#include <algorithm>


namespace studio::configurations
{
    text_game_config::text_game_config(const text_game_config&)
    {

    }

    std::vector<key_type> text_game_config::keys() const
    {
        std::vector<key_type> results;
        results.reserve(line_entries.size());

        std::transform(line_entries.begin(), line_entries.end(), std::back_insertor(results), [](auto& entry) { return entry.key_segments; });

        return results;
    }

    std::string_view text_game_config::find(key_type key) const
    {
        auto iter = std::find_if(line_entries.rbegin(), line_entries.rend(), [](auto& entry) { entry.key_segments == key; });

        if (iter == line_entries.rend())
        {
            return "";
        }

        return iter->value;
    }

    void text_game_config::emplace(key_type key, std::string_view value)
    {
        auto iter = std::find_if(line_entries.rbegin(), line_entries.rend(), [](auto& entry) { entry.key_segments == key; });
        
        if (iter != line_entries.rend())
        {
            iter->value = value;
        }
        else
        {
            line_entries.emplace_back(config_line{"", key, value });
        }
    }

    void text_game_config::remove(key_type key)
    {
        auto iter = std::remove_if(line_entries.begin(), line_entries.end(), [](auto& entry) { entry.key_segments == key; });

        line_entries.erase(iter, line_entries.end());
    }
}