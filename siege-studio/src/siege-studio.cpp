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

#include <siege/platform/win/desktop/shell.hpp>
#include <siege/platform/win/desktop/window_impl.hpp>
#include <siege/platform/win/core/com/client.hpp>
#include <commctrl.h>
#include <oleacc.h>

#include "views/siege_main_window.hpp"

constexpr static std::wstring_view app_title = L"Siege Studio";

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
	win32::com::init_com();

	INITCOMMONCONTROLSEX settings{
		.dwSize{sizeof(INITCOMMONCONTROLSEX)}
	};
	InitCommonControlsEx(&settings);

	win32::window_module_ref this_module(hInstance);

	win32::window_meta_class<siege::views::siege_main_window> info{};
	info.style = CS_HREDRAW | CS_VREDRAW;
	info.hCursor = LoadCursorW(hInstance, IDC_ARROW);
	info.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	this_module.RegisterClassExW(info);

	auto main_menu = ::CreateMenu();

	std::size_t id = 1u;

	auto file_menu = ::CreatePopupMenu();	
	AppendMenuW(main_menu, MF_POPUP, reinterpret_cast<INT_PTR>(file_menu), L"File");
	AppendMenuW(file_menu, MF_STRING, RegisterWindowMessageW(L"COMMAND_OPEN"), L"Open...");
	AppendMenuW(file_menu, MF_STRING, RegisterWindowMessageW(L"COMMAND_OPEN_NEW_TAB"), L"Open in New Tab...");
	AppendMenuW(file_menu, MF_STRING, RegisterWindowMessageW(L"COMMAND_OPEN_WORKSPACE"), L"Open Folder as Workspace");
	AppendMenuW(file_menu, MF_SEPARATOR , id++, nullptr);
	AppendMenuW(file_menu, MF_STRING, RegisterWindowMessageW(L"COMMAND_EXIT"), L"Quit");

	AppendMenuW(main_menu, MF_STRING, id++, L"Edit");
    AppendMenuW(main_menu, MF_STRING, id++, L"View");
    AppendMenuW(main_menu, MF_STRING, id++, L"Help");

	MENUINFO mi = { 0 };
    mi.cbSize = sizeof(mi);
    mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
    mi.hbrBack = ::CreateSolidBrush(0x00383838);

    SetMenuInfo(main_menu, &mi); 

    auto main_window = this_module.CreateWindowExW(CREATESTRUCTW {
		.hMenu = main_menu,
		.cx = CW_USEDEFAULT,
		.x = CW_USEDEFAULT,
		.style = WS_OVERLAPPEDWINDOW,
		.lpszName = app_title.data(),
		.lpszClass = win32::type_name<siege::views::siege_main_window>().c_str()
	});

	if (!main_window)
	{
		return main_window.error();
	}

	ShowWindow(*main_window, nCmdShow);
	UpdateWindow(*main_window);

	MSG msg;

	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;
}