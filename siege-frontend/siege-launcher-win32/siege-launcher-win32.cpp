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
#include <cassert>
#include "win32_controls.hpp"
#include "win32_builders.hpp"
#include "framework.h"
#include "Resource.h"
#include "xml_factory.hpp"
//#include "http_client.hpp"

std::array<wchar_t, 100> app_title;

using win32::overloaded;



struct siege_module
{
	std::unique_ptr<HINSTANCE__, void(*)(HINSTANCE)> module;
	HWND descriptor;

	struct module_data
	{
		module_data(std::unique_ptr<void, void(*)(void*)> buffer, std::size_t size) :
			raw(std::move(buffer)),
			storage(raw.get(), size, std::pmr::get_default_resource()),
			pool(&storage),
			available_classes(&pool),
			available_categories(&pool)
		{
		}

		std::unique_ptr<void, void(*)(void*)> raw;
		std::pmr::monotonic_buffer_resource storage;
		std::pmr::unsynchronized_pool_resource pool;
		std::pmr::unordered_map<std::pmr::wstring, std::u8string_view> available_classes;
		std::pmr::unordered_map<std::pmr::wstring, std::u8string_view> available_categories;
	};

	std::optional<module_data> data;

	siege_module(std::filesystem::path module_path) : module(LoadLibraryExW(module_path.filename().c_str(), nullptr, LOAD_LIBRARY_SEARCH_APPLICATION_DIR), [](auto module) {
		if (module) {
			FreeLibrary(module);
		}
		}), 
		descriptor(module ? win32::FindDirectChildWindow(HWND_MESSAGE, module_path.stem().c_str(), [instance = module.get()](win32::hwnd_t child) {
			return GetWindowLongPtrW(child, GWLP_HINSTANCE) == reinterpret_cast<LONG_PTR>(instance);
		}) : nullptr),
		data(std::nullopt)
		
	{
		WNDCLASSEXW temp;

		if (!descriptor)
		{
			return;
		}

		data.emplace(std::unique_ptr<void, void(*)(void*)>(HeapAlloc(GetProcessHeap(), 0, 4096 * 4), [](void* data) {
				if (data)
				{
					HeapFree(GetProcessHeap(), 0, data);
				}
				}), 4096 * 2);

		win32::ForEachPropertyExW(descriptor, [&](auto, auto name, HANDLE handle) {
			if (GetClassInfoExW(module.get(), name.data(), &temp))
			{
				data->available_classes.emplace(name, std::u8string_view(std::bit_cast<char8_t*>(handle)));
			}
			else
			{
				data->available_categories.emplace(name, std::u8string_view(std::bit_cast<char8_t*>(handle)));
			}
		});
	}

	operator bool()
	{
		return module && descriptor;
	}
};


struct siege_main_window
{
	win32::hwnd_t self;
	std::list<siege_module> loaded_modules;
	
	siege_main_window(win32::hwnd_t self, const CREATESTRUCTW& params) : self(self)
	{
		std::wstring full_app_path(256, '\0');

		GetModuleFileName(params.hInstance, full_app_path.data(), full_app_path.size());

		std::filesystem::path app_path = std::filesystem::path(full_app_path).parent_path();

		for (auto const& dir_entry : std::filesystem::directory_iterator{app_path}) 
		{
			if (dir_entry.path().extension() == ".dll")
			{
				if (auto& plugin = loaded_modules.emplace_back(dir_entry.path()); !plugin)
				{
					loaded_modules.pop_back();
				}
			}
		}
	}

