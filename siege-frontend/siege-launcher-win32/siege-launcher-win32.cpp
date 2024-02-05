// siege-launcher-win32.cpp : Defines the entry point for the application.
//

#define _CRT_SECURE_NO_WARNINGS
#include <array>
#include <optional>
#include <algorithm>
#include <vector>
#include <bit>
#include <variant>
#include <functional>
#include <thread>
#include <iostream>
#include "win32_controls.hpp"
#include "win32_builders.hpp"
#include "framework.h"
#include "Resource.h"
#include "xml_factory.hpp"
//#include "http_client.hpp"

std::array<wchar_t, 100> app_title;

using win32::overloaded;

struct siege_main_window
{
	win32::hwnd_t self;
	siege_main_window(win32::hwnd_t self, const CREATESTRUCTW&) : self(self)
	{
	}

	auto on_create(const win32::create_message&)
	{
		auto button_instance = win32::CreateWindowExW(DLGITEMTEMPLATE{
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						.x = 10,       
						.y = 10,
						.cx = 100,  
						.cy = 100       
						}, self, win32::button::class_name, L"Click me");

		win32::SetWindowSubclass(button_instance, [](win32::hwnd_t button, win32::message button_message) -> std::optional<LRESULT>
						{
							if (button_message.message == win32::command_message::id)
							{
								MessageBoxExW(GetParent(button), L"Hello world", L"Test Message", 0, 0);
								return 0;
							}

							return std::nullopt;
						});

		win32::CreateWindowExW(CREATESTRUCTW{
						.hwndParent = self,              
						.cy = 100,
						.cx = 100,                
						.y = 110, 
						.x = 10,       
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						.lpszClass = win32::edit::class_name});

		win32::CreateWindowExW(CREATESTRUCTW{
						.hwndParent = self,
						.cy = 100,
						.cx = 100,
						.y = 210,
						.x = 10,   
						.style = WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						.lpszClass = win32::combo_box::class_name
		});


		auto tab_control_instance = win32::CreateWindowExW(CREATESTRUCTW {
						.hwndParent = self,
						.cy = 300,
						.cx = 600,
						.y = 310,
						.x = 10,
						.style = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_MULTILINE | TCS_RIGHTJUSTIFY, 
						.lpszClass = win32::tab_control::class_name
					});

		std::array<wchar_t, 10> text {L"Test"};

		TCITEMW newItem {
						.mask = TCIF_TEXT,
						.pszText = text.data()
					};

		SendMessageW(tab_control_instance, TCM_INSERTITEM, 0, std::bit_cast<win32::lparam_t>(&newItem));
					
		text.fill('\0');
		std::memcpy(text.data(), L"Another", 14);
		newItem.pszText = text.data();
		SendMessageW(tab_control_instance, TCM_INSERTITEM, 1, std::bit_cast<win32::lparam_t>(&newItem));

		text.fill('\0');
		std::memcpy(text.data(), L"Tab", 6);
		newItem.pszText = text.data();
		SendMessageW(tab_control_instance, TCM_INSERTITEM, 2, std::bit_cast<win32::lparam_t>(&newItem));
		
		return 0;
	}


	std::optional<LRESULT> on_destroy(const win32::destroy_message& command) {	
				PostQuitMessage(0);
				return 0;
	}

	std::optional<LRESULT> on_command(const win32::command_message& command) {
				if (IsChild(self, command.handle))
				{
					return SendMessageW(command.handle, win32::command_message::id, command.wparam(), command.lparam());
				}
					
				if (command.identifier == IDM_ABOUT)
				{
					win32::dialog_builder builder = win32::dialog_builder{}
                        .create_dialog(DLGTEMPLATE{ .style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION, .cx = 300, .cy = 300 }, std::wstring_view{ L"Hello World" })
						.add_child(DLGITEMTEMPLATE{.style = WS_CHILD | WS_VISIBLE, .x = 10, .y = 10, .cx = 200, .cy = 100}, win32::static_text::dialog_id, std::wstring_view{L"Test Dialog"})
						.add_child(DLGITEMTEMPLATE{.style = WS_CHILD | WS_VISIBLE, .x = 10, .y = 110, .cx = 200, .cy = 100}, win32::button::class_name, std::wstring_view{L"Click Me"});

                    win32::DialogBoxIndirectParamW(self, builder.result(), [](win32::hwnd_t self, const win32::message& dialog_message) -> INT_PTR {

						if (dialog_message.message == win32::command_message::id)
						{
							win32::command_message dialog_command{dialog_message.wParam, dialog_message.lParam};

							if (dialog_command.identifier == IDOK || dialog_command.identifier == IDCANCEL)
										{
											std::array<wchar_t, 32> class_name;
											GetClassName(self, class_name.data(), class_name.size());
										//	PostThreadMessageW(GetThreadId(worker.native_handle()), WM_COUT, 0, reinterpret_cast<win32::lparam_t>(class_name.data()));
											EndDialog(self, dialog_command.identifier);
											return (INT_PTR)TRUE;
										}
						}

						return (INT_PTR)FALSE;
						});
				}
				else if (command.identifier == IDM_EXIT)
				{
					DestroyWindow(self);
					return 0;
				}

				return std::nullopt;
	}
};


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, app_title.data(), int(app_title.size()));


	INITCOMMONCONTROLSEX settings{.dwSize{sizeof(INITCOMMONCONTROLSEX)}};
	InitCommonControlsEx(&settings);

	win32::RegisterClassExW<siege_main_window>(WNDCLASSEXW {
		.style{CS_HREDRAW | CS_VREDRAW},
		.hInstance = hInstance,
		.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SIEGELAUNCHERWIN32)),
		.hCursor = LoadCursorW(hInstance, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.lpszMenuName = MAKEINTRESOURCEW(IDC_SIEGELAUNCHERWIN32),
		.lpszClassName{L"SiegeLauncherMainWindow"},
		.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
	});

	auto main_window = win32::CreateWindowExW(CREATESTRUCTW {
		.cx = CW_USEDEFAULT,
		.x = CW_USEDEFAULT,
		.style = WS_OVERLAPPEDWINDOW,
		.lpszName = app_title.data(),
		.lpszClass = L"SiegeLauncherMainWindow"
	});

	if (!main_window)
	{
		return FALSE;
	}

	ShowWindow(main_window, nCmdShow);
	UpdateWindow(main_window);


	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIEGELAUNCHERWIN32));

	MSG msg;

	// Main message loop:
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	return (int)msg.wParam;
}