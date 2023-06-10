#ifndef OPEN_SIEGE_CONFIG_HPP
#define OPEN_SIEGE_CONFIG_HPP

#include <optional>
#include <nlohmann/json.hpp>
#include <SDL.h>

namespace siege
{

  struct binding
  {
    std::size_t index;
    SDL_GameControllerBindType type;
  };

  struct stick_indexes
  {
    binding x;
    binding y;
    std::optional<binding> twist;
  };

  struct throttle_indexes
  {
    binding y;
    std::optional<binding> mini_rudder;
  };

  std::optional<stick_indexes> default_binding_for_primary_stick(const nlohmann::json& data);

  std::optional<throttle_indexes> default_binding_for_primary_throttle(const nlohmann::json& data);

  std::optional<binding> default_binding_for_primary_rudder(const nlohmann::json& data);

  std::optional<stick_indexes> binding_for_primary_stick(const nlohmann::json& data);

  std::optional<throttle_indexes> binding_for_primary_throttle(const nlohmann::json& data);

  std::optional<binding> binding_for_primary_rudder(const nlohmann::json& data);
}

#endif// OPEN_SIEGE_CONFIG_HPP
