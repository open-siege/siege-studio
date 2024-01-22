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
#include "windows.hpp"
//#include "http_client.hpp"

std::array<wchar_t, 100> app_title;

void worker_thread_main()
{
	MSG msg;

	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		auto cracked_message = make_window_message(msg);

		std::visit(overloaded {
			[&](::message& message) -> std::optional<LRESULT> {
				if (message.message == WM_COUT)
				{
					if (message.wParam == 0)
					{
						std::wcout << reinterpret_cast<wchar_t*>(message.lParam);
					}
				}
				return std::nullopt;
			},
			[&](auto&)  -> std::optional<LRESULT> {
				return std::nullopt;
			}
		}, cracked_message);
		
		std::this_thread::yield();
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	std::thread worker(worker_thread_main);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, app_title.data(), int(app_title.size()));
	static auto descriptor = main_window::get_descriptor(hInstance);
	RegisterClassExW(&descriptor);

	std::vector<dialog> child_dialogs;
	std::vector<client_control> controls;

	main_window window{ .HandleMessage {
		[&](main_window& window, auto message) -> std::optional<LRESULT>
	{
		 return std::visit(overloaded {
			 [&](create_message& command) -> std::optional<LRESULT> {
					
#if _DEBUG
						AllocConsole();

						freopen("CONOUT$", "w", stdout);
						freopen("CONOUT$", "w", stderr);
#endif

					INITCOMMONCONTROLSEX settings{.dwSize{sizeof(INITCOMMONCONTROLSEX)}};
					InitCommonControlsEx(&settings);

					controls.reserve(10);

					PostThreadMessageW(GetThreadId(worker.native_handle()), WM_COUT, 0, reinterpret_cast<LPARAM>(L"Main window created"));

					auto& button_instance = controls.emplace_back(button{ CreateWindowW(
						L"BUTTON",  // Predefined class; Unicode assumed 
						L"Click me",      // Button text 
						WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
						10,         // x position 
						10,         // y position 
						100,        // Button width
						100,        // Button height
						window.handle,     // Parent window
						NULL,       // No menu.
						(HINSTANCE)GetWindowLongPtr(window.handle, GWLP_HINSTANCE),
						NULL), [&](auto& self, UINT_PTR uIdSubclass, auto button_message) -> std::optional<LRESULT>
						{
							return std::visit(overloaded{
								[&](command_message& command) -> std::optional<LRESULT> {
									MessageBoxExW(self.handle, L"Hello world", L"Test Message", 0, 0);
									return TRUE;
								},
								[](auto&) -> std::optional<LRESULT> { return std::nullopt; }
							}, button_message);

							return std::nullopt;
						}
						});

					auto& edit_instance = controls.emplace_back(edit{ CreateWindowW(
						L"EDIT",  // Predefined class; Unicode assumed 
						L"",      // Button text 
						WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
						10,         // x position 
						110,         // y position 
						100,        // Button width
						100,        // Button height
						window.handle,     // Parent window
						NULL,       // No menu.
						(HINSTANCE)GetWindowLongPtr(window.handle, GWLP_HINSTANCE),
						NULL), [&](auto& self, UINT_PTR uIdSubclass, auto message) -> std::optional<LRESULT>
						{
							return std::nullopt;
						}
						});

					auto& combo_box_instance = controls.emplace_back(combo_box{ CreateWindowW(
						L"COMBOBOX",  // Predefined class; Unicode assumed 
						L"",      // Button text 
						WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
						10,         // x position 
						210,         // y position 
						100,        // Button width
						100,        // Button height
						window.handle,     // Parent window
						NULL,       // No menu.
						(HINSTANCE)GetWindowLongPtr(window.handle, GWLP_HINSTANCE),
						NULL), [&](auto& self, UINT_PTR uIdSubclass, auto message) -> std::optional<LRESULT>
						{
							return std::nullopt;
						}
						});

				return 0;
			 },
			 [&](destroy_message& command) -> std::optional<LRESULT> {
				controls.erase(std::remove_if(controls.begin(), controls.end(), [](auto& button) {
						 return std::visit([](auto& real_control) { return !real_control.HandleMessage; }, button);
						 }), controls.end());
				PostQuitMessage(0);
				return 0;
			 },
			 [&](command_message& command) -> std::optional<LRESULT> {
				auto child_control = std::find_if(controls.begin(), controls.end(), [&](auto& control) {
					return command.handle == std::visit([](auto& real_control) { return real_control.handle; }, control);
					});

				if (child_control != controls.end())
				{
					return SendMessageW(command.handle, command_message::id, command.wparam(), command.lparam());
				}

				if (command.identifier == IDM_ABOUT)
				{
					auto& modal = child_dialogs.emplace_back([&](dialog& self, auto dialog_message) -> INT_PTR {
						return std::visit(overloaded{
									[&](command_message& dialog_command) -> INT_PTR {
										if (dialog_command.identifier == IDOK || dialog_command.identifier == IDCANCEL)
										{
											EndDialog(self.handle, dialog_command.identifier);
											return (INT_PTR)TRUE;
										}

										return (INT_PTR)FALSE;
									},
								[](auto&) -> INT_PTR {
									return (INT_PTR)FALSE;
									}
						}, dialog_message);
						});
					DialogBoxParamW(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), window.handle, dialog::HandleAboutDialogMessage, reinterpret_cast<LPARAM>(&modal));

					child_dialogs.erase(std::remove_if(child_dialogs.begin(), child_dialogs.end(), [](auto& dialog) {
						return !dialog.HandleMessage;
						}), child_dialogs.end());
				}
				else if (command.identifier == IDM_EXIT)
				{
					DestroyWindow(window.handle);
					return 0;
				}

				return std::nullopt;
			},
			 [&](auto& raw_message) -> std::optional<LRESULT> {
				 return std::nullopt;
			}
		}, message);
	}
	} };

	HWND hWnd = CreateWindowW(descriptor.lpszClassName, app_title.data(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, &window);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


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