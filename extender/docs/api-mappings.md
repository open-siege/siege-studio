# Starsiege
## wsock32 to ZeroTier
| wsock32               | alternative                                                                                              |
|-----------------------|----------------------------------------------------------------------------------------------------------|
| WSACancelBlockingCall | deprecated                                                                                               |
| WSACleanup            | zts_close + zts_node_stop                                                                                |
| WSAGetLastError       | zts_get_last_socket_error                                                                                |
| WSASetBlockingHook    | deprecated                                                                                               |
| WSAStartup            | zts_init_from_storage + zts_node_start + zts_node_is_online + zts_net_join  + zts_net_transport_is_ready |
| WSAUnhookBlockingHook | deprecated                                                                                               |
| accept                | zts_bsd_accept                                                                                           |
| closesocket           | zts_bsd_close                                                                                            |
| connect               | zts_bsd_connect                                                                                          |
| gethostbyname         | zts_bsd_gethostbyname                                                                                    |
| getsockname           | zts_getsockname                                                                                          |
| getsockopt            | zts_bsd_getsockopt                                                                                       |
| htonl                 | boost_endian                                                                                             |
| htons                 | boost_endian                                                                                             |
| inet_addr             | ?                                                                                                        |
| ioctlsocket           | zts_bsd_ioctl                                                                                            |
| listen                | zts_bsd_listen                                                                                           |
| ntohs                 | zts_util_ntop + boost_endian                                                                             |
| recv                  | zts_bsd_recv                                                                                             |
| recvfrom              | zts_bsd_recvfrom                                                                                         |
| send                  | zts_bsd_send                                                                                             |
| sendto                | zts_bsd_sendto                                                                                           |
| setsockopt            | zts_bsd_setsockopt                                                                                       |
| socket                | zts_bsd_socket                                                                                           |
| bind                  | zts_bsd_bind                                                                                             |

## UWP supported mapping
| advapi32              | alternative |
|-----------------------|-------------|
| RegCloseKey           | deprecated  |
| RegCreateKeyExA       | deprecated  |
| RegDeleteKeyA         | deprecated  |
| RegEnumKeyExA         | deprecated  |
| RegEnumValueA         | deprecated  |
| RegOpenKeyExA         | deprecated  |
| RegQueryValueExA      | deprecated  |
| RegSetValueExA        | deprecated  |