	auto on_create(const win32::create_message&)
	{
		auto parent_size = win32::GetClientRect(self);

		assert(parent_size);
		auto tab_control_instance = win32::CreateWindowExW(CREATESTRUCTW {
						.hwndParent = self,
						.cy = parent_size->bottom,
						.cx = parent_size->right,
						.y = 0,
						.x = 0,
						.style = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_MULTILINE | TCS_RIGHTJUSTIFY, 
						.lpszClass = win32::tab_control::class_name
					});

		assert(tab_control_instance);

		parent_size = win32::GetClientRect(*tab_control_instance);

		int index = 0;
		for (auto& plugin : loaded_modules)
		{
			for (auto& window : plugin.data->available_classes)
			{
				TCITEMW newItem {
						.mask = TCIF_TEXT,
						.pszText = const_cast<wchar_t*>(window.first.c_str())
					};

				SendMessageW(*tab_control_instance, TCM_INSERTITEMW, index, std::bit_cast<win32::lparam_t>(&newItem));

				SendMessageW(*tab_control_instance, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&parent_size));

				auto child = win32::CreateWindowExW(win32::window_params<RECT>{
					.parent = *tab_control_instance,
					.class_name = window.first.c_str(),
					.class_module = plugin.module.get(),
					.position = *parent_size
				});

				assert(child);

				SetWindowLongPtrW(*child, GWLP_ID, index);
				index++;
			}

			SendMessageW(*tab_control_instance, TCM_SETCURSEL, 0, 0);
			NMHDR notification{.hwndFrom = *tab_control_instance, .code = TCN_SELCHANGE};
			SendMessageW(self, WM_NOTIFY, 0, std::bit_cast<LPARAM>(&notification));
		}
		
		return 0;
	}

	auto on_size(win32::size_message sized)
	{
		win32::ForEachDirectChildWindow(self, [&](auto child) {
			win32::SetWindowPos(child, POINT{}, sized.client_size);
			RECT temp {.left = 0, .top = 0, .right = sized.client_size.cx, .bottom = sized.client_size.cy };

			SendMessageW(child, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&temp));

			auto top_child = GetWindow(child, GW_CHILD);

			win32::SetWindowPos(top_child, temp);
		});

		return std::nullopt;
	}

	auto on_notify(win32::notify_message notification)
	{
		auto [sender, id, code] = notification;

		if (code == TCN_SELCHANGE)
		{
			auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);
			win32::hwnd_t child = GetWindow(sender, GW_CHILD);

			for (auto first = GetWindow(child, GW_HWNDFIRST); first != nullptr; first = GetWindow(first, GW_HWNDNEXT))
			{
				if (GetWindowLongPtrW(first, GWLP_ID) == current_index)
				{
					child = first;
					break;
				}
			}
			
			win32::ForEachChildWindow(sender, [sender](auto child) {
				if (GetParent(child) == sender)
				{
					ShowWindow(child, SW_HIDE);
				}
			});

			win32::SetWindowPos(child, HWND_TOP);

			auto temp = win32::GetClientRect(GetParent(sender));

			SendMessageW(sender, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&temp.value()));

			win32::SetWindowPos(child, *temp);


			ShowWindow(child, SW_SHOW);
		}

		return 0;
	}


	std::optional<LRESULT> on_destroy(const win32::destroy_message& command) {	
				PostQuitMessage(0);
				return 0;
	}

	std::optional<LRESULT> on_command(const win32::command_message& command) {
				if (IsChild(self, command.sender))
				{
					return SendMessageW(command.sender, win32::command_message::id, command.wparam(), command.lparam());
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

	if (IsDebuggerPresent())
	{
		std::pmr::set_default_resource(std::pmr::null_memory_resource());
	}

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, app_title.data(), int(app_title.size()));

	INITCOMMONCONTROLSEX settings{
		.dwSize{sizeof(INITCOMMONCONTROLSEX)}
	};
	InitCommonControlsEx(&settings);

	win32::RegisterClassExW<siege_main_window>(WNDCLASSEXW {
		.style{CS_HREDRAW | CS_VREDRAW},
		.hInstance = hInstance,
		.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SIEGELAUNCHERWIN32)),
		.hCursor = LoadCursorW(hInstance, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.lpszMenuName = MAKEINTRESOURCEW(IDC_SIEGELAUNCHERWIN32),
		.lpszClassName{win32::type_name<siege_main_window>().c_str()},
		.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_SMALL)),
	});

	auto main_window = win32::CreateWindowExW(CREATESTRUCTW {
		.cx = CW_USEDEFAULT,
		.x = CW_USEDEFAULT,
		.style = WS_OVERLAPPEDWINDOW,
		.lpszName = app_title.data(),
		.lpszClass = win32::type_name<siege_main_window>().c_str()
	});

	if (!main_window)
	{
		return main_window.error();
	}

	ShowWindow(*main_window, nCmdShow);
	UpdateWindow(*main_window);


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