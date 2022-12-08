#include <limits>
#include <array>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CINTERFACE
#include <windows.h>
#include <joystickapi.h>
#include <memory>
#include <fstream>
#include "winmm.hpp"

#include <platform/platform.hpp>
#include <virtual_joystick.hpp>

inline auto joystick_get_or_open(int device_index)
{
  auto instance_id = siege::JoystickGetDeviceInstanceID(device_index);

  return instance_id == -1 || instance_id == 0 ? siege::JoystickOpen(device_index) : siege::JoystickFromInstanceID(instance_id);
}

namespace winmm
{
  enum class joystickresult_t : UINT
  {
    no_error = JOYERR_NOERROR,
    bad_joystick_id = JOYERR_PARMS,
    unplugged = JOYERR_UNPLUGGED,
    bad_device_id = MMSYSERR_BADDEVICEID,
    no_driver = MMSYSERR_NODRIVER,
    invalid_param = MMSYSERR_INVALPARAM
  };

  //These functions count as being a part of DirectX 1 and 2.
  static auto* TrueJoyGetNumDevs = joyGetNumDevs;
  static auto* TrueJoyGetDevCapsA = joyGetDevCapsA;
  static auto* TrueJoyGetPos = joyGetPos;
  static auto* TrueJoyGetPosEx = joyGetPosEx;
  static auto* TrueJoySetThreshold = joySetThreshold;

  void init_joysticks()
  {
    if (siege::WasInit(SDL_INIT_JOYSTICK) & SDL_INIT_JOYSTICK)
    {
      return;
    }

    auto result = siege::InitSubSystem(SDL_INIT_JOYSTICK);
    if (result == 0)
    {
      std::ofstream log("darkstar.winmm.log", std::ios::trunc);
      log << "SDL Joystick sub system init\n";
      siege::JoystickUpdate();
      siege::InitVirtualJoysticksFromJoysticks();
    }
  }

  UINT WINAPI DarkJoyGetNumDevs()
  {
    init_joysticks();
    std::ofstream log("darkstar.winmm.log", std::ios::app);

    auto result = siege::NumJoysticks();
    log << "DarkJoyGetNumDevs: " << result << '\n';

    log << "DarkJoyGetDevCapsA returning \n";
    return result;
  }

  joystickresult_t WINAPI DarkJoyGetDevCapsA(
    UINT_PTR joy_id,
    LPJOYCAPSA caps,
    UINT size
    )
  {
    std::ofstream log("darkstar.winmm.log", std::ios::app);
    log << "DarkJoyGetDevCapsA: joy_id=" << joy_id << '\n';

    if (!caps || size < sizeof(JOYCAPSA))
    {
      return joystickresult_t::invalid_param;
    }
    init_joysticks();

    if (joy_id == -1)
    {
      joy_id = 0;
    }

    if (joy_id > std::size_t(siege::NumJoysticks() - 1))
    {
      return joystickresult_t::no_driver;
    }

    joy_id = siege::NumJoysticks() - 1 - joy_id;

    log << "Actual joystick ID " << joy_id << '\n';

    auto temp = joystick_get_or_open(int(joy_id));

    if (!temp)
    {
      log << "Could not open joystick" << joy_id << '\n';
      return joystickresult_t::no_driver;
    }

    *caps = JOYCAPSA{};
    caps->wCaps = JOYCAPS_HASZ | JOYCAPS_HASR | JOYCAPS_HASPOV;
    caps->wMid = siege::JoystickGetVendor(temp);
    caps->wPid = siege::JoystickGetProduct(temp);
    caps->wMaxAxes = siege::JoystickNumAxes(temp);
    caps->wNumAxes = siege::JoystickNumAxes(temp);
    caps->wMaxButtons = siege::JoystickNumButtons(temp);
    caps->wNumButtons = siege::JoystickNumButtons(temp);
    caps->wXmin = 0;
    caps->wXmax = std::numeric_limits<std::uint16_t>::max();
    caps->wYmin = 0;
    caps->wYmax = std::numeric_limits<std::uint16_t>::max();
    caps->wZmin = 0;
    caps->wZmax = std::numeric_limits<std::uint16_t>::max();
    caps->wRmin = 0;
    caps->wRmax = std::numeric_limits<std::uint16_t>::max();
    log << "Found" << siege::JoystickName(temp) << '\n';
    auto length = std::strlen(siege::JoystickName(temp)) > sizeof(caps->szPname) ? sizeof(caps->szPname) : std::strlen(siege::JoystickName(temp));
    std::memcpy(caps->szPname, siege::JoystickName(temp), length);

    log << "DarkJoyGetDevCapsA returning \n";
    return joystickresult_t::no_error;
  }

