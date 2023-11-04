#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <optional>
#include <memory>
#include <SDL.h>
#include "platform/platform.hpp"
#include "config.hpp"
#include "joystick_info.hpp"

namespace siege
{

  constexpr static auto sdl_mapping = std::array<std::array<std::string_view, 2>, 14> {{
        { "leftstick", xbox::ls },
        { "rightstick", xbox::rs },
        { "leftshoulder", xbox::lb },
        { "rightshoulder", xbox::rb },
        { "dpup", common::d_pad_up },
        { "dpdown", common::d_pad_down },
        { "dpleft", common::d_pad_left },
        { "dpright", common::d_pad_right },
        { "leftx", common::left_x },
        { "lefty", common::left_y },
        { "rightx", common::right_x },
        { "righty", common::right_y },
        { "lefttrigger", xbox::lt },
        { "righttrigger", xbox::rt }
    }};

    constexpr std::string_view to_common(std::string_view value)
    {
        for (auto i = 0; i < sdl_mapping.size(); ++i)
        {
            if (sdl_mapping[i][0] == value)
            {
                return sdl_mapping[i][1];
            }
        }

        return to_xbox(value);
    }

    constexpr static auto a = std::string_view("a");
    constexpr static auto leftstick = std::string_view("leftstick");
    constexpr static auto leftx = std::string_view("leftx");

    static_assert(to_common(a) == xbox::a);
    static_assert(to_common(leftstick) == xbox::ls);
    static_assert(to_common(leftx) == common::left_x);

