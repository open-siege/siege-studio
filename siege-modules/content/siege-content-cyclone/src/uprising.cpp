#include <siege/configuration/uprising.hpp>
#include <vector>
#include <algorithm>

namespace siege::configuration::cyclone
{
  std::optional<uprising_1_settings> read_settings(std::istream& input)
  {
    uprising_1_settings result{};

    input.read(reinterpret_cast<char*>(&result), offsetof(uprising_1_settings, player_name));

    int32_le temp;
    input.read(reinterpret_cast<char*>(&temp), sizeof(temp));
    result.player_name.resize(temp);
    input.read(result.player_name.data(), result.player_name.size());

    input.read(reinterpret_cast<char*>(&temp), sizeof(temp));
    result.last_map.resize(temp);
    input.read(result.last_map.data(), result.last_map.size());

    constexpr auto remaining_size = offsetof(uprising_1_settings, sound_effects_volume) - offsetof(uprising_1_settings, last_map);
    input.read(reinterpret_cast<char*>(&result) + offsetof(uprising_1_settings, last_map), remaining_size);

    return std::make_optional(std::move(result));
  }

  void write_settings(std::ostream& output, const uprising_1_settings& data)
  {
    output.write(reinterpret_cast<const char*>(&data), offsetof(uprising_1_settings, player_name));

    int32_le temp = static_cast<std::int32_t>(data.player_name.size());
    output.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
    output.write(data.player_name.data(), data.player_name.size());

    temp = static_cast<std::int32_t>(data.last_map.size());
    output.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
    output.write(data.last_map.data(), data.last_map.size());

    constexpr auto remaining_size = offsetof(uprising_1_settings, sound_effects_volume) - offsetof(uprising_1_settings, last_map);
    output.write(reinterpret_cast<const char*>(&data) + offsetof(uprising_1_settings, last_map), remaining_size);
  }

  std::optional<uprising_1_key_map> read_key_map(std::istream& input)
  {
    uprising_1_key_map result{};

    std::vector<int32_le> mappings(result.keyboard_mappings.size());
    input.read(reinterpret_cast<char*>(mappings.data()), sizeof(int32_le) * mappings.size());

    std::transform(mappings.begin(), mappings.end(), result.keyboard_mappings.begin(), [](int32_le value) {
      return static_cast<uprising_1_key_map::action>(static_cast<std::int32_t>(value));
    });

    input.read(reinterpret_cast<char*>(&result.joystick_mappings), sizeof(result.joystick_mappings));

    return std::make_optional(std::move(result));
  }

  void write_key_map(std::ostream& output, const uprising_1_key_map& data)
  {
    std::vector<int32_le> mappings(data.keyboard_mappings.size());
    std::transform(data.keyboard_mappings.begin(), data.keyboard_mappings.end(), mappings.begin(), [](uprising_1_key_map::action value) {
      return static_cast<std::int32_t>(value);
    });

    output.write(reinterpret_cast<const char*>(mappings.data()), sizeof(int32_le) * mappings.size());
    output.write(reinterpret_cast<const char*>(&data.joystick_mappings), sizeof(data.joystick_mappings));
  }
}// namespace siege::configuration::cyclone