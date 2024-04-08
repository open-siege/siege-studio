#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <SDKDDKVer.h>
#include <array>
#include <optional>
#include <algorithm>
#include <vector>
#include <bit>
#include <variant>
#include <functional>
#include <thread>
#include <iostream>
#include <filesystem>
#include <cassert>

#include "win32_controls.hpp"
#include "win32_builders.hpp"
#include <oleacc.h>
#include <shobjidl.h> 
#include "win32_com_client.hpp"
#include "win32_dialogs.hpp"
//#include "http_client.hpp"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WNDCLASSEXW windowClass {
		.cbSize = sizeof(WNDCLASSEXW),
		.style{CS_HREDRAW | CS_VREDRAW},
		.lpfnWndProc  = DefWindowProcW,
		.hInstance = hInstance,
		.hCursor = LoadCursorW(hInstance, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.lpszClassName{L"Main Window"},
	};

	 ::RegisterClassExW(&windowClass);

	auto main_window = win32::CreateWindowExW(CREATESTRUCTW {
		.cx = CW_USEDEFAULT,
		.x = CW_USEDEFAULT,
		.style = WS_OVERLAPPEDWINDOW,
		.lpszName = L"Siege Launcher",
		.lpszClass = L"Main Window",	
		//.dwExStyle = WS_EX_COMPOSITED,
	});

	if (!main_window)
	{
		return main_window.error();
	}

	ShowWindow(*main_window, nCmdShow);
	UpdateWindow(*main_window);

	MSG msg;

	// Main message loop:
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;
}