  joystickresult_t WINAPI DarkJoyGetPos(
    UINT joy_id,
    LPJOYINFO info
  )
  {
    std::ofstream log("darkstar.winmm.log", std::ios::app);
    log << "DarkJoyGetPosEx: joy_id=" << joy_id << '\n';

    log.flush();

    if (!info)
    {
      return joystickresult_t::invalid_param;
    }

    init_joysticks();

    *info = JOYINFO{};

    joy_id = siege::NumJoysticks() - 1 - joy_id;

    auto temp = joystick_get_or_open(int(joy_id));

    if (!temp)
    {
      return joystickresult_t::unplugged;
    }

    siege::JoystickUpdate();

    info->wXpos = WORD(siege::JoystickGetAxis(temp, 0));
    info->wYpos = WORD(siege::JoystickGetAxis(temp, 1));
    info->wZpos = WORD(siege::JoystickGetAxis(temp, 2));

    constexpr static std::array<DWORD, 4> buttons = {
      JOY_BUTTON1,
      JOY_BUTTON2,
      JOY_BUTTON3,
      JOY_BUTTON4
    };

    for (auto i = 0u; i < buttons.size(); ++i)
    {
      if (siege::JoystickGetButton(temp, i))
      {
        info->wButtons |= buttons[i];
      }
    }

    return joystickresult_t::no_error;
  }

