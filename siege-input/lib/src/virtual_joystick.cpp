#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <SDL.h>


#include "config.hpp"
#include "platform/platform.hpp"

namespace fs = std::filesystem;

auto joystick_get_or_open(int device_index)
{
  auto instance_id = SDL_JoystickGetDeviceInstanceID(device_index);

  return instance_id == -1 || instance_id == 0 ? SDL_JoystickOpen(device_index) : SDL_JoystickFromInstanceID(instance_id);
}

struct game_axis
{
  int target_axis_index;
  int joystick_id;
  std::int16_t current_value;
};

struct game_bindings
{
  game_axis stick_x;
  game_axis stick_y;
  game_axis rudder;
  game_axis throttle;
};

struct joystick_axis
{
  int axis_index;
  std::int16_t previous_value;
};

struct joystick_bindings
{
  joystick_axis stick_x;
  joystick_axis stick_y;
  joystick_axis rudder;
  joystick_axis throttle;
};

struct joystick_data
{
  SDL_Joystick* virtual_joystick;
  game_bindings game_binds;
  std::vector<joystick_bindings> joystick_binds;
  std::unordered_map<std::string, nlohmann::json> profiles;
};

void SDLCALL JoyUpdate(void* userdata)
{
  auto* real_data = reinterpret_cast<joystick_data*>(userdata);

  auto& joystick_binds = real_data->joystick_binds;
  auto* virtual_joystick = real_data->virtual_joystick;
  game_bindings& game_binds = real_data->game_binds;

  auto num_joysticks = SDL_NumJoysticks();

  if (joystick_binds.size() != num_joysticks)
  {
    joystick_binds.clear();

    for (auto i = 0; i < num_joysticks; ++i)
    {
      joystick_bindings& temp = joystick_binds.emplace_back();

      std::unique_ptr<SDL_Joystick, void(*)(SDL_Joystick*)> real_joystick{ SDL_JoystickOpen(i), SDL_JoystickClose};

      auto joystick_guid = to_string(SDL_JoystickGetGUID(real_joystick.get()));

      auto profile = real_data->profiles.find(joystick_guid);

      if (profile == real_data->profiles.end())
      {
        if (SDL_JoystickIsVirtual(i) == SDL_bool::SDL_TRUE)
        {
          continue;
        }
        profile = real_data->profiles.emplace(joystick_guid, joystick_to_json(real_joystick.get(), i)).first;
      }

      auto stick = binding_for_primary_stick(profile->second);
      auto throttle_indexes = binding_for_primary_throttle(profile->second);
      auto rudder_indexes = binding_for_primary_rudder(profile->second);

      if (!stick.has_value())
      {
        stick = default_binding_for_primary_stick(profile->second);
      }

      if (!throttle_indexes.has_value())
      {
        throttle_indexes = default_binding_for_primary_throttle(profile->second);
      }

      if (stick.has_value())
      {
        temp.stick_x.axis_index = int(stick.value().x.index);
        temp.stick_x.previous_value = SDL_JoystickGetAxis(real_joystick.get(), temp.stick_x.axis_index);
        temp.stick_y.axis_index = int(stick.value().y.index);
        temp.stick_y.previous_value = SDL_JoystickGetAxis(real_joystick.get(), temp.stick_y.axis_index);
      }

      if (throttle_indexes.has_value())
      {
        temp.throttle.axis_index = int(throttle_indexes.value().y.index);
        temp.throttle.previous_value = SDL_JoystickGetAxis(real_joystick.get(), temp.throttle.axis_index);
      }

      if (rudder_indexes.has_value())
      {
        temp.rudder.axis_index = int(rudder_indexes.value().index);
        temp.rudder.previous_value = SDL_JoystickGetAxis(real_joystick.get(), temp.rudder.axis_index);
      }
    }
  }

  for (auto i = 0u; i < joystick_binds.size(); ++i)
  {
    if (SDL_JoystickIsVirtual(i) == SDL_bool::SDL_TRUE)
    {
      continue;
    }

    auto* joystick = joystick_get_or_open(i);


    if (!joystick)
    {
      continue;
    }

    joystick_bindings& temp = joystick_binds[i];
    auto x = SDL_JoystickGetAxis(joystick, temp.stick_x.axis_index);
    auto y = SDL_JoystickGetAxis(joystick, temp.stick_y.axis_index);
    auto throttle = SDL_JoystickGetAxis(joystick, temp.throttle.axis_index);
    auto rudder = SDL_JoystickGetAxis(joystick, temp.rudder.axis_index);

    if (x != temp.stick_x.previous_value || y != temp.stick_y.previous_value)
    {
      temp.stick_x.previous_value = x;
      temp.stick_x.previous_value = y;
      game_binds.stick_x.joystick_id = i;
      game_binds.stick_y.joystick_id = i;
      game_binds.stick_x.current_value = x;
      game_binds.stick_y.current_value = y;
    }

    if (throttle != temp.throttle.previous_value)
    {
      temp.throttle.previous_value = throttle;
      game_binds.throttle.joystick_id = i;
      game_binds.throttle.current_value = throttle;
    }

    if (rudder != temp.rudder.previous_value)
    {
      game_binds.rudder.joystick_id = i;
      game_binds.rudder.current_value = rudder;
    }
  }

  SDL_JoystickSetVirtualAxis(virtual_joystick,
    game_binds.stick_x.target_axis_index,
    game_binds.stick_x.current_value);

  SDL_JoystickSetVirtualAxis(virtual_joystick,
    game_binds.stick_y.target_axis_index,
    game_binds.stick_y.current_value);

  SDL_JoystickSetVirtualAxis(virtual_joystick,
    game_binds.throttle.target_axis_index,
    game_binds.throttle.current_value);

  SDL_JoystickSetVirtualAxis(virtual_joystick,
    game_binds.rudder.target_axis_index,
    game_binds.rudder.current_value);
  //extern DECLSPEC int SDLCALL SDL_JoystickSetVirtualButton(SDL_Joystick *joystick, int button, Uint8 value);
  //extern DECLSPEC int SDLCALL SDL_JoystickSetVirtualHat(SDL_Joystick *joystick, int hat, Uint8 value);
}

