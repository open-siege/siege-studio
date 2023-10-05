#include "configurations/shared.hpp"
#include <algorithm>
#include "endian_arithmetic.hpp"

namespace studio::configurations
{
    namespace endian = boost::endian;

    std::uint32_t little_endian_stream_reader::read_uint32()
    {
        endian::little_uint32_t result;
        stream.read(reinterpret_cast<char*>(&result), sizeof(result));
        return result;
    }

    std::uint8_t little_endian_stream_reader::read_uint8()
    {
        endian::little_uint8_t result;
        stream.read(reinterpret_cast<char*>(&result), sizeof(result));
        return result;
    }

    void little_endian_stream_writer::write(const std::uint32_t& value)
    {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void little_endian_stream_writer::write(const std::uint8_t& value)
    {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void little_endian_stream_writer::write(const std::string_view value)
    {
        stream.write(value.data(), value.size());
    }

    void write(little_endian_stream_writer& writer, const config_value& value)
    {
        if (value.has<std::uint32_t>())
        {
            writer.write(value.get<std::uint32_t>());
        }

        if (value.has<std::uint8_t>())
        {
            writer.write(value.get<std::uint8_t>());
        }

        if (value.has<std::string>())
        {
            writer.write(value.get<std::string>());
        }
    }

    text_game_config::text_game_config(const text_game_config&)
    {
        // TODO copy data without invalidating string views
    }

    text_game_config::text_game_config(std::string&& raw, std::vector<config_line>&& entries) 
        : raw_data(raw), line_entries(entries)
    {
    }

    std::vector<key_type> text_game_config::keys() const
    {
        std::vector<key_type> results;
        results.reserve(line_entries.size());

        std::transform(line_entries.begin(), line_entries.end(), std::back_inserter(results), [](auto& entry) { return entry.key_segments; });

        return results;
    }

    std::string_view text_game_config::find(key_type key) const
    {
        auto iter = std::find_if(line_entries.rbegin(), line_entries.rend(), [&](auto& entry) { return entry.key_segments == key; });

        if (iter == line_entries.rend())
        {
            return "";
        }

        return iter->value;
    }

    void text_game_config::emplace(key_type key, std::string_view value)
    {
        auto iter = std::find_if(line_entries.rbegin(), line_entries.rend(), [&](auto& entry) { return entry.key_segments == key; });
        
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
        auto iter = std::remove_if(line_entries.begin(), line_entries.end(), [&](auto& entry) { return entry.key_segments == key; });

        line_entries.erase(iter, line_entries.end());
    }

    void text_game_config::persist(std::function<void(const std::vector<config_line>&)> func) const
    {
        if (func)
        {
            func(line_entries);
        }
    }

    binary_game_config::binary_game_config(const binary_game_config&)
    {
        // TODO implement or default
    }

    binary_game_config::binary_game_config(std::vector<config_entry>&& entries) 
        : entries(entries)
    {
    }

    std::vector<std::string_view> binary_game_config::keys() const
    {
        std::vector<std::string_view> results;
        results.reserve(entries.size());

        std::transform(entries.begin(), entries.end(), std::back_inserter(results), [](auto& entry) { return entry.key; });

        return results;
    }

    std::optional<config_value> binary_game_config::find(std::string_view key) const
    {
        auto iter = std::find_if(entries.rbegin(), entries.rend(), [&](auto& entry) { return entry.key == key; });

        if (iter == entries.rend())
        {
            return std::nullopt;
        }

        return std::make_optional<config_value>(iter->value);
    }

    void binary_game_config::emplace(std::string_view key, config_value value)
    {
        auto iter = std::find_if(entries.rbegin(), entries.rend(), [&](auto& entry) { return entry.key == key; });
        
        if (iter != entries.rend())
        {
            iter->value = value;
        }
        else
        {
            entries.emplace_back(config_entry{key, value});
        }
    }

    void binary_game_config::remove(std::string_view key)
    {
        auto iter = std::remove_if(entries.begin(), entries.end(), [&](auto& entry) { return entry.key == key; });

        entries.erase(iter, entries.end());
    }

    void binary_game_config::persist(std::function<void(const std::vector<config_entry>&)> func) const
    {
        if (func)
        {
            func(entries);
        }
    }

}