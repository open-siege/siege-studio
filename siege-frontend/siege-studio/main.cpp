#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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
#include "win32_controls.hpp"
#include "win32_builders.hpp"
#include <commctrl.h>
#include <oleacc.h>
#include <shobjidl.h> 
#include "win32_com_client.hpp"
#include "win32_dialogs.hpp"
#include "siege-plugin.hpp"
//#include "http_client.hpp"

constexpr static std::wstring_view app_title = L"Siege Studio";

using win32::overloaded;

struct siege_main_window
{
	win32::hwnd_t self;
	std::list<siege::siege_plugin> loaded_modules;
	
	std::set<std::wstring> extensions;
	std::set<std::wstring> categories;

	siege_main_window(win32::hwnd_t self, const CREATESTRUCTW& params) : self(self)
	{
		std::wstring full_app_path(256, '\0');

		GetModuleFileName(params.hInstance, full_app_path.data(), full_app_path.size());

		std::filesystem::path app_path = std::filesystem::path(full_app_path).parent_path();

		for (auto const& dir_entry : std::filesystem::directory_iterator{app_path}) 
		{
			if (dir_entry.path().extension() == ".dll")
			{
				try
				{
					loaded_modules.emplace_back(dir_entry.path());			
				}
				catch(...)
				{
				}
			}
		}

		for (auto& module : loaded_modules)
		{
			auto module_exts = module.GetSupportedExtensions();
			std::copy(module_exts.begin(), module_exts.end(), std::inserter(extensions, extensions.begin()));

			auto category_exts = module.GetSupportedFormatCategories(LOCALE_USER_DEFAULT);
			std::copy(category_exts.begin(), category_exts.end(), std::inserter(categories, categories.begin()));
		}
	}

	auto on_create(const win32::create_message&)
	{
		auto parent_size = win32::GetClientRect(self);

		assert(parent_size);

		auto mfcModule = GetModuleHandleW(L"siege-mfc.dll");

		auto left_size = (parent_size->right - parent_size->left) / 9;
		auto dir_list = win32::CreateWindowExW(CREATESTRUCTW {
						.hwndParent = self,
						.cy = parent_size->bottom - parent_size->top,
						.cx = left_size,
						.y = 0,
						.x = 0,
						.style = WS_CHILD  | WS_VISIBLE, 
						.lpszClass = L"MFC::CMFCShellTreeCtrl"
					});
		assert(dir_list);

		auto tab_control_instance = win32::CreateWindowExW(CREATESTRUCTW {
						.hwndParent = self,
						.cy = parent_size->bottom - parent_size->top,
						.cx = parent_size->right  - parent_size->left - left_size - 10,
						.y = 0,
						.x = left_size + 10,
						.style = WS_CHILD | WS_VISIBLE | TCS_MULTILINE | TCS_RIGHTJUSTIFY, 
						.lpszClass = win32::tab_control::class_name
					});

		assert(tab_control_instance);

		auto children = std::array{*dir_list, *tab_control_instance};
        win32::StackChildren(*win32::GetClientSize(self), children, win32::StackDirection::Horizontal);

		parent_size = win32::GetClientRect(*tab_control_instance);

		int index = 0;
		for (auto& plugin : loaded_modules)
		{
			/*for (auto& window : plugin.data->available_classes)
			{
				auto child = win32::CreateWindowExW(win32::window_params<RECT>{
					.parent = self,
					.class_name = window.first.c_str(),
					.class_module = plugin.module.get(),
					.position = *parent_size
				});

				assert(child);

				win32::tab_control::InsertItem(*tab_control_instance, index, TCITEMW {
						.mask = TCIF_TEXT | TCIF_PARAM,
						.pszText = const_cast<wchar_t*>(window.first.c_str()),
						.lParam = win32::lparam_t(*child)
					});

				SendMessageW(*tab_control_instance, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&parent_size));

				SetWindowLongPtrW(*child, GWLP_ID, index);
				index++;
			}

			SendMessageW(*tab_control_instance, TCM_SETCURSEL, 0, 0);
			NMHDR notification{.hwndFrom = *tab_control_instance, .code = TCN_SELCHANGE};
			SendMessageW(self, WM_NOTIFY, 0, std::bit_cast<LPARAM>(&notification));*/
		}

		win32::tab_control::InsertItem(*tab_control_instance, index, TCITEMW {
						.mask = TCIF_TEXT,
						.pszText = const_cast<wchar_t*>(L"+"),
					});

		
		return 0;
	}

	auto on_size(win32::size_message sized)
	{
		/*auto tab = ::FindWindowExW(self, nullptr, win32::tab_control::class_name, nullptr);
		assert(tab);

		win32::SetWindowPos(tab, POINT{}, sized.client_size);
			
		RECT temp {.left = 0, .top = 0, .right = sized.client_size.cx, .bottom = sized.client_size.cy };

		SendMessageW(tab, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&temp));

		for (auto i = 0; i < win32::tab_control::GetItemCount(tab); ++i)
		{
			auto tab_item = win32::tab_control::GetItem(tab, i);
			assert(win32::SetWindowPos(win32::hwnd_t(tab_item->lParam), temp));		
		}*/

		return std::nullopt;
	}

