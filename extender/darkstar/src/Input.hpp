#ifndef OPEN_SIEGE_INPUT_HPP
#define OPEN_SIEGE_INPUT_HPP

#define NOMINMAX
#include <windows.h>

enum class joystickresult_t : UINT
{
  no_error = JOYERR_NOERROR,
  bad_joystick_id = JOYERR_PARMS,
  unplugged = JOYERR_UNPLUGGED,
  bad_device_id = MMSYSERR_BADDEVICEID,
  no_driver = MMSYSERR_NODRIVER,
  invalid_param = MMSYSERR_INVALPARAM
};

UINT WINAPI DarkJoyGetNumDevs();
joystickresult_t WINAPI DarkJoyGetDevCapsA(UINT_PTR joy_id, LPJOYCAPSA capabilities, UINT capabilities_size);
joystickresult_t WINAPI DarkJoyGetPos(UINT joy_id, LPJOYINFO info);
joystickresult_t WINAPI DarkJoyGetPosEx(UINT joy_id, LPJOYINFOEX info);
joystickresult_t WINAPI DarkJoySetThreshold(UINT joy_id, UINT threshold);

#endif// OPEN_SIEGE_INPUT_HPP
