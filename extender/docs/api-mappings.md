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
| WSACancelBlockingCall | deprecated  |
| WSACancelBlockingCall | deprecated  |
| WSACancelBlockingCall | deprecated  |
| WSACancelBlockingCall | deprecated  |
| WSACancelBlockingCall | deprecated  |
| WSACancelBlockingCall | deprecated  |
| WSACancelBlockingCall | deprecated  |

| kernel32                | alternative              |
|-------------------------|--------------------------|
| AllocConsole            | AllocConsole             |
| CloseHandle             | CloseHandle              |
| CopyFileA               | CopyFileA                |
| CreateDirectoryA        | CreateDirectoryA         |
| CreateEventA            | CreateEventA             |
| CreateFileA             | CreateFile2              |
| CreateFileMappingA      | CreateFileMappingFromApp |
| CreateMutexA            | CreateMutexA             |
| CreateSemaphoreA        | CreateSemaphoreA         |
| CreateThread            | CreateThread             |
| DebugBreak              | DebugBreak               |
| DeleteCriticalSection   | DeleteCriticalSection    |
| DeleteFileA             | DeleteFileA              |
| EnterCriticalSection    | EnterCriticalSection     |
| ExitProcess             | ExitProcess              |
| FileTimeToDosDateTime   | deprecated               |
| FileTimeToLocalFileTime | FileTimeToLocalFileTime  |
| FindClose               | FindClose                |
| FindFirstFileA          | FindFirstFileA           |
| FindNextFileA           | FindNextFileA            |
| FlushFileBuffers        | FlushFileBuffers         |

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
