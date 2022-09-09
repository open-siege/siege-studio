#include <limits>
#include <array>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CINTERFACE
#include <windows.h>
#include <joystickapi.h>

#include "winmm.hpp"


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

  UINT WINAPI DarkJoyGetNumDevs(void)
  {
    return 1;
  }

  joystickresult_t WINAPI DarkJoyGetDevCapsA(
    UINT_PTR,
    LPJOYCAPSA caps,
    UINT size
    )
  {
    if (!caps || size < sizeof(JOYCAPSA))
    {
      return joystickresult_t::invalid_param;
    }

    *caps = JOYCAPSA{};
    caps->wCaps = JOYCAPS_HASZ | JOYCAPS_HASR | JOYCAPS_HASPOV;
    caps->wMid = 0x044f;
    caps->wPid = 0xb106;
    caps->wMaxAxes = 4;
    caps->wNumAxes = 0;
    caps->wMaxButtons = 12;
    caps->wNumButtons = 0;
    caps->wXmin = 0;
    caps->wXmax = std::numeric_limits<std::uint16_t>::max();
    caps->wYmin = 0;
    caps->wYmax = std::numeric_limits<std::uint16_t>::max();
    caps->wZmin = 0;
    caps->wZmax = std::numeric_limits<std::uint16_t>::max();
    caps->wRmin = 0;
    caps->wRmax = std::numeric_limits<std::uint16_t>::max();
    std::memcpy(caps->szPname, "T.Flight Stick X", sizeof(caps->szPname));


    return joystickresult_t::no_error;
  }

  joystickresult_t WINAPI DarkJoyGetPos(
    UINT,
    LPJOYINFO info
  )
  {
    if (!info)
    {
      return joystickresult_t::invalid_param;
    }

    *info = JOYINFO{};
    return joystickresult_t::no_error;
  }

  joystickresult_t WINAPI DarkJoyGetPosEx(
    UINT,
    LPJOYINFOEX info
  )
  {
    if (!info || info->dwSize < sizeof(JOYINFOEX))
    {
      return joystickresult_t::invalid_param;
    }

    //const auto flags = info->dwFlags;
    *info = JOYINFOEX{};
    return joystickresult_t::no_error;
  }

  joystickresult_t WINAPI DarkJoySetThreshold(
    UINT,
    UINT
  )
  {
    return joystickresult_t::no_error;
  }

  static auto* TrueJoyGetNumDevs = joyGetNumDevs;
  static auto* TrueJoyGetDevCapsA = joyGetDevCapsA;
  static auto* TrueJoyGetPos = joyGetPos;
  static auto* TrueJoyGetPosEx = joyGetPosEx;
  static auto* TrueJoySetThreshold = joySetThreshold;

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

