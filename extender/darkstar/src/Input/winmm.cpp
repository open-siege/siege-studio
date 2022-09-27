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

  static auto* TrueJoyGetNumDevs = joyGetNumDevs;
  static auto* TrueJoyGetDevCapsA = joyGetDevCapsA;
  static auto* TrueJoyGetPos = joyGetPos;
  static auto* TrueJoyGetPosEx = joyGetPosEx;
  static auto* TrueJoySetThreshold = joySetThreshold;

  void init_joysticks()
  {
    if (SDL_WasInit(SDL_INIT_JOYSTICK) & SDL_INIT_JOYSTICK)
    {
      return;
    }

    auto result = SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    if (result == 0)
    {
      //joystick_subsystem.reset(reinterpret_cast<void*>(SDL_INIT_JOYSTICK), [](void*){ SDL_QuitSubSystem(SDL_INIT_JOYSTICK);});
      std::ofstream log("darkstar.winmm.log", std::ios::trunc);
      log << "SDL Joystick sub system init\n";
      SDL_JoystickUpdate();
    }
  }

  UINT WINAPI DarkJoyGetNumDevs()
  {
    init_joysticks();
    std::ofstream log("darkstar.winmm.log", std::ios::app);

    auto result = SDL_NumJoysticks();
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

    if (joy_id > std::size_t(SDL_NumJoysticks() - 1))
    {
      return joystickresult_t::no_driver;
    }

    auto temp = SDL_JoystickOpen(int(joy_id));

    if (!temp)
    {
      log << "Could not open joystick" << joy_id << '\n';
      return joystickresult_t::no_driver;
    }

    *caps = JOYCAPSA{};
    caps->wCaps = JOYCAPS_HASZ | JOYCAPS_HASR | JOYCAPS_HASPOV;
    caps->wMid = SDL_JoystickGetVendor(temp);
    caps->wPid = SDL_JoystickGetProduct(temp);
    caps->wMaxAxes = SDL_JoystickNumAxes(temp);
    caps->wNumAxes = SDL_JoystickNumAxes(temp);
    caps->wMaxButtons = SDL_JoystickNumButtons(temp);
    caps->wNumButtons = SDL_JoystickNumButtons(temp);
    caps->wXmin = 0;
    caps->wXmax = std::numeric_limits<std::uint16_t>::max();
    caps->wYmin = 0;
    caps->wYmax = std::numeric_limits<std::uint16_t>::max();
    caps->wZmin = 0;
    caps->wZmax = std::numeric_limits<std::uint16_t>::max();
    caps->wRmin = 0;
    caps->wRmax = std::numeric_limits<std::uint16_t>::max();
    log << "Found" << SDL_JoystickName(temp) << '\n';
    auto length = std::strlen(SDL_JoystickName(temp)) > sizeof(caps->szPname) ? sizeof(caps->szPname) : std::strlen(SDL_JoystickName(temp));
    std::memcpy(caps->szPname, SDL_JoystickName(temp), length);

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


    SDL_JoystickUpdate();

    auto temp = SDL_JoystickOpen(int(joy_id));

    if (!temp)
    {
      return joystickresult_t::unplugged;
    }

    info->wXpos = WORD(SDL_JoystickGetAxis(temp, 0));
    info->wYpos = WORD(SDL_JoystickGetAxis(temp, 1));
    info->wZpos = WORD(SDL_JoystickGetAxis(temp, 2));

    if (SDL_JoystickGetButton(temp, 0))
    {
        info->wButtons |= JOY_BUTTON1;
    }

    if (SDL_JoystickGetButton(temp, 1))
    {
        info->wButtons |= JOY_BUTTON2;
    }

    if (SDL_JoystickGetButton(temp, 2))
    {
        info->wButtons |= JOY_BUTTON3;
    }

    if (SDL_JoystickGetButton(temp, 3))
    {
        info->wButtons |= JOY_BUTTON4;
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

    auto temp = SDL_JoystickOpen(int(joy_id));

    if (!temp)
    {
      return joystickresult_t::unplugged;
    }

    const auto flags = info->dwFlags;

    *info = JOYINFOEX{};

    SDL_JoystickUpdate();

    log << "Flags " << flags << '\n';

    log << "joy_id stick: " << SDL_JoystickGetAxis(temp, 0) << " " << SDL_JoystickGetAxis(temp, 1) << '\n';

    log << "joy_id rudder: " << SDL_JoystickGetAxis(temp, 2) << " " << DWORD(SDL_JoystickGetAxis(temp, 2)) << '\n';
    log << "joy_id throttle: " << SDL_JoystickGetAxis(temp, 3) << " " << DWORD(SDL_JoystickGetAxis(temp, 3)) <<  '\n';

    info->dwXpos = int(SDL_JoystickGetAxis(temp, 0)) + std::numeric_limits<std::int16_t>::max();
    info->dwYpos = int(SDL_JoystickGetAxis(temp, 1)) + std::numeric_limits<std::int16_t>::max();
    info->dwRpos = int(SDL_JoystickGetAxis(temp, 2)) + std::numeric_limits<std::int16_t>::max();
    info->dwZpos = int(SDL_JoystickGetAxis(temp, 3)) + std::numeric_limits<std::int16_t>::max();


    info->dwSize = sizeof(JOYINFOEX);
    info->dwFlags = flags;

    if (SDL_JoystickGetButton(temp, 0))
    {
        info->dwButtons |= JOY_BUTTON1;
        info->dwButtonNumber = JOY_BUTTON1;
    }

    if (SDL_JoystickGetButton(temp, 1))
    {
        info->dwButtons |= JOY_BUTTON2;
    }

    if (SDL_JoystickGetButton(temp, 2))
    {
        info->dwButtons |= JOY_BUTTON3;
    }

    if (SDL_JoystickGetButton(temp, 3))
    {
        info->dwButtons |= JOY_BUTTON4;
    }

    auto pov = SDL_JoystickGetHat(temp, 0);

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
      info->dwPOV = JOY_POVBACKWARD + JOY_POVRIGHT / 2;
    }
    else if (pov == SDL_HAT_LEFTDOWN)
    {
      info->dwPOV = JOY_POVBACKWARD - JOY_POVRIGHT / 2;
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

