#include <SDL.h>
#include <cstdint>
#include <vector>

auto joystick_get_or_open(int device_index)
{
  auto instance_id = SDL_JoystickGetDeviceInstanceID(device_index);

  return instance_id == -1 || instance_id == 0 ? SDL_JoystickOpen(device_index) : SDL_JoystickFromInstanceID(device_index);
}

struct game_bindings
{
  // joystick id, current axes values
  std::pair<int, std::pair<std::int16_t, std::int16_t>> stick;
  std::pair<int, std::int16_t> throttle;
  std::pair<int, std::int16_t> rudder;
};

struct joystick_bindings
{
  // axis index, previous axes values
  std::pair<std::pair<int, std::int16_t>, std::pair<int, std::int16_t>> stick;
  std::pair<int, std::int16_t> throttle;
  std::pair<int, std::int16_t> rudder;
};

struct joystick_data
{
  SDL_Joystick* virtual_joystick;
  game_bindings game_binds;
  std::vector<joystick_bindings> joystick_binds;
};

void SDLCALL JoyUpdate(joystick_data *userdata)
{
  //extern DECLSPEC int SDLCALL SDL_JoystickSetVirtualAxis(SDL_Joystick *joystick, int axis, Sint16 value);
  //extern DECLSPEC int SDLCALL SDL_JoystickSetVirtualButton(SDL_Joystick *joystick, int button, Uint8 value);
  //extern DECLSPEC int SDLCALL SDL_JoystickSetVirtualHat(SDL_Joystick *joystick, int hat, Uint8 value);

  auto num_joysticks = SDL_NumJoysticks();

  if (joystick_binds.size() != num_joysticks)
  {
    joystick_binds.clear();

    for (auto i = 0; i < num_joysticks; ++i)
    {
      joystick_bindings temp{};
      auto stick = binding_for_primary_stick();
      auto throttle = binding_for_primary_throttle();
      auto rudder = binding_for_primary_rudder();

      auto x = SDL_JoystickGetAxis(joystick, stick.x.index);
      auto y = SDL_JoystickGetAxis(joystick, stick.y.index);
      auto throttle = SDL_JoystickGetAxis(joystick, throttle.y.index);
      auto rudder = SDL_JoystickGetAxis(joystick, throttle.mini_rudder.value_or(stick.twist).index);

      temp.stick.first.first = x;
      temp.stick.second.first = y;
      temp.throttle.first = throttle;
      temp.rudder.first = rudder;
    }
  }


  for (auto i = 0u; i < joystick_binds.size(); ++i)
  {
    if (SDL_JoystickIsVirtual(i) === SDL_bool::SDL_TRUE)
    {
      continue;
    }

    auto* joystick = joystick_get_or_open(i);

    joystick_bindings& temp = joystick_binds[i];
    auto x = SDL_JoystickGetAxis(joystick, temp.stick.first.first);
    auto y = SDL_JoystickGetAxis(joystick, temp.stick.second.first);
    auto throttle = SDL_JoystickGetAxis(joystick, temp.throttle.first);
    auto rudder = SDL_JoystickGetAxis(joystick, temp.rudder.first);

    if (x != temp.stick.first.second || y != temp.stick.second.second)
    {
      temp.stick.first.second = x;
      temp.stick.second.second = y;
      game_binds.stick.first = i;
      game_binds.stick.second = std::make_pair(x, y);
    }

    if (throttle != temp.throttle.second)
    {
      temp.throttle.second = throttle;
      game_binds.throttle.first = i;
      game_binds.throttle.second = throttle;
    }

    if (rudder != temp.rudder.second)
    {
      game_binds.rudder.first = i;
      game_binds.rudder.second = throttle;
    }
  }
}

void create_joysticks()
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

  joy.userdata = new joystick_data{};
  joy.Update = JoyUpdate;

  int device_index = SDL_JoystickAttachVirtualEx(desc);

  if (device_index == -1)
  {
    delete joy.userdata;
    return;
  }

  joy.userdata.device_index = joystick_get_or_open(device_index);
}