| kernel32                      | alternative              |
|-------------------------------|--------------------------|
| AllocConsole                  | same                     |
| GetNumberOfConsoleInputEvents | same                     |
| ReadConsoleInputA             | same                     |
| CloseHandle                   | same                     |
| CopyFileA                     | same                     |
| CreateEventA                  | same                     |
| CreateFileMappingA            | CreateFileMappingFromApp |
| CreateMutexA                  | same                     |
| CreateSemaphoreA              | same                     |
| CreateThread                  | same                     |
| SetThreadPriority             | same                     |
| Sleep                         | same                     |
| TlsAlloc                      | same                     |
| TlsFree                       | same                     |
| TlsGetValue                   | same                     |
| TlsSetValue                   | same                     |
| DebugBreak                    | same                     |
| DeleteCriticalSection         | same                     |
| DeleteFileA                   | same                     |
| EnterCriticalSection          | same                     |
| ExitProcess                   | same                     |
| FileTimeToDosDateTime         | deprecated               |
| FileTimeToLocalFileTime       | same                     |
| GetTimeZoneInformation        | same                     |
| GetFileAttributesA            | same                     |
| GetFileSize                   | GetFileSizeEx            |
| CreateDirectoryA              | same                     |
| CreateFileA                   | CreateFile2              |
| WriteFile                     | same                     |
| ReadFile                      | same                     |
| SetEndOfFile                  | same                     |
| SetFilePointer                | same                     |
| SetFileAttributesA            | same                     |
| FindFirstFileA                | same                     |
| FindNextFileA                 | same                     |
| FindClose                     | same                     |
| GetDiskFreeSpaceA             | same                     |
| GetFileType                   | same                     |
| FlushFileBuffers              | same                     |
| FormatMessageA                | same                     |
| FreeLibrary                   | same                     |
| LoadLibraryA                  | LoadPackagedLibrary      |
| GetProcAddress                | same                     |
| GetModuleFileNameA            | same                     |
| GetACP                        | same                     |
| GetCPInfo                     | same                     |
| GetCommandLineA               | same                     |
| GetCurrentDirectoryA          | GetCurrentDirectoryW	    |
| SetCurrentDirectoryA          | SetCurrentDirectoryW     |
| GetCurrentProcessId           | same                     |
| GetCurrentThreadId            | same                     |
| GetDateFormatA                | GetDateFormatEx          |
| GetLogicalDrives              | same                     |
| GetDriveTypeA                 | same                     |
| GetFullPathNameA              | same                     |
| GetEnvironmentStrings         | same                     |
| GetStdHandle                  | same                     |
| GetLastError                  | same                     |
| RaiseException                | same                     |
| GetLocalTime                  | same                     |
| GetModuleHandleA              | deprecated               |
| GetStartupInfoA               | deprecated               |
| GetStringTypeW                | same                     |
| MultiByteToWideChar           | same                     |
| GetTickCount                  | same                     |
| GetVersion                    | GetVersionExA            |
| GetVersionExA                 | same                     |
| GetVolumeInformationA         | same                     |
| GlobalAlloc                   | same                     |
| LocalAlloc                    | same                     |
| LocalFree                     | same                     |
| GlobalLock                    | same                     |
| GlobalMemoryStatus            | GlobalMemoryStatusEx     |
| GlobalUnlock                  | same                     |
| InitializeCriticalSection     | same                     |
| LeaveCriticalSection          | same                     |
| ResetEvent                    | same                     |
| SetEvent                      | same                     |
| InterlockedExchange           | same                     |
| IsBadReadPtr                  | deprecated               |
| MapViewOfFile                 | MapViewOfFileFromApp     |
| QueryPerformanceCounter       | same                     |
| QueryPerformanceFrequency     | same                     |
| RtlUnwind                     | same                     |
| SearchPathA                   | deprecated               |
| SetConsoleCtrlHandler         | same                     |
| SetConsoleTitleA              | same                     |
| SetErrorMode                  | same                     |
| SetHandleCount                | deprecated               |
| SetUnhandledExceptionFilter   | same                     |
| UnhandledExceptionFilter      | deprecated               |
| UnmapViewOfFile               | same                     |
| VirtualAlloc                  | VirtualAllocFromApp      |
| VirtualFree                   | same                     |
| VirtualProtect                | VirtualProtectFromApp    |
| VirtualQuery                  | same                     |
| WaitForSingleObject           | same                     |
| WideCharToMultiByte           | same                     |
| lstrlenA                      | deprecated               |

| wsock32               | alternative |
|-----------------------|-------------|
| WSACancelBlockingCall | deprecated  |
| WSACleanup            | same        |
| WSAGetLastError       | same        |
| WSASetBlockingHook    | deprecated  |
| WSAStartup            | same        |
| WSAUnhookBlockingHook | deprecated  |
| accept                | same        |
| closesocket           | same        |
| connect               | same        |
| gethostbyname         | same        |
| getsockname           | same        |
| getsockopt            | same        |
| htonl                 | same        |
| htons                 | same        |
| inet_addr             | same        |
| ioctlsocket           | same        |
| listen                | same        |
| ntohs                 | same        |
| recv                  | same        |
| recvfrom              | same        |
| send                  | same        |
| sendto                | same        |
| setsockopt            | same        |
| socket                | same        |
| bind                  | same        |

| comctl32                 | alternative |
|--------------------------|-------------|
| CreateStatusWindowA      | deprecated  |
| CreateToolbarEx          | deprecated  |
| ImageList_AddMasked      | deprecated  |
| ImageList_BeginDrag      | deprecated  |
| ImageList_Create         | deprecated  |
| ImageList_DragEnter      | deprecated  |
| ImageList_DragLeave      | deprecated  |
| ImageList_DragMove       | deprecated  |
| ImageList_DragShowNolock | deprecated  |
| ImageList_EndDrag        | deprecated  |
| InitCommonControls       | deprecated  |

| comdlg32         | alternative |
|------------------|-------------|
| GetOpenFileNameA | deprecated  |
| GetSaveFileNameA | deprecated  |