	auto on_notify(win32::notify_message notification)
	{
		auto [sender, id, code] = notification;

		
		if (code == TCN_SELCHANGING)
		{
			auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);
			auto tab_item = win32::tab_control::GetItem(sender, current_index);

			::ShowWindow(win32::hwnd_t(tab_item->lParam), SW_HIDE);
		}
		else if (code == TCN_SELCHANGE)
		{
			auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);
			auto tab_item = win32::tab_control::GetItem(sender, current_index);
			

			if (tab_item->lParam == 0)
			{
				return 0;
			}

			win32::SetWindowPos(win32::hwnd_t(tab_item->lParam), HWND_TOP);

			auto temp = win32::GetClientRect(sender);

			::MapWindowPoints(sender, GetParent(sender), std::bit_cast<POINT*>(&temp), 2);

			SendMessageW(sender, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&temp.value()));

			win32::SetWindowPos(win32::hwnd_t(tab_item->lParam), *temp);

			win32::com::ICollection* object = nullptr;

			auto com_result = AccessibleObjectFromWindow(win32::hwnd_t(tab_item->lParam), OBJID_NATIVEOM, __uuidof(IDispatch), reinterpret_cast<void**>(&object));
			
			if (com_result == S_OK && object)
			{
				auto count = object->Count();

				if (count == 0)
				{
					assert(count);
					assert(*count == 0);

					win32::com::Variant streamHolder;
				
					streamHolder.vt = VT_UNKNOWN;
				
					if (CreateStreamOnHGlobal(nullptr, TRUE, reinterpret_cast<IStream**>(&streamHolder.punkVal)) == S_OK)
					{
						auto addResult = object->Add(streamHolder);

						assert(addResult);
						assert(addResult->vt == VT_EMPTY);

						count = object->Count();

						assert(count);
						assert(*count == 1);

						auto enumerator = object->NewEnum();

						assert(enumerator);

						std::array<win32::com::Variant, 8> files{};
						ULONG actual = 0;

						assert(enumerator.value()->Next(files.size(), files.data(), &actual) == S_FALSE);
						assert(actual == 1);
						assert(files[0].vt == VT_UNKNOWN);

						assert(enumerator.value()->Release() == 0);

//						assert(files[0].punkVal->AddRef() == 3);
	//					assert(files[0].punkVal->Release() == 2);

						for (auto& item : *object)
						{
							if (IsDebuggerPresent())
							{
								assert(&item);							
							}
						}
					}
				}
			}
			
			ShowWindow(win32::hwnd_t(tab_item->lParam), SW_SHOW);
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
					
				if (command.identifier == 100)
				{
					auto dialog = win32::com::CreateFileOpenDialog();

					if (dialog)
					{
						auto result = dialog.value()->Show(nullptr);

						if (result == S_OK)
						{
							auto item = dialog.value()->GetResult();

							if (item)
							{
								auto path = item.value()->GetFileSysPath();

								OutputDebugStringW(path->c_str());
							}
						}
					}
				}

				if (command.identifier == 101)
				{
					auto dialog = win32::MakeDialogTemplate(
						DLGTEMPLATE{.style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION, .cx = 300, .cy = 300 }
					);

					/*
					auto dialog = win32::MakeDialogTemplate(
					DLGTEMPLATE{.style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION, .cdit = 2, .cx = 300, .cy = 300 },
						std::array<wchar_t, 12>{L"Test Dialog"},
						std::make_pair(
							win32::MakeDialogItemTemplate(DLGITEMTEMPLATE{.style = WS_CHILD | WS_VISIBLE, .x = 10, .y = 10, .cx = 200, .cy = 100}, std::array<wchar_t, 12>{L"Hello World"} ),
							win32::MakeDialogItemTemplate(DLGITEMTEMPLATE{.style = WS_CHILD | WS_VISIBLE, .x = 10, .y = 110, .cx = 200, .cy = 100}, std::array<wchar_t, 9>{L"Click me"}, std::array<wchar_t, 2>{{0xffff, win32::button::dialog_id}})
						)
						);
					*/
				
                    win32::DialogBoxIndirectParamW(self, &dialog.dialog, [](win32::hwnd_t self, const win32::message& dialog_message) -> INT_PTR {
						if (dialog_message.message == win32::init_dialog_message::id)
						{
							SetWindowTextW(self, L"Test Dialog Title");

							RECT size {
								.right = 300,
								.bottom = 100
							};
							MapDialogRect(self, &size);

							win32::CreateWindowExW(win32::window_params<RECT>{
								.parent = self,
								.class_name = win32::button::class_name,
								.caption = L"Hello world",
								.style = win32::window_style(WS_CHILD | WS_VISIBLE),
								.position = size,
								
							});
							return (INT_PTR)TRUE;
						}
						else if (dialog_message.message == win32::command_message::id)
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
				else if (command.identifier == 100)
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

	win32::com::init_com();

	INITCOMMONCONTROLSEX settings{
		.dwSize{sizeof(INITCOMMONCONTROLSEX)}
	};
	InitCommonControlsEx(&settings);


	auto mfcHandle = LoadLibraryExW(L"siege-mfc.dll", nullptr, 0);

	win32::RegisterClassExW<siege_main_window>(WNDCLASSEXW {
		.style{CS_HREDRAW | CS_VREDRAW},
		.hInstance = hInstance,
		.hCursor = LoadCursorW(hInstance, IDC_ARROW),
		.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
		.lpszClassName{win32::type_name<siege_main_window>().c_str()},
	});

	auto main_window = win32::CreateWindowExW(CREATESTRUCTW {
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