  joystickresult_t WINAPI DarkJoyGetPosEx(
    UINT joy_id,
    LPJOYINFOEX info
  )
  {
    std::ofstream log("darkstar.winmm.log", std::ios::app);
    log << "DarkJoyGetPosEx: joy_id=" << joy_id << '\n';

    if (!info || info->dwSize < sizeof(JOYINFOEX))
    {
      if (info)
      {
        log << "Info is the wrong size for " << joy_id << ". Reported size: " << info->dwSize << '\n';
      }
      else
      {
        log << "Info not correct for " << joy_id << '\n';
      }

      return joystickresult_t::invalid_param;
    }

    init_joysticks();

    joy_id = siege::NumJoysticks() - 1 - joy_id;

    log << "Actual joystick ID " << joy_id << '\n';

    auto temp = joystick_get_or_open(int(joy_id));

    if (!temp)
    {
      return joystickresult_t::unplugged;
    }

    const auto flags = info->dwFlags;

    *info = JOYINFOEX{};

    siege::JoystickUpdate();

    log << "Flags " << flags << '\n';

    log << "joy_id stick: " << siege::JoystickGetAxis(temp, 0) << " " << siege::JoystickGetAxis(temp, 1) << '\n';

    log << "joy_id rudder: " << siege::JoystickGetAxis(temp, 2) << " " << DWORD(siege::JoystickGetAxis(temp, 2)) << '\n';
    log << "joy_id throttle: " << siege::JoystickGetAxis(temp, 3) << " " << DWORD(siege::JoystickGetAxis(temp, 3)) <<  '\n';

    info->dwXpos = int(siege::JoystickGetAxis(temp, 0)) + std::numeric_limits<std::int16_t>::max();
    info->dwYpos = int(siege::JoystickGetAxis(temp, 1)) + std::numeric_limits<std::int16_t>::max();
    info->dwRpos = int(siege::JoystickGetAxis(temp, 2)) + std::numeric_limits<std::int16_t>::max();
    info->dwZpos = int(siege::JoystickGetAxis(temp, 3)) + std::numeric_limits<std::int16_t>::max();


    info->dwSize = sizeof(JOYINFOEX);
    info->dwFlags = flags;

    constexpr static std::array<DWORD, 32> buttons = {
      JOY_BUTTON1,
      JOY_BUTTON2,
      JOY_BUTTON3,
      JOY_BUTTON4,
      JOY_BUTTON5,
      JOY_BUTTON6,
      JOY_BUTTON7,
      JOY_BUTTON8,
      JOY_BUTTON9,
      JOY_BUTTON10,
      JOY_BUTTON11,
      JOY_BUTTON12,
      JOY_BUTTON13,
      JOY_BUTTON14,
      JOY_BUTTON15,
      JOY_BUTTON16,
      JOY_BUTTON17,
      JOY_BUTTON18,
      JOY_BUTTON19,
      JOY_BUTTON20,
      JOY_BUTTON21,
      JOY_BUTTON22,
      JOY_BUTTON23,
      JOY_BUTTON24,
      JOY_BUTTON25,
      JOY_BUTTON26,
      JOY_BUTTON27,
      JOY_BUTTON28,
      JOY_BUTTON29,
      JOY_BUTTON30,
      JOY_BUTTON31,
      JOY_BUTTON32
    };

    for (auto i = 0u; i < buttons.size(); ++i)
    {
      if (siege::JoystickGetButton(temp, i))
      {
        info->dwButtons |= buttons[i];
      }
    }

    auto pov = siege::JoystickGetHat(temp, 0);

    info->dwPOV = JOY_POVCENTERED;

    if (pov == SDL_HAT_UP)
    {
      info->dwPOV = JOY_POVFORWARD;
    }
    else if (pov == SDL_HAT_DOWN)
    {
      info->dwPOV = JOY_POVBACKWARD;
    }
    else if (pov == SDL_HAT_LEFT)
    {
      info->dwPOV = JOY_POVLEFT;
    }
    else if (pov == SDL_HAT_LEFTUP)
    {
      info->dwPOV = JOY_POVLEFT + JOY_POVRIGHT / 2;
    }
    else if (pov == SDL_HAT_LEFTDOWN)
    {
      info->dwPOV = JOY_POVLEFT - JOY_POVRIGHT / 2;
    }
    else if (pov == SDL_HAT_RIGHT)
    {
      info->dwPOV = JOY_POVRIGHT;
    }
    else if (pov == SDL_HAT_RIGHTUP)
    {
      info->dwPOV = JOY_POVRIGHT / 2;
    }
    else if (pov == SDL_HAT_RIGHTDOWN)
    {
      info->dwPOV = JOY_POVBACKWARD - JOY_POVRIGHT / 2;
    }

    return joystickresult_t::no_error;
  }

  joystickresult_t WINAPI DarkJoySetThreshold(
    UINT,
    UINT
  )
  {
    return joystickresult_t::no_error;
  }

  std::array<std::pair<void**, void*>, 5> GetWinmmDetours()
  {
    return std::array<std::pair<void**, void*>, 5>{ {
      { &(void*&)TrueJoyGetNumDevs, DarkJoyGetNumDevs },
      { &(void*&)TrueJoyGetDevCapsA, DarkJoyGetDevCapsA },
      { &(void*&)TrueJoyGetPos, DarkJoyGetPos },
      { &(void*&)TrueJoyGetPosEx, DarkJoyGetPosEx },
      { &(void*&)TrueJoySetThreshold, DarkJoySetThreshold } }
    };
  }
}