| gdi32                   | alternative |
|-------------------------|-------------|
| AnimatePalette          | deprecated  |
| BitBlt                  | deprecated  |
| ChoosePixelFormat       | deprecated  |
| CreateCompatibleDC      | deprecated  |
| CreateDIBSection        | deprecated  |
| CreateDIBitmap          | deprecated  |
| CreatePalette           | deprecated  |
| DeleteDC                | deprecated  |
| DeleteObject            | deprecated  |
| DescribePixelFormat     | deprecated  |
| GetDIBColorTable        | deprecated  |
| GetDeviceCaps           | deprecated  |
| GetPixelFormat          | deprecated  |
| GetStockObject          | deprecated  |
| GetSystemPaletteEntries | deprecated  |
| GetTextExtentPoint32A   | deprecated  |
| LineTo                  | deprecated  |
| MoveToEx                | deprecated  |
| Polygon                 | deprecated  |
| RealizePalette          | deprecated  |
| SelectObject            | deprecated  |
| SelectPalette           | deprecated  |
| SetDIBColorTable        | deprecated  |
| SetPixelFormat          | deprecated  |
| SetROP2                 | deprecated  |
| StretchBlt              | deprecated  |
| SwapBuffers             | deprecated  |

| shell32          | alternative |
|------------------|-------------|
| ShellExecuteA    | deprecated  |

