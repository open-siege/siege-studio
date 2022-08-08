#include <windows.h>

namespace winmm
{
  UINT WINAPI DarkJoyGetNumDevs(void)
  {
    return 0;
  }

  MMRESULT WINAPI DarkJoyGetDevCapsA(
    UINT_PTR,
    LPJOYCAPSA,
    UINT
    )
  {
    return 0;
  }

  MMRESULT WINAPI DarkJoyGetPosEx(
    UINT,
    LPJOYINFOEX
  )
  {
    return 0;
  }

  MMRESULT WINAPI DarkJoyGetPos(
    UINT      ,
    LPJOYINFO
  )
  {
    return 0;
  }

  MMRESULT WINAPI DarkJoySetThreshold(
    UINT,
    UINT
  )
  {
    return 0;
  }
}

