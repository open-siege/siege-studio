#ifndef SIEGE_MAIN_WINDOW_HPP
#define SIEGE_MAIN_WINDOW_HPP

#include <siege/platform/win/desktop/win32_controls.hpp>
#include "siege-plugin.hpp"

struct siege_main_window
{
	win32::hwnd_t self;
	win32::tab_control tab_control;
	std::list<siege::siege_plugin> loaded_modules;
	
	std::set<std::wstring> extensions;
	std::set<std::wstring> categories;

	std::size_t open_id = 0u;

	siege_main_window(win32::hwnd_t self, const CREATESTRUCTW& params) : self(self), tab_control(nullptr)
	{
		open_id = RegisterWindowMessageW(L"COMMAND_OPEN");
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

		auto mfcModule = GetModuleHandleW(L"siege-win-mfc.dll");

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

		auto tab_control_instance = win32::CreateWindowExW<win32::tab_control>(CREATESTRUCTW {
						.hwndParent = self,
						.cy = parent_size->bottom - parent_size->top,
						.cx = parent_size->right  - parent_size->left - left_size - 10,
						.y = 0,
						.x = left_size + 10,
						.style = WS_CHILD | WS_VISIBLE | TCS_MULTILINE | TCS_RIGHTJUSTIFY, 
						.lpszClass = win32::tab_control::class_name
					});

		assert(tab_control_instance);
		tab_control = *tab_control_instance;

		auto children = std::array<win32::hwnd_t, 2>{*dir_list, *tab_control_instance};
        win32::StackChildren(*win32::GetClientSize(self), children, win32::StackDirection::Horizontal);

		parent_size = win32::GetClientRect(*tab_control_instance);

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

		tab_control_instance->InsertItem(0, TCITEMW {
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
			auto tab_item = win32::tab_control(sender).GetItem(current_index);

			::ShowWindow(win32::hwnd_t(tab_item->lParam), SW_HIDE);
		}
		else if (code == TCN_SELCHANGE)
		{
			auto current_index = SendMessageW(sender, TCM_GETCURSEL, 0, 0);
			auto tab_item = win32::tab_control(sender).GetItem(current_index);
			

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
					
				if (command.notification_code == 0 && command.identifier == open_id)
				{
					auto dialog = win32::com::CreateFileOpenDialog();

					if (dialog)
					{
						struct filter : COMDLG_FILTERSPEC
						{
							std::wstring name;
							std::wstring spec;

							filter(std::wstring name, std::wstring spec) noexcept : name(std::move(name)), spec(std::move(spec)) 
							{
								this->pszName = this->name.c_str();
								this->pszSpec = this->spec.c_str();
							}
						};

						std::vector<filter> temp;

						temp.reserve(extensions.size());

						for (auto& extension : extensions)
						{
							temp.emplace_back(L"", L"*" + extension);
						}

						std::vector<COMDLG_FILTERSPEC> filetypes(temp.begin(), temp.end());
						dialog.value()->SetFileTypes(filetypes.size(), filetypes.data());
						auto result = dialog.value()->Show(nullptr);

						if (result == S_OK)
						{
							auto item = dialog.value()->GetResult();

							if (item)
							{
								auto path = item.value()->GetFileSysPath();

								IStream* stream = nullptr;

								if (SHCreateStreamOnFileEx(path->c_str(), STGM_READ, 0, FALSE, nullptr, &stream) == S_OK)
								{
									auto plugin = std::find_if(loaded_modules.begin(), loaded_modules.end(), [&](auto& module) {
										return 	module.IsStreamSupported(*stream);
									});			

									if (plugin != loaded_modules.end())
									{
										auto class_name = plugin->GetWindowClassForStream(*stream);

										auto parent_size = win32::GetClientRect(self);

										auto child = win32::CreateWindowExW(win32::window_params<RECT>{
											.parent = self,
											.class_name = class_name.c_str(),
											.class_module = plugin->GetHandle(),
											.position = *parent_size
										});

										assert(child);

										auto index = tab_control.GetItemCount() - 1;

										tab_control.InsertItem(index, TCITEMW {
												.mask = TCIF_TEXT | TCIF_PARAM,
												.pszText = const_cast<wchar_t*>(path->filename().c_str()),
												.lParam = win32::lparam_t(*child)
											});

										SendMessageW(tab_control, TCM_ADJUSTRECT, FALSE, std::bit_cast<win32::lparam_t>(&parent_size));

										SetWindowLongPtrW(*child, GWLP_ID, index);
									}

									stream->Release();
								}
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
#endif