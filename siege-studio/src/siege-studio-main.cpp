#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>
#include <array>
#include <optional>
#include <algorithm>
#include <vector>
#include <set>
#include <bit>
#include <variant>
#include <functional>
#include <thread>
#include <iostream>
#include <filesystem>
#include <cassert>

#include <siege/platform/win/desktop/win32_dialogs.hpp>
#include <siege/platform/win/core/com_client.hpp>
#include <commctrl.h>
#include <oleacc.h>
#include <shobjidl.h> 
#include <shlwapi.h> 

#include "siege_main_window.hpp"

//#include "http_client.hpp"

constexpr static std::wstring_view app_title = L"Siege Studio";

using win32::overloaded;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (IsDebuggerPresent())
	{
		std::pmr::set_default_resource(std::pmr::null_memory_resource());
	}

	win32::com::init_com();

	INITCOMMONCONTROLSEX settings{
		.dwSize{sizeof(INITCOMMONCONTROLSEX)}
	};
	InitCommonControlsEx(&settings);


	auto mfcHandle = LoadLibraryExW(L"siege-win-mfc.dll", nullptr, 0);

	win32::RegisterClassExW<siege_main_window>(WNDCLASSEXW {
		.style{CS_HREDRAW | CS_VREDRAW},
		.hInstance = hInstance,
		.hCursor = LoadCursorW(hInstance, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.lpszClassName{win32::type_name<siege_main_window>().c_str()},
	});

	auto main_menu = ::CreateMenu();

	std::size_t id = 1u;

	auto file_menu = ::CreatePopupMenu();	
	AppendMenuW(main_menu, MF_POPUP, reinterpret_cast<INT_PTR>(file_menu), L"File");
	AppendMenuW(file_menu, MF_STRING, RegisterWindowMessageW(L"COMMAND_OPEN"), L"Open");
	AppendMenuW(file_menu, MF_STRING, RegisterWindowMessageW(L"COMMAND_SAVE"), L"Save");
	AppendMenuW(file_menu, MF_STRING, RegisterWindowMessageW(L"COMMAND_SAVE_AS"), L"Save As");
	AppendMenuW(file_menu, MF_SEPARATOR , id++, nullptr);
	AppendMenuW(file_menu, MF_STRING, RegisterWindowMessageW(L"COMMAND_EXIT"), L"Exit");

	AppendMenuW(main_menu, MF_STRING, id++, L"Edit");
    AppendMenuW(main_menu, MF_STRING, id++, L"View");
    AppendMenuW(main_menu, MF_STRING, id++, L"Help");

    auto main_window = win32::window_module_ref::current_module().CreateWindowExW(CREATESTRUCTW {
		.hMenu = main_menu,
		.cx = CW_USEDEFAULT,
		.x = CW_USEDEFAULT,
		.style = WS_OVERLAPPEDWINDOW,
		.lpszName = app_title.data(),
		.lpszClass = win32::type_name<siege_main_window>().c_str(),
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

	CoUninitialize();
	FreeLibrary(mfcHandle);

	return (int)msg.wParam;
}