  std::vector<joystick_info> get_all_joysticks()
  {
    std::vector<joystick_info> joysticks;

    const auto num_joysticks = siege::NumJoysticks();

    joysticks.reserve(num_joysticks);

    for (auto i = 0; i < num_joysticks; ++i)
    {
      auto& info = joysticks.emplace_back();
      auto joystick = std::shared_ptr<Joystick>(siege::JoystickOpen(i), siege::JoystickClose);

      const char* joystick_name = siege::JoystickName(joystick.get());

      if (joystick_name != nullptr)
      {
        info.name = joystick_name;
      }

      auto joystick_type = siege::JoystickGetType(joystick.get());

      if (joystick_type == SDL_JoystickType::SDL_JOYSTICK_TYPE_GAMECONTROLLER)
      {
        info.type.emplace(common::types::game_controller);
      }
      else if (joystick_type == SDL_JoystickType::SDL_JOYSTICK_TYPE_FLIGHT_STICK)
      {
        info.type.emplace(common::types::flight_stick);
      }
      else if (joystick_type == SDL_JoystickType::SDL_JOYSTICK_TYPE_THROTTLE)
      {
        info.type.emplace(common::types::throttle);
      }
      else if (joystick_type == SDL_JoystickType::SDL_JOYSTICK_TYPE_WHEEL)
      {
        info.type.emplace(common::types::steering_wheel);
      }

      std::shared_ptr<SDL_GameController> controller;

      if (SDL_IsGameController(i) == SDL_bool::SDL_TRUE)
      {
        controller.reset(SDL_GameControllerOpen(i), SDL_GameControllerClose);

        const auto controller_type = SDL_GameControllerGetType(controller.get());

        if (controller_type == SDL_GameControllerType::SDL_CONTROLLER_TYPE_XBOX360 || controller_type == SDL_GameControllerType::SDL_CONTROLLER_TYPE_XBOX360)
        {
          info.controller_type.emplace(common::types::xbox);
        }
        else if (controller_type == SDL_GameControllerType::SDL_CONTROLLER_TYPE_XBOX360 || controller_type == SDL_GameControllerType::SDL_CONTROLLER_TYPE_XBOX360)
        {
          info.controller_type.emplace(common::types::playstation);
        }
      }

      std::optional<SDL_GameControllerButtonBind> left_trigger_binding;
      std::optional<SDL_GameControllerButtonBind> right_trigger_binding;

      if (controller && SDL_GameControllerHasAxis(controller.get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT) == SDL_bool::SDL_TRUE)
      {
          left_trigger_binding = SDL_GameControllerGetBindForAxis(controller.get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
      }

      if (controller && SDL_GameControllerHasAxis(controller.get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT) == SDL_bool::SDL_TRUE)
      {
          right_trigger_binding = SDL_GameControllerGetBindForAxis(controller.get(), SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
      }

      const auto num_buttons = siege::JoystickNumButtons(joystick.get());
      info.buttons.reserve(num_buttons);

      for (auto b = 0; b < num_buttons; ++b)
      {
        auto& button = info.buttons.emplace_back();

        button.is_left_trigger = left_trigger_binding.has_value() && 
        left_trigger_binding.value().bindType == SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_BUTTON &&
        left_trigger_binding.value().value.button == i;

        button.is_right_trigger = right_trigger_binding.has_value() && 
        right_trigger_binding.value().bindType == SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_BUTTON &&
        right_trigger_binding.value().value.button == i;

        if (controller)
        {
          for (auto bb = 0; bb < static_cast<int>(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_MAX); ++bb)
          {
              auto button_type = static_cast<SDL_GameControllerButton>(bb);
              if (SDL_GameControllerHasButton(controller.get(), button_type) == SDL_bool::SDL_TRUE)
              {
                  auto binding = SDL_GameControllerGetBindForButton(controller.get(), button_type);

                  if (binding.bindType == SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_BUTTON && binding.value.button == b)
                  {
                    if (info.controller_type.has_value() && info.controller_type.value() == common::types::xbox)
                    {
                        button.button_type.emplace(to_xbox(to_common(SDL_GameControllerGetStringForButton(button_type))));
                    }
                    else if(info.controller_type.has_value() && info.controller_type.value() == common::types::playstation)
                    {
                        button.button_type.emplace(to_playstation(to_common(SDL_GameControllerGetStringForButton(button_type))));
                    }

                    break;
                  }
              }
          }
        }
      }


      const auto num_axes = siege::JoystickNumAxes(joystick.get());
      info.axes.reserve(num_axes);

      for (auto a = 0; a < num_axes; ++a)
      {
        auto& axis = info.axes.emplace_back();

        if (controller)
        {
          for (auto aa = 0; aa < static_cast<int>(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_MAX); ++aa)
          {
              auto axis_type = static_cast<SDL_GameControllerAxis>(aa);
              if (SDL_GameControllerHasAxis(controller.get(), axis_type) == SDL_bool::SDL_TRUE)
              {
                  auto binding = SDL_GameControllerGetBindForAxis(controller.get(), axis_type);

                  if (binding.bindType == SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_AXIS && binding.value.axis == a)
                  {
                    if (info.controller_type.has_value() && info.controller_type.value() == common::types::xbox)
                    {
                        axis.axis_type.emplace(to_xbox(to_common(SDL_GameControllerGetStringForAxis(axis_type))));
                    }
                    else if(info.controller_type.has_value() && info.controller_type.value() == common::types::playstation)
                    {
                        axis.axis_type.emplace(to_playstation(to_common(SDL_GameControllerGetStringForAxis(axis_type))));
                    }
                    break;
                  }
              }
          }
        }
      }

      std::optional<SDL_GameControllerButtonBind> d_pad_up_binding;
      std::optional<SDL_GameControllerButtonBind> d_pad_down_binding;
      std::optional<SDL_GameControllerButtonBind> d_pad_left_binding;
      std::optional<SDL_GameControllerButtonBind> d_pad_right_binding;

      if (controller && SDL_GameControllerHasButton(controller.get(), SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP) == SDL_bool::SDL_TRUE)
      {
          d_pad_up_binding = SDL_GameControllerGetBindForButton(controller.get(), SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP);
      }

      if (controller && SDL_GameControllerHasButton(controller.get(), SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN) == SDL_bool::SDL_TRUE)
      {
          d_pad_down_binding = SDL_GameControllerGetBindForButton(controller.get(), SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN);
      }

      if (controller && SDL_GameControllerHasButton(controller.get(), SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT) == SDL_bool::SDL_TRUE)
      {
          d_pad_left_binding = SDL_GameControllerGetBindForButton(controller.get(), SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT);
      }

      if (controller && SDL_GameControllerHasButton(controller.get(), SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT) == SDL_bool::SDL_TRUE)
      {
          d_pad_right_binding = SDL_GameControllerGetBindForButton(controller.get(), SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
      }

      const auto num_hats = siege::JoystickNumHats(joystick.get());
      info.hats.reserve(num_hats);

      for (auto b = 0; b < num_hats; ++b)
      {
        auto& hat = info.hats.emplace_back();

        hat.is_controller_dpad = d_pad_up_binding.has_value() && 
        d_pad_down_binding.has_value() &&
        d_pad_left_binding.has_value() &&
        d_pad_right_binding.has_value() &&
        d_pad_up_binding.value().bindType == SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_HAT &&
        d_pad_up_binding.value().value.hat.hat == i &&
        d_pad_down_binding.value().bindType == SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_HAT &&
        d_pad_down_binding.value().value.hat.hat == i &&
        d_pad_left_binding.value().bindType == SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_HAT &&
        d_pad_left_binding.value().value.hat.hat == i &&
        d_pad_right_binding.value().bindType == SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_HAT &&
        d_pad_right_binding.value().value.hat.hat == i;
      }

    }


    return joysticks;
  }

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
    if (!data.contains("numAxes"))
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
    if (!data.contains("numAxes"))
    {
      return std::nullopt;
    }

    const auto num_axes = data["numAxes"].get<int>();

    if (num_axes < 3)
    {
      return std::nullopt;
    }

    throttle_indexes result{};

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

  std::optional<binding> default_binding_for_primary_rudder(const nlohmann::json& data)
  {
    if (!data.contains("numAxes"))
    {
      return std::nullopt;
    }

    const auto num_axes = data["numAxes"].get<int>();

    if (num_axes <= 3)
    {
      return std::nullopt;
    }

    binding result{};

    if (num_axes >= 4)
    {
      result.index = 2;
      result.type = SDL_GameControllerBindType::SDL_CONTROLLER_BINDTYPE_AXIS;
    }

    return result;
  }

  std::optional<stick_indexes> binding_for_primary_stick(const nlohmann::json& data)
  {
    if (!data.contains("bindings"))
    {
      return std::nullopt;
    }

    if (!data["bindings"].contains("axes"))
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
    if (!data.contains("bindings"))
    {
      return std::nullopt;
    }

    if (!data["bindings"].contains("axes"))
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
    if (!data.contains("bindings"))
    {
      return std::nullopt;
    }

    if (!data["bindings"].contains("axes"))
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
}