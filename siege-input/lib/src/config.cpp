#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <optional>
#include <SDL.h>
#include "config.hpp"

inline SDL_GameControllerBindType from_string(std::string_view type)
{
  if (type == "axis")
  {
    return SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_AXIS;
  }

  if (type == "button")
  {
    return SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_BUTTON;
  }

  if (type == "hat")
  {
    return SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_HAT;
  }

  return SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_NONE;
}


auto parse_profile(std::filesystem::path file_name)
{
  std::ifstream profile_file(file_name);
  return nlohmann::json::parse(profile_file);
}

std::optional<stick_indexes> default_binding_for_primary_stick(const nlohmann::json& data)
{
  if (data.contains("numAxes"))
  {
    return std::nullopt;
  }

  stick_indexes result{};

  const auto num_axes = data["numAxes"].get<int>();

  if (num_axes >= 2)
  {
    result.x.index = 0;
    result.x.type = SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_AXIS;
    result.y.index = 1;
    result.y.type = SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_AXIS;
  }

  if (num_axes == 4)
  {
    result.twist.emplace(binding{
      2,
      SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_AXIS
    });
  }

  return result;
}

std::optional<throttle_indexes> default_binding_for_primary_throttle(const nlohmann::json& data)
{
  if (data.contains("numAxes"))
  {
    return std::nullopt;
  }

  throttle_indexes result{};

  const auto num_axes = data["numAxes"].get<int>();


  if (num_axes == 3)
  {
    result.y.index = 2;
    result.y.type = SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_AXIS;
  }

  if (num_axes >= 4)
  {
    result.y.index = 3;
    result.y.type = SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_AXIS;
  }

  return result;
}

std::optional<stick_indexes> binding_for_primary_stick(const nlohmann::json& data)
{
  if (!data.contains("bindings") && !data["bindings"].contains("axes"))
  {
    return std::nullopt;
  }

  const auto& axes = data["bindings"]["axes"];

  auto parse_stick = [&](const nlohmann::json& stick) -> std::optional<stick_indexes> {
    if (!stick.contains("x") && !stick.contains("y"))
    {
      return std::nullopt;
    }

    stick_indexes result{};

    result.x.index = stick["x"]["index"].get<std::size_t>();
    result.x.type = from_string(stick["x"]["type"].get<std::string>());
    result.y.index = stick["y"]["index"].get<std::size_t>();
    result.y.type = from_string(stick["y"]["type"].get<std::string>());

    if (stick.contains("twist"))
    {
      result.twist.emplace(binding {
        stick["twist"]["index"].get<std::size_t>(),
        from_string(stick["twist"]["type"].get<std::string>()) });
    }

    return result;
  };

  if (axes.contains("mainStick"))
  {
    return parse_stick(axes["mainStick"]);
  }

  if (axes.contains("analogueStick"))
  {
    return parse_stick(axes["analogueStick"]);
  }

  if (axes.contains("analogStick"))
  {
    return parse_stick(axes["analogStick"]);
  }

  return std::nullopt;
}

std::optional<throttle_indexes> binding_for_primary_throttle(const nlohmann::json& data)
{
  if (!data.contains("bindings") && !data["bindings"].contains("axes"))
  {
    return std::nullopt;
  }

  const auto& axes = data["bindings"]["axes"];

  auto parse_throttle = [&](const nlohmann::json& throttle) ->  std::optional<throttle_indexes> {
    if (!throttle.contains("y"))
    {
      return std::nullopt;
    }

    throttle_indexes result{};

    result.y.index = throttle["y"]["index"].get<std::size_t>();
    result.y.type = from_string(throttle["y"]["type"].get<std::string>());

    if (throttle.contains("miniRudder"))
    {
      result.mini_rudder.emplace( binding{ throttle["miniRudder"]["index"].get<std::size_t>(),
        from_string(throttle["miniRudder"]["type"].get<std::string>()) });
    }

    return result;
  };

  if (axes.contains("mainThrottle"))
  {
    auto result = parse_throttle(axes["mainThrottle"]);

    if (result.has_value() &&
        !result.value().mini_rudder.has_value() &&
        axes.contains("miniRudder") &&
        axes["miniRudder"].contains("location") &&
        axes["miniRudder"]["location"].contains("part") &&
        axes["miniRudder"]["location"]["part"] == "mainThrottle")
    {
      result.value().mini_rudder.emplace(binding{ axes["miniRudder"]["x"]["index"].get<std::size_t>(),
        from_string(axes["miniRudder"]["x"]["type"].get<std::string>()) });
    }

    return result;
  }

  if (axes.contains("miniThrottle"))
  {
    return parse_throttle(axes["miniThrottle"]);
  }

  return std::nullopt;
}

std::optional<binding> binding_for_primary_rudder(const nlohmann::json& data)
{
  if (!data.contains("bindings") && !data["bindings"].contains("axes"))
  {
    return std::nullopt;
  }

  std::optional<binding> result;

  auto stick_binding = binding_for_primary_stick(data);

  if (stick_binding.has_value() && stick_binding.value().twist.has_value())
  {
    result = stick_binding.value().twist;
  }

  if (!result.has_value())
  {
    auto throttle_binding = binding_for_primary_throttle(data);

    if (throttle_binding.has_value() && throttle_binding.value().mini_rudder.has_value())
    {
      result = throttle_binding.value().mini_rudder;
    }
  }

  if (result.has_value())
  {
    return result;
  }

  const auto& axes = data["bindings"]["axes"];

  if (axes.contains("pedals") && axes["pedals"].contains("rudder"))
  {
    return binding{
      axes["pedals"]["rudder"]["index"].get<std::size_t>(),
      from_string(axes["pedals"]["rudder"]["type"].get<std::string>())
    };
  }

  if (axes.contains("externalPedals") && axes["externalPedals"].contains("rudder"))
  {
    return binding{
      axes["externalPedals"]["rudder"]["index"].get<std::size_t>(),
      from_string(axes["externalPedals"]["rudder"]["type"].get<std::string>())
    };
  }

  if (axes.contains("rudder"))
  {
    return binding{
      axes["rudder"]["index"].get<std::size_t>(),
      from_string(axes["rudder"]["type"].get<std::string>())
    };
  }

  return std::nullopt;
}