void Siege_InitVirtualJoysticksFromJoysticks()
{
  SDL_VirtualJoystickDesc joy{};

  joy.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
  joy.type = SDL_JoystickType::SDL_JOYSTICK_TYPE_FLIGHT_STICK;
  joy.naxes = 4;
  joy.nbuttons = 8;
  joy.nhats = 1;
  joy.vendor_id = 0x1234;
  joy.product_id = 0x9876;
  joy.name = "Virtual Composite Joystick";

  auto temp_data = new joystick_data{};
  joy.userdata = temp_data;
  joy.Update = JoyUpdate;

  int device_index = SDL_JoystickAttachVirtualEx(&joy);

  if (device_index == -1)
  {
    delete temp_data;
    return;
  }

  const auto profiles_path = fs::path("profiles") / fs::path("devices");

  if (fs::exists(profiles_path))
  {
    for (const fs::directory_entry& dir_entry :
      fs::recursive_directory_iterator(profiles_path))
    {
      if (!dir_entry.is_directory() && dir_entry.path().extension() == ".json")
      {
        try
        {
          std::ifstream profile_stream{dir_entry.path(), std::ios::binary};
          auto profile_data = nlohmann::json::parse(profile_stream);

          if (profile_data.contains("deviceGuid"))
          {
            temp_data->profiles.emplace(profile_data["deviceGuid"].get<std::string>(), profile_data);
          }
        }
        catch(...)
        {

        }
      }
    }
  }

  temp_data->virtual_joystick = SDL_JoystickOpen(device_index);
  temp_data->game_binds = {
    {
      0, 0, 0
    },
    {
      1, 0, 0
    }, {
      2, 0, 0
    }, {
      3, 0, 0
    }
  };
}