| user32                     | alternative |
|----------------------------|-------------|
| AdjustWindowRect           | deprecated  |
| AdjustWindowRectEx         | deprecated  |
| AppendMenuA                | deprecated  |
| BeginDeferWindowPos        | deprecated  |
| BeginPaint                 | deprecated  |
| BringWindowToTop           | deprecated  |
| CallNextHookEx             | deprecated  |
| CallWindowProcA            | deprecated  |
| ChangeDisplaySettingsA     | deprecated  |
| CheckDlgButton             | deprecated  |
| CheckMenuItem              | deprecated  |
| ChildWindowFromPointEx     | deprecated  |
| ClientToScreen             | deprecated  |
| CloseClipboard             | deprecated  |
| CreateDialogParamA         | deprecated  |
| CreateMDIWindowA           | deprecated  |
| CreatePopupMenu            | deprecated  |
| CreateWindowExA            | deprecated  |
| DefMDIChildProcA           | deprecated  |
| DefWindowProcA             | deprecated  |
| DeferWindowPos             | deprecated  |
| DeleteMenu                 | deprecated  |
| DestroyMenu                | deprecated  |
| DestroyWindow              | deprecated  |
| DialogBoxIndirectParamA    | deprecated  |
| DialogBoxParamA            | deprecated  |
| DispatchMessageA           | deprecated  |
| DrawMenuBar                | deprecated  |
| EmptyClipboard             | deprecated  |
| EnableMenuItem             | deprecated  |
| EnableScrollBar            | deprecated  |
| EnableWindow               | deprecated  |
| EndDeferWindowPos          | deprecated  |
| EndDialog                  | deprecated  |
| EndPaint                   | deprecated  |
| EnumDisplaySettingsA       | deprecated  |
| EnumThreadWindows          | deprecated  |
| GetActiveWindow            | deprecated  |
| GetAsyncKeyState           | deprecated  |
| GetClientRect              | deprecated  |
| GetClipboardData           | deprecated  |
| GetCursorPos               | deprecated  |
| GetDC                      | deprecated  |
| GetDesktopWindow           | deprecated  |
| GetDialogBaseUnits         | deprecated  |
| GetDlgItem                 | deprecated  |
| GetDlgItemTextA            | deprecated  |
| GetDoubleClickTime         | deprecated  |
| GetFocus                   | deprecated  |
| GetForegroundWindow        | deprecated  |
| GetKeyState                | deprecated  |
| GetKeyboardState           | deprecated  |
| GetMenu                    | deprecated  |
| GetMenuItemCount           | deprecated  |
| GetMenuItemID              | deprecated  |
| GetParent                  | deprecated  |
| GetScrollPos               | deprecated  |
| GetSubMenu                 | deprecated  |
| GetSystemMetrics           | deprecated  |
| GetTopWindow               | deprecated  |
| GetWindow                  | deprecated  |
| GetWindowLongA             | deprecated  |
| GetWindowPlacement         | deprecated  |
| GetWindowRect              | deprecated  |
| GetWindowTextA             | deprecated  |
| GetWindowTextLengthA       | deprecated  |
| GetWindowThreadProcessId   | deprecated  |
| InsertMenuA                | deprecated  |
| InvalidateRect             | deprecated  |
| IsClipboardFormatAvailable | deprecated  |
| IsDialogMessageA           | deprecated  |
| IsDlgButtonChecked         | deprecated  |
| IsIconic                   | deprecated  |
| IsMenu                     | deprecated  |
| IsWindow                   | deprecated  |
| KillTimer                  | deprecated  |
| LoadCursorA                | deprecated  |
| LoadIconA                  | deprecated  |
| LoadMenuA                  | deprecated  |
| LoadStringA                | deprecated  |
| MapWindowPoints            | deprecated  |
| MessageBeep                | deprecated  |
| MessageBoxA                | deprecated  |
| MoveWindow                 | deprecated  |
| MsgWaitForMultipleObjects  | deprecated  |
| OpenClipboard              | deprecated  |
| PeekMessageA               | deprecated  |
| PostMessageA               | deprecated  |
| PostQuitMessage            | deprecated  |
| RedrawWindow               | deprecated  |
| RegisterClassA             | deprecated  |
| RegisterClassExA           | deprecated  |
| ReleaseCapture             | deprecated  |
| ReleaseDC                  | deprecated  |
| ScreenToClient             | deprecated  |
| SendDlgItemMessageA        | deprecated  |
| SendMessageA               | deprecated  |
| SetCapture                 | deprecated  |
| SetClipboardData           | deprecated  |
| SetCursor                  | deprecated  |
| SetCursorPos               | deprecated  |
| SetDlgItemTextA            | deprecated  |
| SetFocus                   | deprecated  |
| SetForegroundWindow        | deprecated  |
| SetMenu                    | deprecated  |
| SetRect                    | deprecated  |
| SetScrollPos               | deprecated  |
| SetTimer                   | deprecated  |
| SetWindowLongA             | deprecated  |
| SetWindowPos               | deprecated  |
| SetWindowTextA             | deprecated  |
| SetWindowsHookExA          | deprecated  |
| ShowCursor                 | deprecated  |
| ToAscii                    | deprecated  |
| TrackPopupMenu             | deprecated  |
| TrackPopupMenuEx           | deprecated  |
| TranslateAcceleratorA      | deprecated  |
| TranslateMessage           | deprecated  |
| UnhookWindowsHookEx        | deprecated  |
| UpdateWindow               | deprecated  |
| ValidateRect               | deprecated  |
| WindowFromPoint            | deprecated  |
| wsprintfA                  | deprecated  |

| winmm                   | alternative |
|-------------------------|-------------|
| auxGetDevCapsA          | deprecated  |
| auxGetNumDevs           | deprecated  |
| auxGetVolume            | deprecated  |
| auxSetVolume            | deprecated  |
| mciSendCommandA         | deprecated  |
| mixerClose              | deprecated  |
| mixerGetControlDetailsA | deprecated  |
| mixerGetLineControlsA   | deprecated  |
| mixerGetLineInfoA       | deprecated  |
| mixerGetNumDevs         | deprecated  |
| mixerOpen               | deprecated  |
| mixerSetControlDetails  | deprecated  |

| dsound            | alternative |
|-------------------|-------------|
| DirectSoundCreate | deprecated  |

| ole32            | alternative             |
|------------------|-------------------------|
| CoCreateInstance | CoCreateInstanceFromApp |
| CoInitialize     | CoInitializeEx          |
| CoUninitialize   | same                    |

# Earthsiege 2
## Joystick
### winmm
* joyGetNumDevs
* joyGetDevCapsA
* joyGetPosEx
* joyGetPos
* joyGetDevCapsA
* joySetThreshold

# Other
## ws_32
* WSARecvFrom
* WSASendTo
* WSAGetOverlappedResult
* WSAWaitForMultipleEvents
* WSAResetEvent
## iphlpapi
* GetAdaptersInfo
