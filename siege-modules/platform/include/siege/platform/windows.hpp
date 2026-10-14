#ifndef SIEGE_WINDOWS_HPP
#define SIEGE_WINDOWS_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef STRICT
#define STRICT
#endif

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>
#include <shellapi.h>
#include <objbase.h>
#include <windowsx.h>

#undef GetModuleFileName
#undef CreateFileMapping
#undef GetEnvironmentVariable
#undef DefaultIcon
#undef GetFirstSibling
#undef GetNextSibling
#undef GetWindowStyle

#endif// !SIEGE_WINDOWS_HPP
