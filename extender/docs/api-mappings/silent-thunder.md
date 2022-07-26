# A-10 Tank Killer 2: Silent Thunder
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
* StretchBlt
* SetDIBColorTable
* SelectPalette
* SelectObject
* RealizePalette
* GetSystemPaletteEntries
* GetStockObject
* GetDIBColorTable
* DeleteObject
* DeleteDC
* CreatePalette
* CreateDIBSection
* CreateCompatibleDC
* BitBlt
* AnimatePalette

### user32
* GetCaretBlinkTime
* GetAsyncKeyState
* GetActiveWindow
* DispatchMessageA
* DestroyWindow
* DefWindowProcA
* CreateWindowExA
* AdjustWindowRectEx
* wsprintfA
* keybd_event
* UpdateWindow
* TranslateMessage
* ShowWindow
* ShowCursor
* SetWindowTextA
* SetWindowPos
* SetRect
* SetMenu
* SendMessageA
* ScreenToClient
* ReleaseDC
* RegisterClassExA
* PostQuitMessage
* PostMessageA
* PeekMessageA
* MessageBoxA
* LoadStringA
* LoadMenuA
* LoadIconA
* LoadCursorA
* GetWindowRect
* GetSystemMetrics
* GetParent
* GetKeyState
* GetFocus
* GetDC
* GetCursorPos


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
