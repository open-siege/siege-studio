# Earthsiege 2
## Joystick
### winmm
* joyGetNumDevs
* joyGetDevCapsA
* joyGetPosEx
* joyGetPos
* joyGetDevCapsA
* joySetThreshold

## Graphics
### gdi32
* GetDeviceCaps
* DeleteObject
* CreatePalette
* CreateDIBSection
* CreateCompatibleDC
* AnimatePalette
* StretchBlt
* SetDIBColorTable
* SelectPalette
* SelectObject
* RealizePalette
* GetSystemPaletteUse
* GetSystemPaletteEntries

### user32
* PeekMessageA
* MoveWindow
* MessageBoxA
* LoadStringA
* LoadIconA
* LoadCursorA
* KillTimer
* InvalidateRect
* GetWindowRect
* GetSystemMetrics
* GetParent
* GetKeyState
* GetDC
* GetCursorPos
* GetClientRect
* EndPaint
* DispatchMessageA
* DestroyWindow
* DefWindowProcA
* CreateWindowExA
* ClipCursor
* ClientToScreen
* BeginPaint
* AdjustWindowRectEx
* WinHelpA
* UpdateWindow
* TranslateMessage
* ShowWindow
* ShowCursor
* SetWindowTextA
* SetWindowPos
* SetTimer
* SetRect
* SetCursorPos
* SetCursor
* ReleaseDC
* RegisterClassA
* PostQuitMessage
* PostMessageA


## UWP Support Mapping
| kernel32                 | alternative          |
|--------------------------|----------------------|
| GlobalLock               | same                 |
| GetModuleHandleA         | deprecated           |
| GlobalHandle             | deprecated           |
| OpenFile                 | CreateFile2          |
| CreateFileA              | CreateFile2          |
| FreeLibrary              | same                 |
| LoadLibraryA             | LoadPackagedLibrary  |
| GetProcAddress           | same                 |
| CloseHandle              | same                 |
| GlobalAlloc              | same                 |
| GlobalFree               | same                 |
| GlobalUnlock             | same                 |
| _lopen                   | deprecated           |
| _llseek                  | deprecated           |
| _lclose                  | deprecated           |
| _hread                   | deprecated           |
| Sleep                    | same                 |
| SetLastError             | same                 |
| GetVersion               | GetVersionExA        |
| GetTickCount             | same                 |
| DeviceIoControl          | same                 |
| GetPrivateProfileStringA | deprecated           |
| GlobalMemoryStatus       | GlobalMemoryStatusEx |
